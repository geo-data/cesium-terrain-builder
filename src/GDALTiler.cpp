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
#include <string.h>             // strlen

#include "ogr_spatialref.h"
#include "ogr_srs_api.h"

#include "config.hpp"
#include "CTBException.hpp"
#include "GDALTiler.hpp"

using namespace ctb;

GDALTiler::GDALTiler(GDALDataset *poDataset, const Grid &grid, const TilerOptions &options):
  mGrid(grid),
  poDataset(poDataset),
  options(options)
{
  // if the dataset is set we need to initialise the tile bounds and raster
  // resolution from it.
  if (poDataset != NULL) {

    // Get the bounds of the dataset
    double adfGeoTransform[6];
    CRSBounds bounds;

    if (poDataset->GetGeoTransform(adfGeoTransform) == CE_None) {
      bounds = CRSBounds(adfGeoTransform[0],
                         adfGeoTransform[3] + (poDataset->GetRasterYSize() * adfGeoTransform[5]),
                         adfGeoTransform[0] + (poDataset->GetRasterXSize() * adfGeoTransform[1]),
                         adfGeoTransform[3]);
    } else {
      throw CTBException("Could not get transformation information from source dataset");
    }

    // Find out whether the dataset SRS matches that of the grid
    const char *srcWKT = poDataset->GetProjectionRef();
    if (!strlen(srcWKT))
      throw CTBException("The source dataset does not have a spatial reference system assigned");

    OGRSpatialReference srcSRS = OGRSpatialReference(srcWKT);
    OGRSpatialReference gridSRS = mGrid.getSRS();

    if (!srcSRS.IsSame(&gridSRS)) { // it doesn't match
      // Check the srs is valid
      switch(srcSRS.Validate()) {
      case OGRERR_NONE:
        break;
      case OGRERR_CORRUPT_DATA:
        throw CTBException("The source spatial reference system appears to be corrupted");
        break;
      case OGRERR_UNSUPPORTED_SRS:
        throw CTBException("The source spatial reference system is not supported");
        break;
      default:
        throw CTBException("There is an unhandled return value from `srcSRS.Validate()`");
      }

      // We need to transform the bounds to the grid SRS
      double x[4] = { bounds.getMinX(), bounds.getMaxX(), bounds.getMaxX(), bounds.getMinX() };
      double y[4] = { bounds.getMinY(), bounds.getMinY(), bounds.getMaxY(), bounds.getMaxY() };

      OGRCoordinateTransformation *transformer = OGRCreateCoordinateTransformation(&srcSRS, &gridSRS);
      if (transformer == NULL) {
        throw CTBException("The source dataset to tile grid coordinate transformation could not be created");
      } else if (transformer->Transform(4, x, y) != true) {
        delete transformer;
        throw CTBException("Could not transform dataset bounds to tile spatial reference system");
      }
      delete transformer;

      // Get the min and max values of the transformed coordinates (this should
      // be replaced using std::minmax in C++11).
      double minX = std::min(std::min(x[0], x[1]), std::min(x[2], x[3])),
        maxX = std::max(std::max(x[0], x[1]), std::max(x[2], x[3])),
        minY = std::min(std::min(y[0], y[1]), std::min(y[2], y[3])),
        maxY = std::max(std::max(y[0], y[1]), std::max(y[2], y[3]));

      mBounds = CRSBounds(minX, minY, maxX, maxY); // set the bounds
      mResolution = mBounds.getWidth() / poDataset->GetRasterXSize(); // set the resolution

      // cache the SRS string for use in reprojections later
      char *srsWKT = NULL;
      if (gridSRS.exportToWkt(&srsWKT) != OGRERR_NONE) {
        CPLFree(srsWKT);
        throw CTBException("Could not create grid WKT string");
      }
      crsWKT = srsWKT;
      CPLFree(srsWKT);
      srsWKT = NULL;

    } else {                    // no reprojection is necessary
      mBounds = bounds;         // use the existing dataset bounds
      mResolution = std::abs(adfGeoTransform[1]); // use the existing dataset resolution
    }

    poDataset->Reference();     // increase the refcount of the dataset
  }
}

GDALTiler::GDALTiler(const GDALTiler &other):
  mGrid(other.mGrid),
  poDataset(other.poDataset),
  mBounds(other.mBounds),
  mResolution(other.mResolution),
  crsWKT(other.crsWKT)
{
  if (poDataset != NULL) {
    poDataset->Reference();     // increase the refcount of the dataset
  }
}

GDALTiler::GDALTiler(GDALTiler &other):
  mGrid(other.mGrid),
  poDataset(other.poDataset),
  mBounds(other.mBounds),
  mResolution(other.mResolution),
  crsWKT(other.crsWKT)
{
  if (poDataset != NULL) {
    poDataset->Reference();     // increase the refcount of the dataset
  }
}

GDALTiler &
GDALTiler::operator=(const GDALTiler &other) {
  closeDataset();

  mGrid = other.mGrid;
  poDataset = other.poDataset;

  if (poDataset != NULL) {
    poDataset->Reference();     // increase the refcount of the dataset
  }

  mBounds = other.mBounds;
  mResolution = other.mResolution;
  crsWKT = other.crsWKT;

  return *this;
}

GDALTiler::~GDALTiler() {
  closeDataset();
}

GDALTile *
GDALTiler::createRasterTile(const TileCoordinate &coord) const {
  // Convert the tile bounds into a geo transform
  double adfGeoTransform[6],
    resolution = mGrid.resolution(coord.zoom);
  CRSBounds tileBounds = mGrid.tileBounds(coord);

  adfGeoTransform[0] = tileBounds.getMinX(); // min longitude
  adfGeoTransform[1] = resolution;
  adfGeoTransform[2] = 0;
  adfGeoTransform[3] = tileBounds.getMaxY(); // max latitude
  adfGeoTransform[4] = 0;
  adfGeoTransform[5] = -resolution;

  GDALTile *tile = createRasterTile(adfGeoTransform);
  tile->coord = coord;

  // Set the shifted geo transform to the VRT
  if (GDALSetGeoTransform(tile->dataset, adfGeoTransform) != CE_None) {
    throw CTBException("Could not set geo transform on VRT");
  }

  return tile;
}

/**
 * @details This method is the heart of the tiler.  A `TileCoordinate` is used
 * to obtain the geospatial extent associated with that tile as related to the
 * underlying GDAL dataset. This mapping may require a reprojection if the
 * underlying dataset is not in the tile projection system.  This information
 * is then encapsulated as a GDAL virtual raster (VRT) dataset and returned to
 * the caller.
 *
 * It is the caller's responsibility to call `GDALClose()` on the returned
 * dataset.
 */
GDALTile *
GDALTiler::createRasterTile(double (&adfGeoTransform)[6]) const {
  if (poDataset == NULL) {
    throw CTBException("No GDAL dataset is set");
  }

  // The source and sink datasets
  GDALDatasetH hSrcDS = (GDALDatasetH) dataset();
  GDALDatasetH hDstDS;

  // The transformation option list
  CPLStringList transformOptions;

  // The source, sink and grid srs
  const char *pszSrcWKT = GDALGetProjectionRef(hSrcDS),
    *pszGridWKT = pszSrcWKT;

  if (!strlen(pszSrcWKT))
    throw CTBException("The source dataset no longer has a spatial reference system assigned");

  // Populate the SRS WKT strings if we need to reproject
  if (requiresReprojection()) {
    pszGridWKT = crsWKT.c_str();
    transformOptions.SetNameValue("SRC_SRS", pszSrcWKT);
    transformOptions.SetNameValue("DST_SRS", pszGridWKT);
  }

  // Set the warp options
  GDALWarpOptions *psWarpOptions = GDALCreateWarpOptions();
  //psWarpOptions->eResampleAlg = eResampleAlg;
  psWarpOptions->dfWarpMemoryLimit = options.warpMemoryLimit;
  psWarpOptions->hSrcDS = hSrcDS;
  psWarpOptions->nBandCount = poDataset->GetRasterCount();
  psWarpOptions->panSrcBands =
    (int *) CPLMalloc(sizeof(int) * psWarpOptions->nBandCount );
  psWarpOptions->panDstBands =
    (int *) CPLMalloc(sizeof(int) * psWarpOptions->nBandCount );

  for (short unsigned int i = 0; i < psWarpOptions->nBandCount; ++i) {
    psWarpOptions->panDstBands[i] = psWarpOptions->panSrcBands[i] = i + 1;
  }

  // Create the image to image transformer
  void *transformerArg = GDALCreateGenImgProjTransformer2(hSrcDS, NULL, transformOptions.List());
  if(transformerArg == NULL) {
    GDALDestroyWarpOptions(psWarpOptions);
    throw CTBException("Could not create image to image transformer");
  }

  // Specify the destination geotransform
  GDALSetGenImgProjTransformerDstGeoTransform(transformerArg, adfGeoTransform );

  // Decide if we are doing an approximate or exact transformation
  if (options.errorThreshold) {
    // approximate: wrap the transformer with a linear approximator
    psWarpOptions->pTransformerArg =
      GDALCreateApproxTransformer(GDALGenImgProjTransform, transformerArg, options.errorThreshold);

    if (psWarpOptions->pTransformerArg == NULL) {
      GDALDestroyWarpOptions(psWarpOptions);
      GDALDestroyGenImgProjTransformer(transformerArg);
      throw CTBException("Could not create linear approximator");
    }

    psWarpOptions->pfnTransformer = GDALApproxTransform;

  } else {
    // exact: no wrapping required
    psWarpOptions->pTransformerArg = transformerArg;
    psWarpOptions->pfnTransformer = GDALGenImgProjTransform;
  }

  // Specify a multi threaded warp operation using all CPU cores
  CPLStringList warpOptions(psWarpOptions->papszWarpOptions, false);
  warpOptions.SetNameValue("NUM_THREADS", "ALL_CPUS");
  psWarpOptions->papszWarpOptions = warpOptions.StealList();

  // The raster tile is represented as a VRT dataset
  hDstDS = GDALCreateWarpedVRT(hSrcDS, mGrid.tileSize(), mGrid.tileSize(), adfGeoTransform, psWarpOptions);
  GDALDestroyWarpOptions( psWarpOptions );

  if (hDstDS == NULL) {
    GDALDestroyGenImgProjTransformer(transformerArg);
    throw CTBException("Could not create warped VRT");
  }

  // Set the projection information on the dataset. This will always be the grid
  // SRS.
  if (GDALSetProjection( hDstDS, pszGridWKT ) != CE_None) {
    GDALClose(hDstDS);
    if (transformerArg != NULL) {
      GDALDestroyGenImgProjTransformer(transformerArg);
    }
    throw CTBException("Could not set projection on VRT");
  }

  // If uncommenting the following line for debug purposes, you must also `#include "vrtdataset.h"`
  //std::cout << "VRT: " << CPLSerializeXMLTree(((VRTWarpedDataset *) hDstDS)->SerializeToXML(NULL)) << std::endl;

  // Create the tile, passing it the base image transformer to manage if this is
  // an approximate transform
  return new GDALTile((GDALDataset *) hDstDS,
                      (psWarpOptions->pfnTransformer == GDALApproxTransform)
                      ? transformerArg : NULL);
}

/**
 * @details This dereferences the underlying GDAL dataset and closes it if the
 * reference count falls below 1.
 */
void
GDALTiler::closeDataset() {
  // Dereference and possibly close the GDAL dataset
  if (poDataset != NULL) {
    poDataset->Dereference();

    if (poDataset->GetRefCount() < 1) {
      GDALClose(poDataset);
    }

    poDataset = NULL;
  }
}
