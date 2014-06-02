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
        //cout << "creating " << filename << endl;

        FILE *terrainOut = fopen(filename.c_str(),"wb");
        terrainTile->writeFile(terrainOut);
        fclose(terrainOut);
        delete terrainTile;
      }
    }
  }
}

int main() {
  GDALAllRegister();

  GDALDataset  *poDataset = (GDALDataset *) GDALOpen("./lidar-2007-filled-cut.tif", GA_ReadOnly);
  GDALTiler tiler(poDataset);

  writeTiles(tiler);

  /*int tx = 8146, ty = 6409;
  short int zoom = 13;

  GDALDatasetH hTileDS = tiler.createRasterTile(zoom, tx, ty);
  GDALDatasetH hDstDS;
  GDALDriverH hDriver = GDALGetDriverByName("GTiff");

  hDstDS = GDALCreateCopy( hDriver, "13-8146-6409.gdal.tif", hTileDS, FALSE,
                           NULL, NULL, NULL );

  // Once we're done, close properly the dataset
  if( hDstDS != NULL )
    GDALClose( hDstDS );
  GDALClose( hTileDS );

  TerrainTile *terrainTile = tiler.createTerrainTile(zoom, tx, ty);
  FILE *terrainOut = fopen("13-8146-6409.terrain", "wb");
  terrainTile->writeFile(terrainOut);
  delete terrainTile;
  fclose(terrainOut);
  */

  return 0;
}
