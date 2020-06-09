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

#include <cmath>                // std::abs
#include <algorithm>            // std::minmax
#include <string.h>             // strlen
#include <mutex>

#include "gdal_priv.h"
#include "gdalwarper.h"
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

  // Transformed bounds can give slightly different results on different threads unless mutexed
  static std::mutex mutex;
  std::lock_guard<std::mutex> lock(mutex);

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

    #if ( GDAL_VERSION_MAJOR >= 3 )
    srcSRS.SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);
    gridSRS.SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);
    #endif

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

      // Get the min and max values of the transformed coordinates
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
GDALTiler::createRasterTile(GDALDataset *dataset, const TileCoordinate &coord) const {
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

  GDALTile *tile = createRasterTile(dataset, adfGeoTransform);
  static_cast<TileCoordinate &>(*tile) = coord;

  // Set the shifted geo transform to the VRT
  if (GDALSetGeoTransform(tile->dataset, adfGeoTransform) != CE_None) {
    throw CTBException("Could not set geo transform on VRT");
  }

  return tile;
}

/**
 * @brief Get an overview dataset which best matches a transformation
 *
 * Try and get an overview from the source dataset that corresponds more closely
 * to the resolution belonging to any output of the transformation.  This will
 * make downsampling operations much quicker and work around integer overflow
 * errors that can occur if downsampling very high resolution source datasets to
 * small scale (low zoom level) tiles.
 *
 * This code is adapted from that found in `gdalwarp.cpp` implementing the
 * `gdalwarp -ovr` option.
 */
#if ( GDAL_VERSION_MAJOR >= 3 )
#include "gdaloverviewdataset.cpp"
#elif ( GDAL_VERSION_MAJOR >= 2 && GDAL_VERSION_MINOR >= 2 )
#include "gdaloverviewdataset-gdal2x.cpp"
#endif

static
GDALDatasetH
getOverviewDataset(GDALDatasetH hSrcDS, GDALTransformerFunc pfnTransformer, void *hTransformerArg) {
  GDALDataset* poSrcDS = static_cast<GDALDataset*>(hSrcDS);
  GDALDataset* poSrcOvrDS = NULL;
  int nOvLevel = -2;
  int nOvCount = poSrcDS->GetRasterBand(1)->GetOverviewCount();
  if( nOvCount > 0 )
    {
      double adfSuggestedGeoTransform[6];
      double adfExtent[4];
      int    nPixels, nLines;
      /* Compute what the "natural" output resolution (in pixels) would be for this */
      /* input dataset */
      if( GDALSuggestedWarpOutput2(hSrcDS, pfnTransformer, hTransformerArg,
                                   adfSuggestedGeoTransform, &nPixels, &nLines,
                                   adfExtent, 0) == CE_None)
        {
          double dfTargetRatio = 1.0 / adfSuggestedGeoTransform[1];
          if( dfTargetRatio > 1.0 )
            {
              int iOvr;
              for( iOvr = -1; iOvr < nOvCount-1; iOvr++ )
                {
                  double dfOvrRatio = (iOvr < 0) ? 1.0 : (double)poSrcDS->GetRasterXSize() /
                    poSrcDS->GetRasterBand(1)->GetOverview(iOvr)->GetXSize();
                  double dfNextOvrRatio = (double)poSrcDS->GetRasterXSize() /
                    poSrcDS->GetRasterBand(1)->GetOverview(iOvr+1)->GetXSize();
                  if( dfOvrRatio < dfTargetRatio && dfNextOvrRatio > dfTargetRatio )
                    break;
                  if( fabs(dfOvrRatio - dfTargetRatio) < 1e-1 )
                    break;
                }
              iOvr += (nOvLevel+2);
              if( iOvr >= 0 )
                {
                  //std::cout << "CTB WARPING: Selecting overview level " << iOvr << " for output dataset " << nPixels << "x" << nLines << std::endl;
                #if ( GDAL_VERSION_MAJOR >= 3 || ( GDAL_VERSION_MAJOR >= 2 && GDAL_VERSION_MINOR >= 2 ) )
                  poSrcOvrDS = GDALCreateOverviewDataset( poSrcDS, iOvr, FALSE );
                #else
                  poSrcOvrDS = GDALCreateOverviewDataset( poSrcDS, iOvr, FALSE, FALSE );
                #endif
                }
            }
        }
    }

  return static_cast<GDALDatasetH>(poSrcOvrDS);
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
GDALTiler::createRasterTile(GDALDataset *dataset, double (&adfGeoTransform)[6]) const {
  if (dataset == NULL) {
    throw CTBException("No GDAL dataset is set");
  }

  // The source and sink datasets
  GDALDatasetH hSrcDS = (GDALDatasetH) dataset;
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
  psWarpOptions->eResampleAlg = options.resampleAlg;
  psWarpOptions->dfWarpMemoryLimit = options.warpMemoryLimit;
  psWarpOptions->hSrcDS = hSrcDS;
  psWarpOptions->nBandCount = poDataset->GetRasterCount();
  psWarpOptions->panSrcBands =
    (int *) CPLMalloc(sizeof(int) * psWarpOptions->nBandCount );
  psWarpOptions->panDstBands =
    (int *) CPLMalloc(sizeof(int) * psWarpOptions->nBandCount );

  psWarpOptions->padfSrcNoDataReal =
    (double *)CPLCalloc(psWarpOptions->nBandCount, sizeof(double));
  psWarpOptions->padfSrcNoDataImag =
    (double *)CPLCalloc(psWarpOptions->nBandCount, sizeof(double));
  psWarpOptions->padfDstNoDataReal =
    (double *)CPLCalloc(psWarpOptions->nBandCount, sizeof(double));
  psWarpOptions->padfDstNoDataImag =
    (double *)CPLCalloc(psWarpOptions->nBandCount, sizeof(double));

  for (short unsigned int i = 0; i < psWarpOptions->nBandCount; ++i) {
    int bGotNoData = FALSE;
    double noDataValue = poDataset->GetRasterBand(i + 1)->GetNoDataValue(&bGotNoData);
    if (!bGotNoData) noDataValue = -32768;

    psWarpOptions->padfSrcNoDataReal[i] = noDataValue;
    psWarpOptions->padfSrcNoDataImag[i] = 0;
    psWarpOptions->padfDstNoDataReal[i] = noDataValue;
    psWarpOptions->padfDstNoDataImag[i] = 0;

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

  // Try and get an overview from the source dataset that corresponds more
  // closely to the resolution of this tile.
  GDALDatasetH hWrkSrcDS = getOverviewDataset(hSrcDS, GDALGenImgProjTransform, transformerArg);
  if (hWrkSrcDS == NULL) {
    hWrkSrcDS = psWarpOptions->hSrcDS = hSrcDS;
  } else {
    psWarpOptions->hSrcDS = hWrkSrcDS;

    // We need to recreate the transform when operating on an overview.
    GDALDestroyGenImgProjTransformer( transformerArg );

    transformerArg = GDALCreateGenImgProjTransformer2( hWrkSrcDS, NULL, transformOptions.List() );
    if(transformerArg == NULL) {
      GDALDestroyWarpOptions(psWarpOptions);
      throw CTBException("Could not create overview image to image transformer");
    }

    // Specify the destination geotransform
    GDALSetGenImgProjTransformerDstGeoTransform(transformerArg, adfGeoTransform );
  }

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

  // The raster tile is represented as a VRT dataset
  hDstDS = GDALCreateWarpedVRT(hWrkSrcDS, mGrid.tileSize(), mGrid.tileSize(), adfGeoTransform, psWarpOptions);

  bool isApproxTransform = (psWarpOptions->pfnTransformer == GDALApproxTransform);
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
                      isApproxTransform
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
