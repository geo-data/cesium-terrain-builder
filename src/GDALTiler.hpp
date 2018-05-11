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
#include "gdalwarper.h"

#include "TileCoordinate.hpp"
#include "GlobalGeodetic.hpp"
#include "GDALTile.hpp"
#include "Bounds.hpp"

namespace ctb {
  struct TilerOptions;
  class GDALTiler;
  class GDALDatasetReader; // forward declaration
}

/// Options passed to a `GDALTiler`
struct ctb::TilerOptions {
  /// The error threshold in pixels passed to the approximation transformer
  float errorThreshold = 0.125; // the `gdalwarp` default
  /// The memory limit of the warper in bytes
  double warpMemoryLimit = 0.0; // default to GDAL internal setting
  /// The warp resampling algorithm
  GDALResampleAlg resampleAlg = GRA_Average; // recommended by GDAL maintainer
};

/**
 * @brief Create raster tiles from a GDAL Dataset
 *
 * This abstract base class is associated with a GDAL dataset from which it
 * determines the maximum zoom level (see `GDALTiler::maxZoomLevel`) and tile
 * extents for a particular zoom level (see `GDALTiler::tileBoundsForZoom`).
 * This information can be used to create `TileCoordinate` instances which in
 * turn can be used to create raster representations of a tile coverage (see
 * `GDALTiler::createRasterTile`).  This mechanism is intended to be leveraged
 * by derived classes to override the `GDALTiler::createTile` method.
 *
 * The GDAL dataset assigned to the tiler has its reference count incremented
 * when a tiler is instantiated or copied, meaning that the dataset is shared
 * with any other handles that may also be in use.  When the tiler is destroyed
 * the reference count is decremented and, if it reaches `0`, the dataset is
 * closed.
 */
class CTB_DLL ctb::GDALTiler {
public:

  /// Instantiate a tiler with all required arguments
  GDALTiler(GDALDataset *poDataset, const Grid &grid, const TilerOptions &options);

  /// Instantiate a tiler with an empty GDAL dataset
  GDALTiler():
    GDALTiler(NULL, GlobalGeodetic()) {}

  /// Instantiate a tiler with a dataset and grid but no options
  GDALTiler(GDALDataset *poDataset, const Grid &grid):
    GDALTiler(poDataset, grid, TilerOptions()) {}

  /// The const copy constructor
  GDALTiler(const GDALTiler &other);

  /// The non const copy constructor
  GDALTiler(GDALTiler &other);

  /// Overload the assignment operator
  GDALTiler &
  operator=(const GDALTiler &other);

  /// The destructor
  ~GDALTiler();

  /// Create a tile from a tile coordinate
  virtual Tile *
  createTile(GDALDataset *dataset, const TileCoordinate &coord) const = 0;

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
  friend class GDALDatasetReader;

  /// Close the underlying dataset
  void closeDataset();

  /// Create a raster tile from a tile coordinate
  virtual GDALTile *
  createRasterTile(GDALDataset *dataset, const TileCoordinate &coord) const;

  /// Create a raster tile from a geo transform
  virtual GDALTile *
  createRasterTile(GDALDataset *dataset, double (&adfGeoTransform)[6]) const;

  /// The grid used for generating tiles
  Grid mGrid;

  /// The dataset from which to generate tiles
  GDALDataset *poDataset;

  TilerOptions options;

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
