/*******************************************************************************
 * Copyright 2018 GeoData <geodata@soton.ac.uk>
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *******************************************************************************/

/**
 * @file GDALDatasetReader.cpp
 * @brief This defines the `GDALDatasetReader` class
 */

#include "gdal_priv.h"
#include "gdalwarper.h"

#include "CTBException.hpp"
#include "GDALDatasetReader.hpp"
#include "TerrainTiler.hpp"

using namespace ctb;

/**
 * @details 
 * Read a region of raster heights for the specified Dataset and Coordinate. 
 * This method uses `GDALRasterBand::RasterIO` function.
 */
float *
ctb::GDALDatasetReader::readRasterHeights(const GDALTiler &tiler, GDALDataset *dataset, const TileCoordinate &coord, ctb::i_tile tileSizeX, ctb::i_tile tileSizeY) {
  GDALTile *rasterTile = createRasterTile(tiler, dataset, coord); // the raster associated with this tile coordinate

  const ctb::i_tile TILE_CELL_SIZE = tileSizeX * tileSizeY;
  float *rasterHeights = (float *)CPLCalloc(TILE_CELL_SIZE, sizeof(float));

  GDALRasterBand *heightsBand = rasterTile->dataset->GetRasterBand(1);

  if (heightsBand->RasterIO(GF_Read, 0, 0, tileSizeX, tileSizeY,
                            (void *) rasterHeights, tileSizeX, tileSizeY, GDT_Float32,
                            0, 0) != CE_None) {
    delete rasterTile;
    CPLFree(rasterHeights);

    throw CTBException("Could not read heights from raster");
  }
  delete rasterTile;
  return rasterHeights;
}

/// Create a raster tile from a tile coordinate
GDALTile *
ctb::GDALDatasetReader::createRasterTile(const GDALTiler &tiler, GDALDataset *dataset, const TileCoordinate &coord) {
  return tiler.createRasterTile(dataset, coord);
}

/// Create a VTR raster overview from a GDALDataset
GDALDataset *
ctb::GDALDatasetReader::createOverview(const GDALTiler &tiler, GDALDataset *dataset, const TileCoordinate &coord, int overviewIndex) {
  int nFactorScale = 2 << overviewIndex;
  int nRasterXSize = dataset->GetRasterXSize() / nFactorScale;
  int nRasterYSize = dataset->GetRasterYSize() / nFactorScale;

  GDALDataset *poOverview = NULL;
  double adfGeoTransform[6];

  // Should We create an overview of the Dataset?
  if (nRasterXSize > 4 && nRasterYSize > 4 && dataset->GetGeoTransform(adfGeoTransform) == CE_None) {
    adfGeoTransform[1] *= nFactorScale;
    adfGeoTransform[5] *= nFactorScale;

    TerrainTiler tempTiler(tiler.dataset(), tiler.grid(), tiler.options);
    tempTiler.crsWKT = "";
    GDALTile *rasterTile = createRasterTile(tempTiler, dataset, coord);
    if (rasterTile) {
      poOverview = rasterTile->detach();
      delete rasterTile;
    }
  }
  return poOverview;
}

/// The destructor
ctb::GDALDatasetReaderWithOverviews::~GDALDatasetReaderWithOverviews() {
  reset();
}

/// Read a region of raster heights into an array for the specified Dataset and Coordinate
float *
ctb::GDALDatasetReaderWithOverviews::readRasterHeights(GDALDataset *dataset, const TileCoordinate &coord, ctb::i_tile tileSizeX, ctb::i_tile tileSizeY) {
  GDALDataset *mainDataset = dataset;

  const ctb::i_tile TILE_CELL_SIZE = tileSizeX * tileSizeY;
  float *rasterHeights = (float *)CPLCalloc(TILE_CELL_SIZE, sizeof(float));

  // Replace GDAL Dataset by last valid Overview.
  for (int i = mOverviews.size() - 1; i >= 0; --i) {
    if (mOverviews[i]) {
      dataset = mOverviews[i];
      break;
    }
  }

  // Extract the raster data, using overviews when necessary
  bool rasterOk = false;
  while (!rasterOk) {
    GDALTile *rasterTile = createRasterTile(poTiler, dataset, coord); // the raster associated with this tile coordinate

    GDALRasterBand *heightsBand = rasterTile->dataset->GetRasterBand(1);

    if (heightsBand->RasterIO(GF_Read, 0, 0, tileSizeX, tileSizeY,
                              (void *) rasterHeights, tileSizeX, tileSizeY, GDT_Float32,
                              0, 0) != CE_None) {
      
      GDALDataset *psOverview = createOverview(poTiler, mainDataset, coord, mOverviewIndex++);
      if (psOverview) {
        mOverviews.push_back(psOverview);
        dataset = psOverview;
      }
      else {
        delete rasterTile;
        CPLFree(rasterHeights);
        throw CTBException("Could not create an overview of current GDAL dataset");
      }
    }
    else {
      rasterOk = true;
    }
    delete rasterTile;
  }

  // Everything ok?
  if (!rasterOk) {
    CPLFree(rasterHeights);
    throw CTBException("Could not read heights from raster");
  }
  return rasterHeights;
}

/// Releases all overviews
void 
ctb::GDALDatasetReaderWithOverviews::reset() {
  mOverviewIndex = 0;

  for (int i = mOverviews.size() - 1; i >= 0; --i) {
    GDALDataset *poOverview = mOverviews[i];
    GDALClose(poOverview);
  }
  mOverviews.clear();
}
