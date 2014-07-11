/*******************************************************************************
 * Copyright 2014 GeoData <geodata@soton.ac.uk>
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *******************************************************************************/

/**
 * @file ctb-info.cpp
 * @brief A tool to extract information from the terrain tile
 *
 * This tool takes a terrain file and optionally extracts height, child tile
 * and water mask information. It exits with `0` on success or `1` otherwise.
 */

#include <iostream>

#include "gdal_priv.h"
#include "commander.hpp"

#include "config.hpp"
#include "CTBException.hpp"
#include "TerrainTile.hpp"

using namespace std;
using namespace ctb;

/// Handle the terrain info CLI options
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

int
main(int argc, char *argv[]) {
  // Set up the command interface
  TerrainInfo command = TerrainInfo(argv[0], version.cstr);
  command.setUsage("[options] TERRAIN_FILE");
  command.option("-e", "--show-heights", "show the height information as an ASCII raster", TerrainInfo::showHeights);
  command.option("-c", "--no-child", "hide information about child tiles", TerrainInfo::hideChildInfo);
  command.option("-t", "--no-type", "hide information about the tile type (i.e. water/land)", TerrainInfo::hideType);

  // Parse and check the arguments
  command.parse(argc, argv);
  command.check();

  GDALAllRegister();

  // Read the terrain data from the filesystem
  Terrain terrain;
  try {
    terrain = Terrain(command.getInputFilename());
  } catch (CTBException &e) {
    cerr << "Error: " << e.what() << endl;
    return 1;
  }

  // Print out the heights if required
  if (command.mShowHeights) {
    const std::vector<ctb::i_terrain_height> & heights = terrain.getHeights();
    cout << "Heights:";
    for (std::vector<ctb::i_terrain_height>::const_iterator iter = heights.begin(); iter != heights.end(); ++iter) {
      if ((iter - heights.begin()) % TILE_SIZE == 0) cout << endl;
      cout << *iter << " ";
    }
    cout << endl;
  }

  // Print out the child tiles if required
  if (command.mShowChildren) {
    if (terrain.hasChildren()) {
      cout << "Child tiles:";
      if (terrain.hasChildSW()) {
        cout << " SW";
      }
      if (terrain.hasChildSE()) {
        cout << " SE";
      }
      if (terrain.hasChildNW()) {
        cout << " NW";
      }
      if (terrain.hasChildNE()) {
        cout << " NE";
      }
    } else {
      cout << " None";
    }
    cout << endl;
  }

  // Print out the tile type if required
  if (command.mShowType) {
    cout << "Tile type: ";
    if (terrain.hasWaterMask()) {
      cout << "water mask";
    } else if (terrain.isLand()) {
      cout << "all land";
    } else if (terrain.isWater()) {
      cout << "all water";
    } else {
      // should not get here!!
      cout << "unknown";
      cerr << "Unknown tile type!!" << endl;
    }
    cout << endl;
  }

  return 0;
}
