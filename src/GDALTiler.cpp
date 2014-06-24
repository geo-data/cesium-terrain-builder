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

GDALDatasetH
terrain::GDALTiler::createRasterTile(const TileCoordinate &coord) const {
  // Convert the tile bounds into a geo transform
  double adfGeoTransform[6],
    resolution = mProfile.resolution(coord.zoom);
  LatLonBounds tileBounds = mProfile.tileBounds(coord);

  adfGeoTransform[0] = tileBounds.getMinX(); // min longitude
  adfGeoTransform[1] = resolution;
  adfGeoTransform[2] = 0;
  adfGeoTransform[3] = tileBounds.getMaxY(); // max latitude
  adfGeoTransform[4] = 0;
  adfGeoTransform[5] = -resolution;

  GDALDatasetH hDstDS = createRasterTile(adfGeoTransform);

  // Set the shifted geo transform to the VRT
  if (GDALSetGeoTransform( hDstDS, adfGeoTransform ) != CE_None) {
    GDALClose(hDstDS);
    throw TerrainException("Could not set geo transform on VRT");
  }

  return hDstDS;
}

/**
 * @details This method is the heart of the tiler.  A `TileCoordinate` is used
 * to obtain the geospatial extent associated with that tile as related to the
 * underlying GDAL dataset. This mapping may require a reprojection if the
 * underlying dataset is not in the EPSG:4326 projection.  This information is
 * then encapsulated as a GDAL virtual raster (VRT) dataset and returned to the
 * caller.
 *
 * It is the caller's responsibility to call `GDALClose()` on the returned
 * dataset.
 */
GDALDatasetH
terrain::GDALTiler::createRasterTile(double (&adfGeoTransform)[6]) const {
  if (poDataset == NULL) {
    throw TerrainException("No GDAL dataset is set");
  }

  short int nBandCount = poDataset->GetRasterCount();
  if (nBandCount < 1) {
    throw TerrainException("At least one band must be present in the GDAL dataset");
  }

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
  psWarpOptions->nBandCount = nBandCount;
  psWarpOptions->panSrcBands =
    (int *) CPLMalloc(sizeof(int) * psWarpOptions->nBandCount );
  psWarpOptions->panDstBands =
    (int *) CPLMalloc(sizeof(int) * psWarpOptions->nBandCount );

  for (short unsigned int i = 0; i < nBandCount; ++i) {
    psWarpOptions->panDstBands[i] = psWarpOptions->panSrcBands[i] = i + 1;
  }

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
