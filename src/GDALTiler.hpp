#ifndef GDALTILER_HPP
#define GDALTILER_HPP

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
 * @file GDALTiler.hpp
 * @brief This declares the `GDALTiler` class
 */

#include <string>
#include <cmath>                // for `abs()`

#include "gdal_priv.h"
#include "gdalwarper.h"

#include "TileCoordinate.hpp"
#include "GlobalGeodetic.hpp"
#include "TerrainTile.hpp"
#include "Bounds.hpp"

namespace ctb {
  class GDALTiler;
}

/**
 * @brief Create raster tiles from a GDAL Dataset
 *
 * This class is associated with a GDAL dataset from which it determines the
 * maximum zoom level (see `GDALTiler::maxZoomLevel`) and tile extents for a
 * particular zoom level (see `GDALTiler::tileBoundsForZoom`).  This
 * information can be used to create `TileCoordinate` instances which can be
 * used to create raster representations of a tile coverage (see
 * `GDALTiler::createRasterTile`).
 *
 * The GDAL dataset assigned to the tiler has its reference count incremented
 * when a tiler is instantiated or copied, meaning that the dataset is shared
 * with any other handles that may also be in use.  When the tiler is destroyed
 * the reference count is decremented and, if it reaches `0`, the dataset is
 * closed.
 */
class ctb::GDALTiler {
public:

  /// Instantiate a tiler with an empty GDAL dataset
  GDALTiler():
    mGrid(GlobalGeodetic()),
    poDataset(NULL)
  {}

  /// Instantiate a tiler with a GDAL dataset
  GDALTiler(GDALDataset *poDataset, const Grid &grid);

  /// The const copy constructor
  GDALTiler(const GDALTiler &other);

  /// The non const copy constructor
  GDALTiler(GDALTiler &other);

  /// Overload the assignment operator
  GDALTiler &
  operator=(const GDALTiler &other);

  /// The destructor
  ~GDALTiler();

  /// Create a raster tile from a tile coordinate
  virtual GDALDatasetH
  createRasterTile(const TileCoordinate &coord) const;

  /// Get the maximum zoom level for the dataset
  inline i_zoom
  maxZoomLevel() const {
    return mGrid.zoomForResolution(resolution());
  }

  /// Get the lower left tile for a particular zoom level
  inline TileCoordinate
  lowerLeftTile(i_zoom zoom) const {
    return mGrid.crsToTile(mBounds.getLowerLeft(), zoom);
  }

  /// Get the upper right tile for a particular zoom level
  inline TileCoordinate
  upperRightTile(i_zoom zoom) const {
    return mGrid.crsToTile(mBounds.getUpperRight(), zoom);
  }

  /// Get the tile bounds for a particular zoom level
  inline TileBounds
  tileBoundsForZoom(i_zoom zoom) const {
    TileCoordinate ll = mGrid.crsToTile(mBounds.getLowerLeft(), zoom),
      ur = mGrid.crsToTile(mBounds.getUpperRight(), zoom);

    return TileBounds(ll, ur);
  }

  /// Get the resolution of the underlying GDAL dataset
  inline double
  resolution() const {
    return mResolution;
  }

  /// Get the associated GDAL dataset
  inline GDALDataset *
  dataset() const {
    return poDataset;
  }

  /// Get the associated grid
  inline const Grid &
  grid() const {
    return mGrid;
  }

  /// Get the dataset bounds in EPSG:4326 coordinates
  inline const CRSBounds &
  bounds() const {
    return const_cast<const CRSBounds &>(mBounds);
  }

  /// Does the dataset require reprojecting to EPSG:4326?
  inline bool
  requiresReprojection() const {
    return crsWKT.size() > 0;
  }

protected:
  /// Close the underlying dataset
  void closeDataset();

  /// Create a raster tile from a tile coordinate and geo transform
  GDALDatasetH
  createRasterTile(double (&adfGeoTransform)[6]) const;

  /// The grid used for generating tiles
  Grid mGrid;

  /// The dataset from which to generate tiles
  GDALDataset *poDataset;

  /// The extent of the underlying dataset in latitude and longitude
  CRSBounds mBounds;

  /// The cell resolution of the underlying dataset
  double mResolution;

  /**
   * @brief The dataset projection in Well Known Text format
   *
   * This is only set if the underlying dataset does not match the coordinate
   * reference system of the grid being used.
   */
  std::string crsWKT;
};

#endif /* GDALTILER_HPP */
