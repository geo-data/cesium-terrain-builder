#include <iostream>

#include "gdal_priv.h"
#include "commander.hpp"

#include "config.hpp"
#include "TerrainException.hpp"
#include "TerrainTile.hpp"

using namespace std;
using namespace terrain;

class TerrainInfo : public Command {
public:
  TerrainInfo(const char *name, const char *version) :
    Command(name, version),
    mShowHeights(false),
    mShowChildren(true),
    mShowType(true)
  {}

  void
  check() const {
    switch(command->argc) {
    case 1:
      return;
    case 0:
      cerr << "  Error: The terrain file must be specified" << endl;
      break;
    default:
      cerr << "  Error: Only one command line argument must be specified" << endl;
      break;
    }

    help();                   // print help and exit
  }

  static void
  showHeights(command_t *command) {
    static_cast<TerrainInfo *>(Command::self(command))->mShowHeights = true;
  }

  static void
  hideChildInfo(command_t *command) {
    static_cast<TerrainInfo *>(Command::self(command))->mShowChildren = false;
  }

  static void
  hideType(command_t *command) {
    static_cast<TerrainInfo *>(Command::self(command))->mShowType = false;
  }

  const char *
  getInputFilename() const {
    return  (command->argc == 1) ? command->argv[0] : NULL;
  }

  bool mShowHeights;
  bool mShowChildren;
  bool mShowType;
};

int main(int argc, char *argv[]) {
  Terrain terrain;
  TerrainInfo command = TerrainInfo(argv[0], version.c_str());
  command.setUsage("[options] TERRAIN_FILE");
  command.option("-e", "--show-heights", "show the height information as an ASCII raster", TerrainInfo::showHeights);
  command.option("-c", "--no-child", "hide information about child tiles", TerrainInfo::hideChildInfo);
  command.option("-t", "--no-type", "hide information about the tile type (i.e. water/land)", TerrainInfo::hideType);

  // Parse and check the arguments
  command.parse(argc, argv);
  command.check();

  GDALAllRegister();

  try {
    terrain = Terrain(command.getInputFilename());
  } catch (TerrainException &e) {
    cerr << "Error: " << e.what() << endl;
    return 1;
  }

  if (command.mShowHeights) {
    const std::vector<terrain::i_terrain_height> & heights = terrain.getHeights();
    cout << "Heights:";
    for (std::vector<terrain::i_terrain_height>::const_iterator iter = heights.begin(); iter != heights.end(); ++iter) {
      if ((iter - heights.begin()) % TILE_SIZE == 0) cout << endl;
      cout << *iter << " ";
    }
    cout << endl;
  }

  if (command.mShowChildren) {
    if (terrain.hasChildren()) {
      if (terrain.hasChildSW()) {
        cout << "Has a SW child" << endl;
      }
      if (terrain.hasChildSE()) {
        cout << "Has a SE child" << endl;
      }
      if (terrain.hasChildNW()) {
        cout << "Has a NW child" << endl;
      }
      if (terrain.hasChildNE()) {
        cout << "Has a NE child" << endl;
      }
    } else {
      cout << "Doesn't have children" << endl;
    }
  }

  if (command.mShowType) {
    if (terrain.hasWaterMask()) {
      cout << "The tile has a water mask" << endl;
    } else if (terrain.isLand()) {
      cout << "The tile is land" << endl;
    } else if (terrain.isWater()) {
      cout << "The tile is water" << endl;
    } else {
      // should not get here!!
      cerr << "Unknown tile type!!" << endl;
    }
  }

  return 0;
}
