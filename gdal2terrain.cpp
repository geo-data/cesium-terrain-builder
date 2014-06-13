#include <iostream>
#include <sstream>

#include "gdal_priv.h"
#include "commander.hpp"

#include "src/TerrainException.hpp"
#include "src/GDALTiler.hpp"
#include "src/TileIterator.hpp"

using namespace std;
using namespace terrain;

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

void gdal2terrain(const GDALTiler &tiler, const char *outputDir) {
  const string dirname = string(outputDir) + osDirSep;
  
  for(TileIterator iter(tiler); !iter.exhausted(); ++iter) {
    const TerrainTile terrainTile = *iter;
    const TileCoordinate coord = terrainTile.getCoordinate();
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
      terrainTile.writeFile(filename.c_str());
    } catch (TerrainException &e) {
      cerr << "Error: " << e.what() << endl;
    }
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
  if (poDataset == NULL) {
    cerr << "Error: could not open GDAL dataset" << endl;
    return 1;
  }
  const GDALTiler tiler(poDataset);

  gdal2terrain(tiler, command.outputDir);

  GDALClose(poDataset);

  return 0;
}
