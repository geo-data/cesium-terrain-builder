#ifndef TERRAINSERIALIZER_HPP
#define TERRAINSERIALIZER_HPP

/*******************************************************************************
 * Copyright 2018 GeoData <geodata@soton.ac.uk>
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
 * @file TerrainSerializer.hpp
 * @brief This declares and defines the `TerrainSerializer` class
 */

#include "config.hpp"
#include "TileCoordinate.hpp"
#include "TerrainTile.hpp"

namespace ctb {
  class TerrainSerializer;
}

/// Store `TerrainTile`s from a GDAL Dataset
class CTB_DLL ctb::TerrainSerializer {
public:

  /// Start a new serialization task
  virtual void startSerialization() = 0;

  /// Returns if the specified Tile Coordinate should be serialized
  virtual bool mustSerializeCoordinate(const ctb::TileCoordinate *coordinate) = 0;

  /// Serialize a TerrainTile to the store
  virtual bool serializeTile(const ctb::TerrainTile *tile) = 0;

  /// Serialization finished, releases any resources loaded
  virtual void endSerialization() = 0;
};

#endif /* TERRAINSERIALIZER_HPP */
