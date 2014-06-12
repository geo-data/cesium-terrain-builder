#include <iostream>
#include <sstream>

#include "gdal_priv.h"
#include "commander.hpp"

#include "src/GDALTiler.hpp"
#include "src/TileIterator.hpp"

using namespace std;

#ifdef _WIN32
static const char *osDirSep = "\\";
#else
static const char *osDirSep = "/";
#endif

class GDAL2Terrain : public Command {
public:
  GDAL2Terrain(const char *name, const char *version) :
    Command(name, version),
    outputDir(".")
  {}

  void
  check() const {
    switch(command->argc) {
    case 1:
      return;
    case 0:
      cerr << "  Error: The gdal datasource must be specified" << endl;
      break;
    default:
      cerr << "  Error: Only one command line argument must be specified" << endl;
      break;
    }

    help();                   // print help and exit
  }

  static void
  setOutputDir(command_t *command) {
    static_cast<GDAL2Terrain *>(Command::self(command))->outputDir = command->arg;
  }

  const char *
  getInputFilename() const {
    return  (command->argc == 1) ? command->argv[0] : NULL;
  }

  const char *outputDir;
};

void writeTiles(const GDALTiler &tiler, const char *outputDir) {
  /*int tminx, tminy, tmaxx, tmaxy;
  short int maxZoom = tiler.maxZoomLevel();
  const string dirname = string(outputDir) + osDirSep;

  for (short int zoom = maxZoom; zoom >= 0; zoom--) {
    tiler.lowerLeftTile(zoom, tminx, tminy);
    tiler.upperRightTile(zoom, tmaxx, tmaxy);

    for (int tx = tminx; tx <= tmaxx; tx++) {
      for (int ty = tminy; ty <= tmaxy; ty++) {
        TerrainTile *terrainTile = tiler.createTerrainTile(zoom, tx, ty);
        const string filename = dirname + static_cast<ostringstream*>
          (
           &(ostringstream()
              << zoom
              << "-"
              << tx
              << "-"
              << ty
              << ".terrain")
           )->str();

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
    }*/

  const string dirname = string(outputDir) + osDirSep;
  
  for(TileIterator iter(tiler); !iter.exhausted(); ++iter) {
    const TerrainTile *terrainTile = *iter;
    const TileCoordinate coord = terrainTile->getCoordinate();
    const string filename = dirname + static_cast<ostringstream*>
      (
       &(ostringstream()
         << coord.zoom
         << "-"
         << coord.x
         << "-"
         << coord.y
         << ".terrain")
       )->str();

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

int main(int argc, char *argv[]) {
  GDAL2Terrain command = GDAL2Terrain(argv[0], "0.0.1");
  command.setUsage("[options] GDAL_DATASOURCE");
  command.option("-o", "--output-dir <dir>", "specify the output directory for the tiles (defaults to working directory)", GDAL2Terrain::setOutputDir);

  // Parse and check the arguments
  command.parse(argc, argv);
  command.check();

  GDALAllRegister();

  GDALDataset  *poDataset = (GDALDataset *) GDALOpen(command.getInputFilename(), GA_ReadOnly);
  const GDALTiler tiler(poDataset);

  writeTiles(tiler, command.outputDir);

  GDALClose(poDataset);

  return 0;
}
