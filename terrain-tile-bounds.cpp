#include <iostream>
#include <fstream>
#include <sstream>

#include "gdal_priv.h"
#include "commander.hpp"

#include "src/GDALTiler.hpp"

using namespace std;

class TerrainTileBounds : public Command {
public:
  TerrainTileBounds(const char *name, const char *version) :
    Command(name, version),
    inputFilename(NULL)
  {}

  void check() const {
    switch(command->argc) {
    case 1:
      return;
    case 0:
      cerr << "  Error: The GDAL dataset must be specified" << endl;
      break;
    default:
      cerr << "  Error: Only one command line argument must be specified" << endl;
      break;
    }

    help();                   // print help and exit
  }

  const char * getInputFilename() const {
    return  (command->argc == 1) ? command->argv[0] : NULL;
  }

  const char *inputFilename;
};

static void printCoord(ofstream& stream, double x, double y) {
  stream << "[" << x << ", " << y << "]";
}

static void writeBounds(GDALTiler &tiler) {
  int tminx, tminy, tmaxx, tmaxy;
  ofstream geojson;
  short int maxZoom = tiler.maxZoomLevel();
  GlobalGeodetic profile = tiler.profile();

  for (short int zoom = maxZoom; zoom >= 0; zoom--) {
    tiler.lowerLeftTile(zoom, tminx, tminy);
    tiler.upperRightTile(zoom, tmaxx, tmaxy);

    string filename = static_cast<ostringstream*>( &(ostringstream() << zoom << ".geojson") )->str();
    cout << "creating " << filename << endl;
    geojson.open(filename.c_str());

    geojson << "{ \"type\": \"FeatureCollection\", \"features\": [" << endl;

    for (int tx = tminx; tx <= tmaxx; tx++) {
      for (int ty = tminy; ty <= tmaxy; ty++) {
        double minLon, minLat, maxLon, maxLat;

        profile.tileBounds(tx, ty, zoom, minLon, minLat, maxLon, maxLat);

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
        if (ty != tmaxy)
          geojson << "," << endl;
      }
      if (tx != tmaxx)
        geojson << "," << endl;

    }

    geojson << "]}" << endl;
    geojson.close();
  }
}

int main(int argc, char *argv[]) {
  TerrainTileBounds command = TerrainTileBounds(argv[0], "0.0.1");
  command.setUsage("GDAL_DATASET");

  // Parse and check the arguments
  command.parse(argc, argv);
  command.check();

  GDALAllRegister();

  GDALDataset  *poDataset = (GDALDataset *) GDALOpen(command.getInputFilename(), GA_ReadOnly);
  GDALTiler tiler(poDataset);

  writeBounds(tiler);

  return 0;
}
