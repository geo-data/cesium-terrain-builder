#ifndef TILECOORDINATEITERATOR_HPP
#define TILECOORDINATEITERATOR_HPP

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
 * @file TileCoordinateIterator.hpp
 * @brief This declares and defines the `TileCoordinateIterator` class
 */

#include "GridIterator.hpp"

namespace ctb {
  class TileCoordinateIterator;
}

/// A `GridIterator` which forward iterates over `TileCoordinates` in a `Grid`
class ctb::TileCoordinateIterator :
  public GridIterator<TileCoordinate>
{
public:

  /// Instantiate an iterator with a grid
  TileCoordinateIterator(const Grid &grid, i_zoom startZoom, i_zoom endZoom = 0) :
    GridIterator(grid, startZoom, endZoom)
  {}

  /// Instantiate an iterator with a grid and separate bounds
  TileCoordinateIterator(const Grid &grid, const CRSBounds &extent, i_zoom startZoom, i_zoom endZoom = 0) :
    GridIterator(grid, extent, startZoom, endZoom)
  {}

  /// Override the dereference operator to return a tile
  TileCoordinate
  operator*() const {
    return currentTile;
  }
};

#endif /* TILECOORDINATEITERATOR_HPP */
