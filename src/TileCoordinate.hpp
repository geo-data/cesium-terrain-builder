#ifndef TILECOORDINATE_HPP
#define TILECOORDINATE_HPP

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
 * @file TileCoordinate.hpp
 * @brief This declares and defines the `TileCoordinate` class
 */

#include "types.hpp"

namespace ctb {
  class TileCoordinate;
}

/**
 * @brief A `TileCoordinate` identifies a particular tile
 *
 * An instance of this class is composed of a tile point and a zoom level:
 * together this identifies an individual tile.
 */
class ctb::TileCoordinate:
  public TilePoint {
public:

  /// Create the 0-0-0 level tile coordinate
  TileCoordinate():
    TilePoint(0, 0),
    zoom(0)
  {}

  /// The const copy constructor
  TileCoordinate(const TileCoordinate &other):
    TilePoint(other.x, other.y),
    zoom(other.zoom)
  {}

  /// Instantiate a tile coordinate from the zoom, x and y
  TileCoordinate(i_zoom zoom, i_tile x, i_tile y):
    TilePoint(x, y),
    zoom(zoom)
  {}

  /// Instantiate a tile coordinate using the zoom and a tile point
  TileCoordinate(i_zoom zoom, const TilePoint &coord):
    TilePoint(coord),
    zoom(zoom)
  {}

  /// Override the equality operator
  inline bool
  operator==(const TileCoordinate &other) const {
    return TilePoint::operator==(other)
      && zoom == other.zoom;
  }

  /// Override the assignment operator
  inline void
  operator=(const TileCoordinate &other) {
    TilePoint::operator=(other);
    zoom = other.zoom;
  }

  /// Set the point
  inline void
  setPoint(const TilePoint &point) {
    TilePoint::operator=(point);
  }

  i_zoom zoom;                  ///< The zoom level
};

#endif /* TILECOORDINATE_HPP */
