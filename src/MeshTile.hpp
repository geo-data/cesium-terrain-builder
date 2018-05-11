#ifndef MESHTILE_HPP
#define MESHTILE_HPP

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
 * @file MeshTile.hpp
 * @brief This declares the `MeshTile` class
 * @author Alvaro Huarte <ahuarte47@yahoo.es>
 */

#include "config.hpp"
#include "Mesh.hpp"
#include "TileCoordinate.hpp"
#include "Tile.hpp"
#include "CTBOutputStream.hpp"

namespace ctb {
  class MeshTile;
}

/**
 * @brief `Terrain` data associated with a `Mesh`
 *
 * This aims to implement the Cesium [quantized-mesh-1.0 terrain
 * format](https://github.com/AnalyticalGraphicsInc/quantized-mesh).
 */
class CTB_DLL ctb::MeshTile :
  public Tile
{
  friend class MeshTiler;

public:

  /// Create an empty mesh tile object
  MeshTile();

  /// Create a mesh tile from a tile coordinate
  MeshTile(const TileCoordinate &coord);

  /// Write terrain data to the filesystem
  void
  writeFile(const char *fileName, bool writeVertexNormals = false) const;

  /// Write terrain data to an output stream
  void
  writeFile(CTBOutputStream &ostream, bool writeVertexNormals = false) const;

  /// Does the terrain tile have child tiles?
  bool
  hasChildren() const;

  /// Does the terrain tile have a south west child tile?
  bool
  hasChildSW() const;

  /// Does the terrain tile have a south east child tile?
  bool
  hasChildSE() const;

  /// Does the terrain tile have a north west child tile?
  bool
  hasChildNW() const;

  /// Does the terrain tile have a north east child tile?
  bool
  hasChildNE() const;

  /// Specify that there is a south west child tile
  void
  setChildSW(bool on = true);

  /// Specify that there is a south east child tile
  void
  setChildSE(bool on = true);

  /// Specify that there is a north west child tile
  void
  setChildNW(bool on = true);

  /// Specify that there is a north east child tile
  void
  setChildNE(bool on = true);

  /// Specify that all child tiles are present
  void
  setAllChildren(bool on = true);

  /// Get the mesh data as a const object
  const ctb::Mesh & getMesh() const;

  /// Get the mesh data
  ctb::Mesh & getMesh();

protected:

  /// The terrain mesh data
  ctb::Mesh mMesh;

private:

  char mChildren;               ///< The child flags

  /**
   * @brief Bit flags defining child tile existence
   *
   * There is a good discussion on bitflags
   * [here](http://www.dylanleigh.net/notes/c-cpp-tricks.html#Using_"Bitflags").
   */
  enum Children {
    TERRAIN_CHILD_SW = 1,       // 2^0, bit 0
    TERRAIN_CHILD_SE = 2,       // 2^1, bit 1
    TERRAIN_CHILD_NW = 4,       // 2^2, bit 2
    TERRAIN_CHILD_NE = 8        // 2^3, bit 3
  };
};

#endif /* MESHTILE_HPP */
