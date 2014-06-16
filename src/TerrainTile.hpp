#ifndef TERRAINTILE_HPP
#define TERRAINTILE_HPP

#include <vector>

#include "gdal_priv.h"

#include "config.hpp"
#include "TileCoordinate.hpp"

namespace terrain {
  class Terrain;
  class TerrainTile;
  class GDALTiler;
}

class terrain::Terrain {
public:
  Terrain();
  Terrain(const char *fileName);
  Terrain(FILE *fp);

  void readFile(const char *fileName);
  
  void writeFile(FILE *fp) const;
  void writeFile(const char *fileName) const;
  std::vector<bool> mask();

  bool hasChildren() const;
  bool hasChildSW() const;
  bool hasChildSE() const;
  bool hasChildNW() const;
  bool hasChildNE() const;

  void setChildSW(bool on = true);
  void setChildSE(bool on = true);
  void setChildNW(bool on = true);
  void setChildNE(bool on = true);

  void setAllChildren(bool on = true);

  void setIsWater();
  bool isWater() const;

  void setIsLand();
  bool isLand() const;
  bool hasWaterMask() const;

  const std::vector<terrain::i_terrain_height> &
  getHeights() const;

  std::vector<terrain::i_terrain_height> &
  getHeights();

protected:
  std::vector<terrain::i_terrain_height> mHeights; // replace with `std::array` in C++11

  static const unsigned short int TILE_CELL_SIZE = TILE_SIZE * TILE_SIZE;
  static const unsigned int MASK_CELL_SIZE = MASK_SIZE * MASK_SIZE;

  // The maximum byte size of an uncompressed terrain tile (heights + child
  // flags + water mask)
  static const unsigned int MAX_TERRAIN_SIZE = (TILE_CELL_SIZE * 2) + 1 + MASK_CELL_SIZE;
private:

  char mChildren;
  char mMask[MASK_CELL_SIZE];
  size_t mMaskLength;

  enum Children {
    TERRAIN_CHILD_SW = 1,       // 2^0, bit 0
    TERRAIN_CHILD_SE = 2,       // 2^1, bit 1
    TERRAIN_CHILD_NW = 4,       // 2^2, bit 2
    TERRAIN_CHILD_NE = 8        // 2^3, bit 3
  };
};

class terrain::TerrainTile :
  public Terrain
{
  friend class terrain::GDALTiler;

public:
  TerrainTile(terrain::TileCoordinate coord);
  TerrainTile(const char *fileName, terrain::TileCoordinate coord);
  TerrainTile(const terrain::Terrain &terrain, terrain::TileCoordinate coord);

  GDALDatasetH heightsToRaster() const;
  
  terrain::TileCoordinate & getCoordinate() {
    return coord;
  }

  const terrain::TileCoordinate & getCoordinate() const {
    return const_cast<const terrain::TileCoordinate &>(coord);
  }


private:
  terrain::TileCoordinate coord;
};

#endif /* TERRAINTILE_HPP */
