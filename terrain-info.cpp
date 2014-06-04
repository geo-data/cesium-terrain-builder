#include <stdio.h>
#include <iostream>
#include "src/TerrainTile.hpp"

#include "gdal_priv.h"

using namespace std;

int main(int argc, char** argv) {
  TerrainTile terrain;
  char *fileName = argv[1];

  GDALAllRegister();

  try {
    terrain = TerrainTile(fileName);
  } catch (int e) {
    switch (e) {
    case 1:
      cerr << "Failed to open " << fileName << endl;
      return 1;
    case 2:
      cerr << "The file has too many bytes" << endl;
      return 2;
    case 3:
      cerr << "The file does not appear to be a terrain tile" << endl;
      return 3;
    default:
      cerr << "Unknown error: " << e << endl;
      return 4;
    }

    return e;
  }

  cout << "Heights:";
  for (short int i = 0; i < TILE_SIZE; i++) {
    if (i % 65 == 0) cout << endl;
    cout << terrain.mHeights[i] << " ";
  }
  cout << endl;

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
