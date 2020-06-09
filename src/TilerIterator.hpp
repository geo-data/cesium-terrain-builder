#ifndef TILERITERATOR_HPP
#define TILERITERATOR_HPP

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
 * @file TilerIterator.hpp
 * @brief This declares and defines the `TilerIterator` class
 */

#include "GridIterator.hpp"
#include "GDALTiler.hpp"

namespace ctb {
  class TilerIterator;
}

/**
 * @brief Forward iterate over tiles in a `GDALTiler`
 *
 * Instances of this class take a `GDALTiler` (or derived class) in the
 * constructor and are used to forward iterate over all tiles in the tiler,
 * returning a `Tile *` when dereferenced.  It is the caller's responsibility to
 * call `delete` on the tile.
 */
class ctb::TilerIterator :
  public GridIterator
{
public:

  /// Instantiate an iterator with a tiler
  TilerIterator(const GDALTiler &tiler) :
    TilerIterator(tiler, tiler.maxZoomLevel(), 0)
  {}

  TilerIterator(const GDALTiler &tiler, i_zoom startZoom, i_zoom endZoom = 0) :
    GridIterator(tiler.grid(), tiler.bounds(), startZoom, endZoom),
    tiler(tiler)
  {}

  /// Override the dereference operator to return a Tile
  virtual Tile *
  operator*() const {
    return tiler.createTile(tiler.dataset(), *(GridIterator::operator*()));
  }

protected:

  const GDALTiler &tiler;              ///< The tiler we are iterating over
};

#endif /* TILERITERATOR_HPP */
