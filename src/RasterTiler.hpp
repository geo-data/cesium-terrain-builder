#ifndef RASTERTILER_HPP
#define RASTERTILER_HPP

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
 * @file RasterTiler.hpp
 * @brief This declares and defines the `RasterTiler` class
 */

#include "GDALTiler.hpp"

namespace ctb {
  class RasterTiler;
}

class ctb::RasterTiler :
  public ctb::GDALTiler
{
public:

  /// Instantiate a tiler with all required arguments
  RasterTiler(GDALDataset *poDataset, const Grid &grid, const TilerOptions &options):
    GDALTiler(poDataset, grid, options) {}

  /// Instantiate a tiler with an empty GDAL dataset
  RasterTiler():
    GDALTiler() {}

  /// Instantiate a tiler with a dataset and grid but no options
  RasterTiler(GDALDataset *poDataset, const Grid &grid):
    RasterTiler(poDataset, grid, TilerOptions()) {}

  /// Overload the assignment operator
  RasterTiler &
  operator=(const RasterTiler &other) {
    GDALTiler::operator=(other);
    return *this;
  }

  /// Override to return a covariant data type
  virtual GDALTile *
  createTile(GDALDataset *dataset, const TileCoordinate &coord) const override {
    return createRasterTile(dataset, coord);
  }
};

#endif /* RASTERTILER_HPP */
