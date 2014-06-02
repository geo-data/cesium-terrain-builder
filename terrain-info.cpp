#include <stdio.h>
#include <iostream>
#include <vector>
#include "src/TerrainTile.hpp"
#include "src/GlobalGeodetic.hpp"

#include "gdal_priv.h"

void terrain2tiff(TerrainTile &terrain, double minx, double miny, double maxx, double maxy) {
  GDALDatasetH hTileDS = terrain.heightsToRaster(minx, miny, maxx, maxy);
  GDALDatasetH hDstDS;
  GDALDriverH hDriver = GDALGetDriverByName("GTiff");

  hDstDS = GDALCreateCopy( hDriver, "9-509-399.terrain.tif", hTileDS, FALSE,
                           NULL, NULL, NULL );
  if( hDstDS != NULL )
    GDALClose( hDstDS );
  GDALClose( hTileDS );
}

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

  /*GlobalGeodetic profile;
  double minLon, minLat, maxLon, maxLat;
  short int zoom = 9;
  int tx = 509, ty = 399;
  profile.tileBounds(tx, ty, zoom, minLon, minLat, maxLon, maxLat);

  terrain2tiff(terrain, minLon, minLat, maxLon, maxLat);*/

  /*FILE *terrainOut = fopen("out.terrain","wb");
  terrain.writeFile(terrainOut);
  fclose(terrainOut);*/

  return 0;
}
