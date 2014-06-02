#include <stdio.h>
#include <iostream>
#include "src/TerrainTile.hpp"

#include "gdal_priv.h"

int main(int argc, char** argv) {
  TerrainTile terrain;
  FILE *terrainIn = fopen(argv[1], "rb");

  GDALAllRegister();

  try {
    terrain = TerrainTile(terrainIn);
  } catch (int e) {
    switch (e) {
    case 1:
      printf("Failed to read child tiles byte\n");
      return 1;
    case 2:
      printf("Failed to read water mask\n");
      return 2;
    default:
      printf("Unknown error: %d\n", e);
      return 3;
    }

    fclose(terrainIn);
    return e;
  }
  fclose(terrainIn);

  if (terrain.hasChildren()) {
    if (terrain.hasChildSW()) {
      printf("Has a SW child\n");
    }
    if (terrain.hasChildSE()) {
      printf("Has a SE child\n");
    }
    if (terrain.hasChildNW()) {
      printf("Has a NW child\n");
    }
    if (terrain.hasChildNE()) {
      printf("Has a NE child\n");
    }
  } else {
    printf("Doesn't have children\n");
  }

  if (terrain.hasWaterMask()) {
    printf("The tile has a water mask\n");
  } else if (terrain.isLand()) {
    printf("The tile is land\n");
  } else if (terrain.isWater()) {
    printf("The tile is water\n");
  } else {
    // should not get here!!
    printf("Unknown tile type!!\n");
  }

  return 0;
}
