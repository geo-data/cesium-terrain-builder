#ifndef CRSBOUNDSITERATOR_HPP
#define CRSBOUNDSITERATOR_HPP

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
 * @file CRSBoundsIterator.hpp
 * @brief This declares and defines the `CRSBoundsIterator` class
 */

#include <utility>              // for std::pair

#include "GridIterator.hpp"

namespace ctb {
  class CRSBoundsIterator;
}

/// Forward iterate over tiles in a `Grid` where tiles are `CRSBounds`
class ctb::CRSBoundsIterator :
  public GridIterator< std::pair<TileCoordinate, CRSBounds> >
{
public:

  /// Instantiate an iterator with a grid
  CRSBoundsIterator(const Grid &grid, i_zoom startZoom, i_zoom endZoom = 0) :
    GridIterator(grid, startZoom, endZoom)
  {}

  /// Instantiate an iterator with a grid and separate bounds
  CRSBoundsIterator(const Grid &grid, const CRSBounds &extent, i_zoom startZoom, i_zoom endZoom = 0) :
    GridIterator(grid, extent, startZoom, endZoom)
  {}

  /// Override the dereference operator to return a tile
  std::pair<TileCoordinate, CRSBounds>
  operator*() const {
    return std::pair<TileCoordinate, CRSBounds>
      (
       currentTile,
       grid.tileBounds(currentTile)
       );
  }
};

#endif /* CRSBOUNDSITERATOR_HPP */
