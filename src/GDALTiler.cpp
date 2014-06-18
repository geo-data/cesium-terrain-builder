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
 * @file GDALTiler.cpp
 * @brief This defines the `GDALTiler` class
 */

#include <algorithm>            // std::minmax

#include "ogr_srs_api.h"

#include "config.hpp"
#include "TerrainException.hpp"
#include "GDALTiler.hpp"

using namespace terrain;

terrain::GDALTiler::GDALTiler(GDALDataset *poDataset):
  poDataset(poDataset)
{
  // if the dataset is set we need to initialise the tile bounds and raster
  // resolution from it.
  if (poDataset != NULL) {

    // Get the bounds of the dataset
    double adfGeoTransform[6];
    LatLonBounds bounds;

    if( poDataset->GetGeoTransform( adfGeoTransform ) == CE_None ) {
      bounds = LatLonBounds(adfGeoTransform[0],
                             adfGeoTransform[3] + (poDataset->GetRasterYSize() * adfGeoTransform[5]),
                             adfGeoTransform[0] + (poDataset->GetRasterXSize() * adfGeoTransform[1]),
                             adfGeoTransform[3]);
    } else {
      throw TerrainException("Could not get transformation information from dataset");
    }

    // Find out whether the dataset is in the EPSG:4326 spatial reference system
    OGRSpatialReference srcSRS = OGRSpatialReference(poDataset->GetProjectionRef()),
      wgs84SRS;

    if (wgs84SRS.importFromEPSG(4326) != OGRERR_NONE) {
      throw TerrainException("Could not create EPSG:4326 spatial reference");
    }

    if (!srcSRS.IsSame(&wgs84SRS)) { // it isn't in EPSG:4326
      // We need to transform the bounds to EPSG:4326
      double x[4] = { bounds.getMinX(), bounds.getMaxX(), bounds.getMaxX(), bounds.getMinX() };
      double y[4] = { bounds.getMinY(), bounds.getMinY(), bounds.getMaxY(), bounds.getMaxY() };

      OGRCoordinateTransformation *transformer = OGRCreateCoordinateTransformation(&srcSRS, &wgs84SRS);
      if (transformer->Transform(4, x, y) != true) {
        delete transformer;
        throw TerrainException("Could not transform dataset bounds to EPSG:4326 spatial reference system");
      }
      delete transformer;

      // Get the min and max values of the transformed coordinates (this should
      // be replaced using std::minmax in C++11).
      double minX = std::min(std::min(x[0], x[1]), std::min(x[2], x[3])),
        maxX = std::max(std::max(x[0], x[1]), std::max(x[2], x[3])),
        minY = std::min(std::min(y[0], y[1]), std::min(y[2], y[3])),
        maxY = std::max(std::max(y[0], y[1]), std::max(y[2], y[3]));

      mBounds = LatLonBounds(minX, minY, maxX, maxY); // set the bounds
      mResolution = mBounds.getWidth() / poDataset->GetRasterXSize(); // set the resolution

      // cache the WGS84 SRS string for use in reprojections later
      char *srsWKT = NULL;
      if (wgs84SRS.exportToWkt(&srsWKT) != OGRERR_NONE) {
        CPLFree(srsWKT);
        throw TerrainException("Could not create EPSG:4326 WKT string");
      }
      wgs84WKT = srsWKT;
      CPLFree(srsWKT);
      srsWKT = NULL;

    } else {                    // no reprojection is necessary
      mBounds = bounds;         // use the existing dataset bounds
      mResolution = std::abs(adfGeoTransform[1]); // use the existing dataset resolution
    }

    poDataset->Reference();     // increase the refcount of the dataset
  }
}

terrain::GDALTiler::GDALTiler(const GDALTiler &other):
  mProfile(other.mProfile),
  poDataset(other.poDataset),
  mBounds(other.mBounds),
  mResolution(other.mResolution),
  wgs84WKT(other.wgs84WKT)
{
  if (poDataset != NULL) {
    poDataset->Reference();     // increase the refcount of the dataset
  }
}

terrain::GDALTiler::GDALTiler(GDALTiler &other):
  mProfile(other.mProfile),
  poDataset(other.poDataset),
  mBounds(other.mBounds),
  mResolution(other.mResolution),
  wgs84WKT(other.wgs84WKT)
{
  if (poDataset != NULL) {
    poDataset->Reference();     // increase the refcount of the dataset
  }
}

GDALTiler &
terrain::GDALTiler::operator=(const GDALTiler &other) {
  closeDataset();

  mProfile = other.mProfile;
  poDataset = other.poDataset;

  if (poDataset != NULL) {
    poDataset->Reference();     // increase the refcount of the dataset
  }

  mBounds = other.mBounds;
  mResolution = other.mResolution;
  wgs84WKT = other.wgs84WKT;

  return *this;
}

terrain::GDALTiler::~GDALTiler() {
  closeDataset();
}

TerrainTile
terrain::GDALTiler::createTerrainTile(const TileCoordinate &coord) const {
  TerrainTile terrainTile(coord); // a terrain tile represented by the tile coordinate
  GDALDataset *rasterTile = (GDALDataset *) createRasterTile(coord); // the raster associated with this tile coordinate
  GDALRasterBand *heightsBand = rasterTile->GetRasterBand(1);

  // Copy the raster data into an array
  float rasterHeights[TerrainTile::TILE_CELL_SIZE];
  if (heightsBand->RasterIO(GF_Read, 0, 0, TILE_SIZE, TILE_SIZE,
                            (void *) rasterHeights, TILE_SIZE, TILE_SIZE, GDT_Float32,
                            0, 0) != CE_None) {
    GDALClose(rasterTile);
    throw TerrainException("Could not read heights from raster");
  }

  // Copy the raster data into the terrain tile heights
  // TODO: try doing this using a VRT derived band:
  // (http://www.gdal.org/gdal_vrttut.html)
  for (unsigned short int i = 0; i < TerrainTile::TILE_CELL_SIZE; i++) {
    terrainTile.mHeights[i] = (i_terrain_height) ((rasterHeights[i] + 1000) * 5);
  }

  GDALClose(rasterTile);

  // If we are not at the maximum zoom level we need to set child flags on the
  // tile where child tiles overlap the dataset bounds.
  if (coord.zoom != maxZoomLevel()) {
    LatLonBounds tileBounds = mProfile.tileBounds(coord);

    if (! (bounds().overlaps(tileBounds))) {
      terrainTile.setAllChildren(false);
    } else {
      if (bounds().overlaps(tileBounds.getSW())) {
        terrainTile.setChildSW();
      }
      if (bounds().overlaps(tileBounds.getNW())) {
        terrainTile.setChildNW();
      }
      if (bounds().overlaps(tileBounds.getNE())) {
        terrainTile.setChildNE();
      }
      if (bounds().overlaps(tileBounds.getSE())) {
        terrainTile.setChildSE();
      }
    }
  }

  return terrainTile;
}

/**
 * @details This method is the heart of the tiler.  A `TileCoordinate` is used
 * to obtain the geospatial extent associated with that tile as related to the
 * underlying GDAL dataset. This mapping may require a reprojection if the
 * underlying dataset is not in the EPSG:4326 projection.  This information is
 * then encapsulated as a GDAL virtual raster (VRT) dataset and returned to the
 * caller.
 */
GDALDatasetH
terrain::GDALTiler::createRasterTile(const TileCoordinate &coord) const {
  if (poDataset == NULL) {
    throw TerrainException("No GDAL dataset is set");
  }

  // Get the bounds and resolution for a tile coordinate which represents the
  // data overlap requested by the terrain specification.
  double resolution;
  LatLonBounds tileBounds = terrainTileBounds(coord, resolution);

  // Convert the tile bounds into a geo transform
  double adfGeoTransform[6];
  adfGeoTransform[0] = tileBounds.getMinX(); // min longitude
  adfGeoTransform[1] = resolution;
  adfGeoTransform[2] = 0;
  adfGeoTransform[3] = tileBounds.getMaxY(); // max latitude
  adfGeoTransform[4] = 0;
  adfGeoTransform[5] = -resolution;

  // The source and sink datasets
  GDALDatasetH hSrcDS = (GDALDatasetH) dataset();
  GDALDatasetH hDstDS;

  // The source and sink srs
  const char *pszSrcWKT = NULL;
  const char *pszDstWKT = NULL;

  // Populate the SRS WKT strings if we need to reproject
  if (requiresReprojection()) {
    pszSrcWKT = GDALGetProjectionRef(hSrcDS);
    pszDstWKT = wgs84WKT.c_str();
  }

  // Set the warp options
  GDALWarpOptions *psWarpOptions = GDALCreateWarpOptions();
  //psWarpOptions->eResampleAlg = eResampleAlg;
  psWarpOptions->hSrcDS = hSrcDS;
  psWarpOptions->nBandCount = 1;
  psWarpOptions->panSrcBands =
    (int *) CPLMalloc(sizeof(int) * psWarpOptions->nBandCount );
  psWarpOptions->panSrcBands[0] = 1;
  psWarpOptions->panDstBands =
    (int *) CPLMalloc(sizeof(int) * psWarpOptions->nBandCount );
  psWarpOptions->panDstBands[0] = 1;

  psWarpOptions->pfnTransformer = GDALGenImgProjTransform;
  psWarpOptions->pTransformerArg =
        GDALCreateGenImgProjTransformer( hSrcDS, pszSrcWKT,
                                         NULL, pszDstWKT,
                                         FALSE, 0.0, 1 );

  if( psWarpOptions->pTransformerArg == NULL ) {
    GDALDestroyWarpOptions( psWarpOptions );
    throw TerrainException("Could not create image to image transformer");
  }

  // Specify the destination geotransform
  GDALSetGenImgProjTransformerDstGeoTransform( psWarpOptions->pTransformerArg, adfGeoTransform );

  // The raster tile is represented as a VRT dataset
  hDstDS = GDALCreateWarpedVRT(hSrcDS, mProfile.tileSize(), mProfile.tileSize(), adfGeoTransform, psWarpOptions);
  GDALDestroyWarpOptions( psWarpOptions );

  if (hDstDS == NULL) {
    throw TerrainException("Could not create warped VRT");
  }

  // Set the projection information on the dataset
  if (GDALSetProjection( hDstDS, pszDstWKT ) != CE_None) {
    GDALClose(hDstDS);
    throw TerrainException("Could not set projection on VRT");
  }

  // The previous geotransform represented the data with an overlap as required
  // by the terrain specification.  This now needs to be overwritten so that
  // the data is shifted to the bounds defined by tile itself.
  tileBounds = mProfile.tileBounds(coord);
  resolution = mProfile.resolution(coord.zoom);
  adfGeoTransform[0] = tileBounds.getMinX(); // min longitude
  adfGeoTransform[1] = resolution;
  adfGeoTransform[2] = 0;
  adfGeoTransform[3] = tileBounds.getMaxY(); // max latitude
  adfGeoTransform[4] = 0;
  adfGeoTransform[5] = -resolution;

  // Set the shifted geo transform to the VRT
  if (GDALSetGeoTransform( hDstDS, adfGeoTransform ) != CE_None) {
    GDALClose(hDstDS);
    throw TerrainException("Could not set projection on VRT");
  }

  // If uncommenting the following line for debug purposes, you must also `#include "vrtdataset.h"`
  //std::cout << "VRT: " << CPLSerializeXMLTree(((VRTWarpedDataset *) hDstDS)->SerializeToXML(NULL)) << std::endl;

  return hDstDS;
}

/**
 * @details This dereferences the underlying GDAL dataset and closes it if the
 * reference count falls below 1.
 */
void
terrain::GDALTiler::closeDataset() {
  // Dereference and possibly close the GDAL dataset
  if (poDataset != NULL) {
    poDataset->Dereference();

    if (poDataset->GetRefCount() < 1) {
      GDALClose(poDataset);
    }

    poDataset = NULL;
  }
}
