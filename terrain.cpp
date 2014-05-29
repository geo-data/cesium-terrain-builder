#include <stdio.h>
#include <iostream>
#include <vector>
#include "src/TerrainTile.hpp"
#include "src/GlobalGeodetic.hpp"

#include "gdal_priv.h"
#include "ogr_spatialref.h"

void terrain2tiff(TerrainTile &terrain, double minx, double miny, double maxx, double maxy) {
  double resolution = (maxx - minx) / 65;
  double adfGeoTransform[6] = { minx, resolution, 0, maxy, 0, -resolution };

  OGRSpatialReference oSRS;
  oSRS.importFromEPSG(4326);

  GDALDriverH hDriver = GDALGetDriverByName( "GTiff" );
  GDALDatasetH hDstDS;
  GDALRasterBandH hBand;

  char *pszDstWKT = NULL;
  oSRS.exportToWkt( &pszDstWKT );

  hDstDS = GDALCreate(hDriver, "9-509-399.terrain.tif", 65, 65, 1, GDT_Int16, NULL );
  GDALSetProjection( hDstDS, pszDstWKT );
  CPLFree( pszDstWKT );
  GDALSetGeoTransform( hDstDS, adfGeoTransform );

  hBand = GDALGetRasterBand( hDstDS, 1 );
  GDALRasterIO( hBand, GF_Write, 0, 0, 65, 65,
                terrain.mHeights, 65, 65, GDT_Int16, 0, 0 );

  // Once we're done, close properly the dataset
  GDALClose( hDstDS );
}

int main() {
  TerrainTile terrain;
  FILE *terrainIn = fopen("9-509-399.terrain", "rb");

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

  if (terrain.isMixed()) {
    printf("The tile is a mixture of land and water\n");
  } else if (terrain.isLand()) {
    printf("The tile is land\n");
  } else if (terrain.isWater()) {
    printf("The tile is water\n");
  } else {
    // should not get here!!
    printf("Unknown tile type!!\n");
  }



  /*std::vector<short int> heights = terrain.heights();
  std::vector<short int>::iterator heightsIterator;
  for(heightsIterator = heights.begin();
      heightsIterator != heights.end();
      heightsIterator++)
    {
      std::cout << "height: " << *heightsIterator << std::endl;
    }

  std::vector<bool> mask = terrain.mask();
  std::vector<bool>::iterator maskIterator;
  for(maskIterator = mask.begin();
      maskIterator != mask.end();
      maskIterator++)
    {
      std::cout << "mask: " << *maskIterator << std::endl;
      }*/

  GlobalGeodetic profile;
  double minLon, minLat, maxLon, maxLat;
  short int zoom = 9;
  int tx = 509, ty = 399;
  profile.tileBounds(tx, ty, zoom, minLon, minLat, maxLon, maxLat);

  //terrain2tiff(terrain, -180, -90, 0, 90);
  terrain2tiff(terrain, minLon, minLat, maxLon, maxLat);

  FILE *terrainOut = fopen("out.terrain","wb");
  terrain.writeFile(terrainOut);
  fclose(terrainOut);

  return 0;
}
