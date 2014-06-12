#include <iostream>
#include <fstream>
#include <sstream>

#include "gdal_priv.h"
#include "commander.hpp"

#include "src/GDALTiler.hpp"

using namespace std;

#ifdef _WIN32
static const char *osDirSep = "\\";
#else
static const char *osDirSep = "/";
#endif

class TerrainTileBounds : public Command {
public:
  TerrainTileBounds(const char *name, const char *version) :
    Command(name, version),
    outputDir(".")
  {}

  void
  check() const {
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

  static void
  setOutputDir(command_t *command) {
    static_cast<TerrainTileBounds *>(Command::self(command))->outputDir = command->arg;
  }

  const char *
  getInputFilename() const {
    return  (command->argc == 1) ? command->argv[0] : NULL;
  }

  const char *outputDir;
};

static void printCoord(ofstream& stream, const Coordinate &coord) {
  stream << "[" << coord.x << ", " << coord.y << "]";
}

static void writeBounds(GDALTiler &tiler, const char *outputDir) {
  unsigned int tminy, tmaxx, tmaxy;
  ofstream geojson;
  unsigned short int maxZoom = tiler.maxZoomLevel();
  GlobalGeodetic profile = tiler.profile();
  const string dirname = string(outputDir) + osDirSep;

  for (short int zoom = maxZoom; zoom >= 0; zoom--) {
    TileCoordinate lowerLeft = tiler.lowerLeftTile(zoom);
    TileCoordinate upperRight = tiler.upperRightTile(zoom);
    TileCoordinate currentTile = lowerLeft;
    tminy = lowerLeft.y;
    tmaxx = upperRight.x;
    tmaxy = upperRight.y;

    const string filename = dirname + static_cast<ostringstream*>( &(ostringstream() << zoom << ".geojson") )->str();
    cout << "creating " << filename << endl;

    geojson.open(filename.c_str());
    geojson << "{ \"type\": \"FeatureCollection\", \"features\": [" << endl;

    for (/* currentTile.x = tminx */; currentTile.x <= tmaxx; currentTile.x++) {
      for (currentTile.y = tminy; currentTile.y <= tmaxy; currentTile.y++) {
        Bounds tileBounds = profile.tileBounds(currentTile);

        geojson << "{ \"type\": \"Feature\", \"geometry\": { \"type\": \"Polygon\", \"coordinates\": [[";
        printCoord(geojson, tileBounds.getLowerLeft());
        geojson << ", ";
        printCoord(geojson, tileBounds.getLowerRight());
        geojson << ", ";
        printCoord(geojson, tileBounds.getUpperRight());
        geojson << ", ";
        printCoord(geojson, tileBounds.getUpperLeft());
        geojson << ", ";
        printCoord(geojson, tileBounds.getLowerLeft());
        geojson << "]]}, \"properties\": {\"tx\": " << currentTile.x << ", \"ty\": " << currentTile.y << "}}";
        if (currentTile.y != tmaxy)
          geojson << "," << endl;
      }
      if (currentTile.x != tmaxx)
        geojson << "," << endl;

    }

    geojson << "]}" << endl;
    geojson.close();
  }
}

int main(int argc, char *argv[]) {
  TerrainTileBounds command = TerrainTileBounds(argv[0], "0.0.1");
  command.setUsage("GDAL_DATASET");
  command.option("-o", "--output-dir <dir>", "specify the output directory for the geojson files (defaults to working directory)", TerrainTileBounds::setOutputDir);

  // Parse and check the arguments
  command.parse(argc, argv);
  command.check();

  GDALAllRegister();

  GDALDataset  *poDataset = (GDALDataset *) GDALOpen(command.getInputFilename(), GA_ReadOnly);
  GDALTiler tiler(poDataset);

  writeBounds(tiler, command.outputDir);

  return 0;
}
