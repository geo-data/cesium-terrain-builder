#ifndef CTBTILE_HPP
#define CTBTILE_HPP

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
 * @file Tile.hpp
 * @brief This declares the `Tile` class
 */

#include "config.hpp"
#include "TileCoordinate.hpp"

namespace ctb {
  class Tile;
}

/**
 * @brief An abstract base class for a tile
 *
 * This provides a way of associating a `TileCoordinate` with tile data.
 */
class ctb::Tile {
public:
  virtual ~Tile () = 0;         // this is an abstract base class

  /// Create an empty tile
  Tile():
    coord() {}

  /// Create a tile from a tile coordinate
  Tile(TileCoordinate coord):
    coord(coord) {}

  /// Get the tile coordinate associated with this tile
  inline TileCoordinate &
  getCoordinate() {
    return coord;
  }

  /// Get the const coordinate associated with this tile
  inline const TileCoordinate &
  getCoordinate() const {
    return const_cast<const TileCoordinate &>(coord);
  }

protected:
  TileCoordinate coord;         ///< The coordinate for this terrain tile
};

inline ctb::Tile::~Tile() { }   // prevents linker errors

#endif /* CTBTILE_HPP */
