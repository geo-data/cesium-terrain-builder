#include <stdio.h>
#include <iostream>
#include <sstream>
#include "gdal_priv.h"
#include "src/GDALTiler.hpp"

using namespace std;

void writeTiles(GDALTiler &tiler) {
  int tminx, tminy, tmaxx, tmaxy;
  short int maxZoom = tiler.maxZoomLevel();

  for (short int zoom = maxZoom; zoom >= 0; zoom--) {
    tiler.lowerLeftTile(zoom, tminx, tminy);
    tiler.upperRightTile(zoom, tmaxx, tmaxy);

    for (int tx = tminx; tx <= tmaxx; tx++) {
      for (int ty = tminy; ty <= tmaxy; ty++) {
        TerrainTile *terrainTile = tiler.createTerrainTile(zoom, tx, ty);
        string filename = "tiles/" + static_cast<ostringstream*>( &(ostringstream() << zoom << "-" << tx << "-" << ty << ".terrain") )->str();
        cout << "creating " << filename << endl;

        try {
          terrainTile->writeFile(filename.c_str());
        } catch (int e) {
          switch(e) {
          case 1:
            cerr << "Failed to open " << filename << endl;
            break;
          case 2:
            cerr << "Failed to write height data" << endl;
            break;
          case 3:
            cerr << "Failed to write child flags" << endl;
            break;
          case 4:
            cerr << "Failed to write water mask" << endl;
            break;
          case 5:
            cerr << "Failed to close file" << endl;
            break;
          }
        }
        delete terrainTile;
      }
    }
  }
}

int main(int argc, char** argv) {
  char *fileName = argv[1];

  GDALAllRegister();

  GDALDataset  *poDataset = (GDALDataset *) GDALOpen(fileName, GA_ReadOnly);
  GDALTiler tiler(poDataset);

  writeTiles(tiler);

  return 0;
}
