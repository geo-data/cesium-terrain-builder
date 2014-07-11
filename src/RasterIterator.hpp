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

#include <utility>              // for std::pair

#include "gdal_priv.h"

#include "TilerIterator.hpp"
#include "GDALTiler.hpp"

namespace ctb {
  class RasterIterator;
}

/**
 * @brief This forward iterates over all tiles in a `GDALTiler`
 *
 * Instances of this class take a `GDALTiler` in the constructor and are used
 * to forward iterate over all tiles in the tiler returning a `GDALDataset *`
 * when dereferenced.  It is the caller's responsibility to call `GDALClose()`
 * on the returned dataset.
 */
class ctb::RasterIterator :
  public TilerIterator< std::pair<const TileCoordinate &, GDALDataset *>, const GDALTiler & >
{
public:

  /// Instantiate an iterator with a tiler
  RasterIterator(const GDALTiler &tiler);

  RasterIterator(const GDALTiler &tiler, i_zoom startZoom, i_zoom endZoom = 0);

  /// Override the dereference operator to return a `GDALDataset *`
  std::pair<const TileCoordinate &, GDALDataset *>
  operator*() const;
};

#endif /* RASTERITERATOR_HPP */
