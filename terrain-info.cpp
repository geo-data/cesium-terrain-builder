#include <stdio.h>
#include <iostream>
#include "src/TerrainTile.hpp"

#include "gdal_priv.h"

using namespace std;

int main(int argc, char** argv) {
  TerrainTile terrain;
  FILE *terrainIn = fopen(argv[1], "rb");

  GDALAllRegister();

  try {
    terrain = TerrainTile(terrainIn);
  } catch (int e) {
    switch (e) {
    case 1:
      cerr << "Failed to read child tiles byte" << endl;
      return 1;
    case 2:
      cerr << "Failed to read water mask" << endl;
      return 2;
    default:
      cerr << "Unknown error: " << e << endl;
      return 3;
    }

    fclose(terrainIn);
    return e;
  }
  fclose(terrainIn);

  if (terrain.hasChildren()) {
    if (terrain.hasChildSW()) {
      cout << "Has a SW child" << endl;
    }
    if (terrain.hasChildSE()) {
      cout << "Has a SE child" << endl;
    }
    if (terrain.hasChildNW()) {
      cout << "Has a NW child" << endl;
    }
    if (terrain.hasChildNE()) {
      cout << "Has a NE child" << endl;
    }
  } else {
    cout << "Doesn't have children" << endl;
  }

  if (terrain.hasWaterMask()) {
    cout << "The tile has a water mask" << endl;
  } else if (terrain.isLand()) {
    cout << "The tile is land" << endl;
  } else if (terrain.isWater()) {
    cout << "The tile is water" << endl;
  } else {
    // should not get here!!
    cerr << "Unknown tile type!!" << endl;
  }

  return 0;
}
