#ifndef TERRAINTILE_HPP
#define TERRAINTILE_HPP

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
 * @file TerrainTile.hpp
 * @brief This declares the `Terrain` and `TerrainTile` classes
 */

#include <vector>

#include "gdal_priv.h"

#include "config.hpp"
#include "Tile.hpp"
#include "TileCoordinate.hpp"
#include "CTBOutputStream.hpp"

namespace ctb {
  class Terrain;
  class TerrainTile;
}

/**
 * @brief Model the terrain heightmap specification
 *
 * This aims to implement the Cesium [heightmap-1.0 terrain
 * format](http://cesiumjs.org/data-and-assets/terrain/formats/heightmap-1.0.html).
 */
class CTB_DLL ctb::Terrain {
public:

  /// Create an empty terrain object
  Terrain();

  /// Instantiate using terrain data on the file system
  Terrain(const char *fileName);

  /// Read terrain data from a file handle
  Terrain(FILE *fp);

  /// Read terrain data from the filesystem
  void
  readFile(const char *fileName);

  /// Write terrain data to a file handle
  void
  writeFile(FILE *fp) const;

  /// Write terrain data to the filesystem
  void
  writeFile(const char *fileName) const;

  /// Write terrain data to an output stream
  void
  writeFile(CTBOutputStream &ostream) const;

  /// Get the water mask as a boolean mask
  std::vector<bool>
  mask();

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

  /// Specify that this tile is all water
  void
  setIsWater();

  /// Is this tile all water?
  bool
  isWater() const;

  /// Specify that this tile is all land
  void
  setIsLand();

  /// Is this tile all land?
  bool
  isLand() const;

  /// Does this tile have a water mask?
  bool
  hasWaterMask() const;

  /// Get the height data as a const vector
  const std::vector<i_terrain_height> &
  getHeights() const;

  /// Get the height data as a vector
  std::vector<i_terrain_height> &
  getHeights();

protected:
  /// The terrain height data
  std::vector<i_terrain_height> mHeights; // replace with `std::array` in C++11

  /// The number of height cells within a terrain tile
  static const unsigned short int TILE_CELL_SIZE = TILE_SIZE * TILE_SIZE;

  /// The number of water mask cells within a terrain tile
  static const unsigned int MASK_CELL_SIZE = MASK_SIZE * MASK_SIZE;

  /**
   * @brief The maximum byte size of an uncompressed terrain tile
   *
   * This is calculated as (heights + child flags + water mask).
   */
  static const unsigned int MAX_TERRAIN_SIZE = (TILE_CELL_SIZE * 2) + 1 + MASK_CELL_SIZE;

private:

  char mChildren;               ///< The child flags
  char mMask[MASK_CELL_SIZE];   ///< The water mask
  size_t mMaskLength;           ///< What size is the water mask?

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

/**
 * @brief `Terrain` data associated with a `Tile`
 *
 * Associating terrain data with a tile coordinate allows the tile to be
 * converted to a geo-referenced raster (see `TerrainTile::heightsToRaster`).
 */
class CTB_DLL ctb::TerrainTile :
  public Terrain, public Tile
{
  friend class TerrainTiler;

public:

  /// Create a terrain tile from a tile coordinate
  TerrainTile(const TileCoordinate &coord);

  /// Create a terrain tile from a file
  TerrainTile(const char *fileName, const TileCoordinate &coord);

  /// Create a terrain tile from terrain data
  TerrainTile(const Terrain &terrain, const TileCoordinate &coord);

  /// Get the height data as an in memory GDAL raster
  GDALDatasetH
  heightsToRaster() const;
};

#endif /* TERRAINTILE_HPP */
