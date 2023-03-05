/*******************************************************************************
 * Copyright 2014 GeoData <geodata@soton.ac.uk>
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
 * @file TerrainTiler.cpp
 * @brief This defines the `TerrainTiler` class
 */

#include "CTBException.hpp"
#include "TerrainTiler.hpp"
#include "GDALDatasetReader.hpp"

using namespace ctb;

void 
ctb::TerrainTiler::prepareSettingsOfTile(TerrainTile *terrainTile, const TileCoordinate &coord, float *rasterHeights, ctb::i_tile tileSizeX, ctb::i_tile tileSizeY) const {
  const ctb::i_tile TILE_CELL_SIZE = tileSizeX * tileSizeY;

  // Convert the raster data into the terrain tile heights.  This assumes the
  // input raster data represents meters above sea level. Each terrain height
  // value is the number of 1/5 meter units above -1000 meters.
  // TODO: try doing this using a VRT derived band:
  // (http://www.gdal.org/gdal_vrttut.html)
  for (unsigned short int i = 0; i < TILE_CELL_SIZE; i++) {
    terrainTile->mHeights[i] = (i_terrain_height) ((rasterHeights[i] + 1000) * 5);
  }

  // If we are not at the maximum zoom level we need to set child flags on the
  // tile where child tiles overlap the dataset bounds.
  if (coord.zoom != maxZoomLevel()) {
    CRSBounds tileBounds = mGrid.tileBounds(coord);

    if (! (bounds().overlaps(tileBounds))) {
      terrainTile->setAllChildren(false);
    } else {
      if (bounds().overlaps(tileBounds.getSW())) {
        terrainTile->setChildSW();
      }
      if (bounds().overlaps(tileBounds.getNW())) {
        terrainTile->setChildNW();
      }
      if (bounds().overlaps(tileBounds.getNE())) {
        terrainTile->setChildNE();
      }
      if (bounds().overlaps(tileBounds.getSE())) {
        terrainTile->setChildSE();
      }
    }
  }
}

TerrainTile *
ctb::TerrainTiler::createTile(GDALDataset *dataset, const TileCoordinate &coord) const {
  // Copy the raster data into an array
  float *rasterHeights = ctb::GDALDatasetReader::readRasterHeights(*this, dataset, coord, TILE_SIZE, TILE_SIZE);

  // Get a terrain tile represented by the tile coordinate
  TerrainTile *terrainTile = new TerrainTile(coord);
  prepareSettingsOfTile(terrainTile, coord, rasterHeights, TILE_SIZE, TILE_SIZE);
  CPLFree(rasterHeights);

  return terrainTile;
}

TerrainTile *
ctb::TerrainTiler::createTile(GDALDataset *dataset, const TileCoordinate &coord, ctb::GDALDatasetReader *reader) const {
  // Copy the raster data into an array
  float *rasterHeights = reader->readRasterHeights(dataset, coord, TILE_SIZE, TILE_SIZE);

  // Get a mesh tile represented by the tile coordinate
  TerrainTile *terrainTile = new TerrainTile(coord);
  prepareSettingsOfTile(terrainTile, coord, rasterHeights, TILE_SIZE, TILE_SIZE);
  CPLFree(rasterHeights);

  return terrainTile;
}

GDALTile *
ctb::TerrainTiler::createRasterTile(GDALDataset *dataset, const TileCoordinate &coord) const {
  // Ensure we have some data from which to create a tile
  if (dataset && dataset->GetRasterCount() < 1) {
    throw CTBException("At least one band must be present in the GDAL dataset");
  }

  // Get the bounds and resolution for a tile coordinate which represents the
  // data overlap requested by the terrain specification.
  double resolution;
  CRSBounds tileBounds = terrainTileBounds(coord, resolution);

  // Convert the tile bounds into a geo transform
  double adfGeoTransform[6];
  adfGeoTransform[0] = tileBounds.getMinX(); // min longitude
  adfGeoTransform[1] = resolution;
  adfGeoTransform[2] = 0;
  adfGeoTransform[3] = tileBounds.getMaxY(); // max latitude
  adfGeoTransform[4] = 0;
  adfGeoTransform[5] = -resolution;

  GDALTile *tile = GDALTiler::createRasterTile(dataset, adfGeoTransform);

  // The previous geotransform represented the data with an overlap as required
  // by the terrain specification.  This now needs to be overwritten so that
  // the data is shifted to the bounds defined by tile itself.
  tileBounds = mGrid.tileBounds(coord);
  resolution = mGrid.resolution(coord.zoom);
  adfGeoTransform[0] = tileBounds.getMinX(); // min longitude
  adfGeoTransform[1] = resolution;
  adfGeoTransform[2] = 0;
  adfGeoTransform[3] = tileBounds.getMaxY(); // max latitude
  adfGeoTransform[4] = 0;
  adfGeoTransform[5] = -resolution;

  // Set the shifted geo transform to the VRT
  if (GDALSetGeoTransform(tile->dataset, adfGeoTransform) != CE_None) {
    throw CTBException("Could not set geo transform on VRT");
  }

  return tile;
}

TerrainTiler &
ctb::TerrainTiler::operator=(const TerrainTiler &other) {
  GDALTiler::operator=(other);

  return *this;
}
