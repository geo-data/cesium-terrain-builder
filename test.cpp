#include <stdio.h>
#include "gdal_priv.h"
#include "src/GDALTiler.hpp"

using namespace std;

int main() {
  GDALAllRegister();

  GDALDataset  *poDataset = (GDALDataset *) GDALOpen("./lidar-2007-filled-cut.tif", GA_ReadOnly);
  GDALTiler tiler(poDataset);

  int tx = 8146, ty = 6409;
  short int zoom = 13;

  GDALDatasetH hTileDS = tiler.createRasterTile(zoom, tx, ty);
  GDALDatasetH hDstDS;
  GDALDriverH hDriver = GDALGetDriverByName("GTiff");

  hDstDS = GDALCreateCopy( hDriver, "13-8146-6409.gdal.tif", hTileDS, FALSE,
                           NULL, NULL, NULL );

  /* Once we're done, close properly the dataset */
  if( hDstDS != NULL )
    GDALClose( hDstDS );
  GDALClose( hTileDS );

  TerrainTile *terrainTile = tiler.createTerrainTile(zoom, tx, ty);
  FILE *terrainOut = fopen("13-8146-6409.terrain", "wb");
  terrainTile->writeFile(terrainOut);
  delete terrainTile;
  fclose(terrainOut);

  /*
  ofstream geojson;
  for (short int zoom = Profile.zoomForResolution(tiler.resolution); zoom >= 0; zoom--) {
    int minx, miny, maxx, maxy;
    Profile.latLonToTile(tiler.bounds[0], tiler.bounds[1], zoom, minx, miny);
    Profile.latLonToTile(tiler.bounds[2], tiler.bounds[3], zoom, maxx, maxy);
    //cout << "ll tile: " << minx << "," << miny << "; ur tile: " << maxx << "," << maxy << endl;

    string filename = static_cast<ostringstream*>( &(ostringstream() << zoom << ".geojson") )->str();
    geojson.open(filename.c_str());

    geojson << "{ \"type\": \"FeatureCollection\", \"features\": [" << endl;

    int tx, ty;
    double minLon, minLat, maxLon, maxLat;
    for (tx = minx; tx <= maxx; tx++) {
      for (ty = miny; ty <= maxy; ty++) {
        Profile.tileBounds(tx, ty, zoom, minLon, minLat, maxLon, maxLat);

        geojson << "{ \"type\": \"Feature\", \"geometry\": { \"type\": \"Polygon\", \"coordinates\": [[";
        printCoord(geojson, minLon, minLat);
        geojson << ", ";
        printCoord(geojson, maxLon, minLat);
        geojson << ", ";
        printCoord(geojson, maxLon, maxLat);
        geojson << ", ";
        printCoord(geojson, minLon, maxLat);
        geojson << ", ";
        printCoord(geojson, minLon, minLat);
        geojson << "]]}, \"properties\": {\"tx\": " << tx << ", \"ty\": " << ty << "}}";
        if (ty != maxy)
          geojson << "," << endl;
      }
      if (tx != maxx)
        geojson << "," << endl;

    }

    geojson << "]}" << endl;
    geojson.close();
  }
  */

  return 0;
}
