#ifndef RASTERITERATOR_HPP
#define RASTERITERATOR_HPP

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
 * @file RasterIterator.hpp
 * @brief This declares the `RasterIterator` class
 */

#include "gdal_priv.h"

#include "TilerIterator.hpp"
#include "RasterTiler.hpp"

namespace ctb {
  class RasterIterator;
}

/**
 * @brief This forward iterates over all tiles in a `RasterTiler`
 *
 * Instances of this class take a `RasterTiler` in the constructor and are used
 * to forward iterate over all tiles in the tiler, returning a `GDALTile *` when
 * dereferenced.  It is the caller's responsibility to call `delete` on the
 * tile.
 */
class ctb::RasterIterator :
  public TilerIterator
{
public:

  /// Instantiate an iterator with a tiler
  RasterIterator(const RasterTiler &tiler):
    RasterIterator(tiler, tiler.maxZoomLevel(), 0)
  {}

  /// The target constructor
  RasterIterator(const RasterTiler &tiler, i_zoom startZoom, i_zoom endZoom):
    TilerIterator(tiler, startZoom, endZoom)
  {}

  virtual GDALTile *
  operator*() const override {
    return static_cast<GDALTile *>(TilerIterator::operator*());
  }
};

#endif /* RASTERITERATOR_HPP */
