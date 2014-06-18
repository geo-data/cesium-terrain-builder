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
 * @file terrain-build.cpp
 * @brief Convert a GDAL raster to terrain tiles
 *
 * This tool takes a GDAL raster and converts it to gzip compressed terrain
 * tiles which are written to an output directory on the filesystem.
 *
 * In the case of a multiband raster, only the first band is used to create the
 * terrain heights.  No water mask is currently set and all tiles are flagged
 * as being 'all land'.
 *
 * It is recommended that the input raster is in the EPSG 4326 spatial
 * reference system. If this is not the case then the tiles will be reprojected
 * to EPSG 4326 as required by the terrain tile format.
 */

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

/// Handle the terrain build CLI options
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

/// Output terrain tiles represented by a tiler to a directory
void
build(const GDALTiler &tiler, const char *outputDir) {
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

int
main(int argc, char *argv[]) {
  TerrainBuild command = TerrainBuild(argv[0], version.cstr);
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
