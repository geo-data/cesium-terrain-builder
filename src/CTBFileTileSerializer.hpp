#ifndef CTBFILETILESERIALIZER_HPP
#define CTBFILETILESERIALIZER_HPP

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
 * @file CTBFileTileSerializer.hpp
 * @brief This declares and defines the `CTBFileTileSerializer` class
 */

#include <string>

#include "TileCoordinate.hpp"
#include "GDALSerializer.hpp"
#include "TerrainSerializer.hpp"
#include "MeshSerializer.hpp"

namespace ctb {
  class CTBFileTileSerializer;
}

/// Implements a serializer of `Tile`s based in a directory of files
class CTB_DLL ctb::CTBFileTileSerializer : 
  public ctb::GDALSerializer,
  public ctb::TerrainSerializer, 
  public ctb::MeshSerializer {
public:
  CTBFileTileSerializer(const std::string &outputDir, bool resume):
    moutputDir(outputDir), 
    mresume(resume) {}

  /// Start a new serialization task
  virtual void startSerialization() {};

  /// Returns if the specified Tile Coordinate should be serialized
  virtual bool mustSerializeCoordinate(const ctb::TileCoordinate *coordinate);

  /// Serialize a GDALTile to the store
  virtual bool serializeTile(const ctb::GDALTile *tile, GDALDriver *driver, const char *extension, CPLStringList &creationOptions);
  /// Serialize a TerrainTile to the store
  virtual bool serializeTile(const ctb::TerrainTile *tile);
  /// Serialize a MeshTile to the store
  virtual bool serializeTile(const ctb::MeshTile *tile, bool writeVertexNormals = false);

  /// Serialization finished, releases any resources loaded
  virtual void endSerialization() {};


  /// Create a filename for a tile coordinate
  static std::string
  getTileFilename(const TileCoordinate *coord, const std::string dirname, const char *extension);

protected:
  /// The target directory where serializing
  std::string moutputDir;
  /// Do not overwrite existing files
  bool mresume;
};

#endif /* CTBFILETILESERIALIZER_HPP */
