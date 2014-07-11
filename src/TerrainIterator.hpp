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
#include "TilerIterator.hpp"

namespace ctb {
  class TerrainIterator;
}

/**
 * @brief This forward iterates over all `TerrainTile`s in a `TerrainTiler`
 *
 * Instances of this class take a `TerrainTiler` in the constructor to provide
 * a collection of `TerrainTile`s.  Deriving from `TileIterator`, the class
 * overrides the `operator*` method to return a `TerrainTile`.
 */
class ctb::TerrainIterator :
  public TilerIterator< TerrainTile, const TerrainTiler & >
{
public:

  /// Instantiate an iterator with a tiler
  TerrainIterator(const TerrainTiler &tiler);

  TerrainIterator(const TerrainTiler &tiler, i_zoom startZoom, i_zoom endZoom = 0);

  /// Override the dereference operator to return a `TerrainTile`
  TerrainTile
  operator*() const;
};

#endif /* TERRAINITERATOR_HPP */
