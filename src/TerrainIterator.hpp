#ifndef TERRAINITERATOR_HPP
#define TERRAINITERATOR_HPP

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
 * @file TerrainIterator.hpp
 * @brief This declares the `TerrainIterator` class
 */

#include "TerrainTile.hpp"
#include "TerrainTiler.hpp"
#include "RasterIterator.hpp"

namespace ctb {
  class TerrainIterator;
}

/**
 * @brief This forward iterates over all `TerrainTile`s in a `TerrainTiler`
 *
 * Instances of this class take a `TerrainTiler` in the constructor to provide a
 * collection of `TerrainTile`s.  Deriving from `TileIterator`, this returns a
 * `Tile *` when dereferenced.  This can be cast to a `TerrainTile`.  It is the
 * caller's responsibility to call `delete` on the tile.
 */
class ctb::TerrainIterator :
  public RasterIterator
{
public:

  /// Instantiate an iterator with a tiler
  TerrainIterator(const TerrainTiler &tiler):
    RasterIterator(tiler) {}

  TerrainIterator(const TerrainTiler &tiler, i_zoom startZoom, i_zoom endZoom = 0):
    RasterIterator(tiler, startZoom, endZoom) {}
};

#endif /* TERRAINITERATOR_HPP */
