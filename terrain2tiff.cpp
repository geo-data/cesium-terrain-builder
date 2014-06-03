#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include "src/TerrainTile.hpp"
#include "src/GlobalGeodetic.hpp"

#include "gdal_priv.h"

using namespace std;

void terrain2tiff(TerrainTile &terrain, char* filename, double minx, double miny, double maxx, double maxy) {
  GDALDatasetH hTileDS = terrain.heightsToRaster(minx, miny, maxx, maxy);
  GDALDatasetH hDstDS;
  GDALDriverH hDriver = GDALGetDriverByName("GTiff");

  hDstDS = GDALCreateCopy( hDriver, filename, hTileDS, FALSE,
                           NULL, NULL, NULL );
  if( hDstDS != NULL )
    GDALClose( hDstDS );
  GDALClose( hTileDS );
}

int main(int argc, char** argv) {
  short int zoom = atoi(argv[2]);
  int tx = atoi(argv[3]);
  int ty = atoi(argv[4]);
  char * tiffOut = argv[5];
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
  }

  cout << "Creating " << tiffOut << " using zoom " << zoom << " from tile " << tx << "," << ty << endl;
  
  GlobalGeodetic profile;
  double minLon, minLat, maxLon, maxLat;
  profile.tileBounds(tx, ty, zoom, minLon, minLat, maxLon, maxLat);

  terrain2tiff(terrain, tiffOut, minLon, minLat, maxLon, maxLat);

  return 0;
}
