#include <iostream>
#include <sstream>

#include "gdal_priv.h"
#include "commander.hpp"

#include "config.hpp"
#include "TerrainException.hpp"
#include "GDALTiler.hpp"
#include "TileIterator.hpp"

using namespace std;
using namespace terrain;

#ifdef _WIN32
static const char *osDirSep = "\\";
#else
static const char *osDirSep = "/";
#endif

class TerrainBuild : public Command {
public:
  TerrainBuild(const char *name, const char *version) :
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
    static_cast<TerrainBuild *>(Command::self(command))->outputDir = command->arg;
  }

  const char *
  getInputFilename() const {
    return  (command->argc == 1) ? command->argv[0] : NULL;
  }

  const char *outputDir;
};

void build(const GDALTiler &tiler, const char *outputDir) {
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

    terrainTile.writeFile(filename.c_str());
  }
}

int main(int argc, char *argv[]) {
  TerrainBuild command = TerrainBuild(argv[0], version.c_str());
  command.setUsage("[options] GDAL_DATASOURCE");
  command.option("-o", "--output-dir <dir>", "specify the output directory for the tiles (defaults to working directory)", TerrainBuild::setOutputDir);

  // Parse and check the arguments
  command.parse(argc, argv);
  command.check();

  GDALAllRegister();

  GDALDataset  *poDataset = (GDALDataset *) GDALOpen(command.getInputFilename(), GA_ReadOnly);
  if (poDataset == NULL) {
    cerr << "Error: could not open GDAL dataset" << endl;
    return 1;
  }

  try {
    const GDALTiler tiler(poDataset);

    build(tiler, command.outputDir);

  } catch (TerrainException &e) {
    cerr << "Error: " << e.what() << endl;
  }

  GDALClose(poDataset);

  return 0;
}
