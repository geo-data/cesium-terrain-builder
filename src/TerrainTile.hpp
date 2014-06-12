#ifndef TERRAINTILE_HPP
#define TERRAINTILE_HPP

#include <vector>

#include "gdal_priv.h"

#define TILE_SIZE 65 * 65
#define MASK_SIZE 256 * 256

// The maximum byte size of an uncompressed terrain tile (heights + child flags
// + water mask)
#define MAX_TERRAIN_SIZE ( TILE_SIZE * 2 ) + 1 + MASK_SIZE

enum TerrainChildren {
  TC_SW = 1,                    // 2^0, bit 0
  TC_SE = 2,                    // 2^1, bit 1
  TC_NW = 4,                    // 2^2, bit 2
  TC_NE = 8                     // 2^3, bit 3
};

class TerrainTile {
public:
  TerrainTile();
  TerrainTile(const char *fileName);
  TerrainTile(FILE *fp);

  void writeFile(FILE *fp);
  void writeFile(const char *fileName);
  std::vector<bool> mask();

  bool hasChildren();
  bool hasChildSW();
  bool hasChildSE();
  bool hasChildNW();
  bool hasChildNE();

  void setChildSW(bool on = true);
  void setChildSE(bool on = true);
  void setChildNW(bool on = true);
  void setChildNE(bool on = true);

  void setAllChildren(bool on = true);

  void setIsWater();
  bool isWater();

  void setIsLand();
  bool isLand();
  bool hasWaterMask();

  GDALDatasetH heightsToRaster(double minx, double miny, double maxx, double maxy);

  std::vector<short int> mHeights;

private:
  char mChildren;
  char mMask[MASK_SIZE];
  size_t mMaskLength;
};

#endif /* TERRAINTILE_HPP */
