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
 * @brief Convert a GDAL raster to a tile format
 *
 * This tool takes a GDAL raster and by default converts it to gzip compressed
 * terrain tiles which are written to an output directory on the filesystem.
 *
 * In the case of a multiband raster, only the first band is used to create the
 * terrain heights.  No water mask is currently set and all tiles are flagged
 * as being 'all land'.
 *
 * It is recommended that the input raster is in the EPSG 4326 spatial
 * reference system. If this is not the case then the tiles will be reprojected
 * to EPSG 4326 as required by the terrain tile format.
 *
 * Using the `--output-format` flag this tool can also be used to create tiles
 * in other raster formats that are supported by GDAL.
 */

#include <iostream>
#include <sstream>
#include <string.h>             // for strcmp
#include <stdlib.h>             // for atoi

#include "gdal_priv.h"
#include "commander.hpp"

#include "config.hpp"
#include "GlobalGeodetic.hpp"
#include "GlobalMercator.hpp"
#include "TerrainException.hpp"
#include "TerrainTiler.hpp"
#include "RasterIterator.hpp"
#include "TerrainIterator.hpp"

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
    outputDir("."),
    outputFormat("Terrain"),
    profile("geodetic"),
    tileSize(0)
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

  static void
  setOutputFormat(command_t *command) {
    static_cast<TerrainBuild *>(Command::self(command))->outputFormat = command->arg;
  }

  static void
  setProfile(command_t *command) {
    static_cast<TerrainBuild *>(Command::self(command))->profile = command->arg;
  }

  static void
  setTileSize(command_t *command) {
    static_cast<TerrainBuild *>(Command::self(command))->tileSize = atoi(command->arg);
  }

  const char *
  getInputFilename() const {
    return  (command->argc == 1) ? command->argv[0] : NULL;
  }

  const char *outputDir;
  const char *outputFormat;
  const char *profile;
  int tileSize;
};

string
getTileFilename(const TileCoordinate &coord, const string dirname, const char *extension) {
  string filename = dirname + static_cast<ostringstream*>
    (
     &(ostringstream()
       << coord.zoom
       << "-"
       << coord.x
       << "-"
       << coord.y)
     )->str();

  if (extension != NULL) {
    filename += ".";
    filename += extension;
  }

  return filename;
}

/// Output GDAL tiles represented by a tiler to a directory
void
buildGDAL(const GDALTiler &tiler, const char *outputDir, const char *outputFormat) {
  GDALDriver *poDriver = GetGDALDriverManager()->GetDriverByName(outputFormat);

  if (poDriver == NULL) {
    throw TerrainException("Could not retrieve GDAL driver");
  }

  if (poDriver->pfnCreateCopy == NULL) {
    throw TerrainException("The GDAL driver must be write enabled, specifically supporting 'CreateCopy'");
  }

  const char *extension = poDriver->GetMetadataItem(GDAL_DMD_EXTENSION);
  const string dirname = string(outputDir) + osDirSep;

  for (RasterIterator iter(tiler); !iter.exhausted(); ++iter) {
    std::pair<const TileCoordinate &, GDALDataset *> result = *iter;
    const TileCoordinate &coord = result.first;
    GDALDataset *poSrcDS = result.second;
    GDALDataset *poDstDS;
    const string filename = getTileFilename(coord, dirname, extension);

    cout << "creating " << filename << endl;

    poDstDS = poDriver->CreateCopy(filename.c_str(), poSrcDS, FALSE,
                                   NULL, NULL, NULL );

    // Close the datasets, flushing data to destination
    if (poDstDS != NULL)
      GDALClose(poDstDS);
    GDALClose(poSrcDS);
  }
}

/// Output terrain tiles represented by a tiler to a directory
void
buildTerrain(const TerrainTiler &tiler, const char *outputDir) {
  const string dirname = string(outputDir) + osDirSep;

  for (TerrainIterator iter(tiler); !iter.exhausted(); ++iter) {
    const TerrainTile terrainTile = *iter;
    const TileCoordinate &coord = terrainTile.getCoordinate();
    const string filename = getTileFilename(coord, dirname, "terrain");

    cout << "creating " << filename << endl;

    terrainTile.writeFile(filename.c_str());
  }
}

int
main(int argc, char *argv[]) {
  TerrainBuild command = TerrainBuild(argv[0], version.cstr);
  command.setUsage("[options] GDAL_DATASOURCE");
  command.option("-o", "--output-dir <dir>", "specify the output directory for the tiles (defaults to working directory)", TerrainBuild::setOutputDir);
  command.option("-f", "--output-format <format>", "specify the output format for the tiles. This is either `Terrain` (the default) or any format listed by `gdalinfo --formats`", TerrainBuild::setOutputFormat);
  command.option("-p", "--profile <profile>", "specify the TMS profile for the tiles. This is either `geodetic` (the default) or `mercator`", TerrainBuild::setProfile);
  command.option("-t", "--tile-size <size>", "specify the size of the tiles in pixels. This defaults to 65 for terrain tiles and 256 for other GDAL formats", TerrainBuild::setTileSize);

  // Parse and check the arguments
  command.parse(argc, argv);
  command.check();

  GDALAllRegister();

  Grid grid;
  if (strcmp(command.profile, "geodetic") == 0) {
    int tileSize = (command.tileSize < 1) ? 65 : command.tileSize;
    grid = GlobalGeodetic(tileSize);
  } else if (strcmp(command.profile, "mercator") == 0) {
    int tileSize = (command.tileSize < 1) ? 256 : command.tileSize;
    grid = GlobalMercator(tileSize);
  } else {
    cerr << "Error: Unknown profile: " << command.profile << endl;
    return 1;
  }

  GDALDataset  *poDataset = (GDALDataset *) GDALOpen(command.getInputFilename(), GA_ReadOnly);
  if (poDataset == NULL) {
    cerr << "Error: could not open GDAL dataset" << endl;
    return 1;
  }

  try {
    if (strcmp(command.outputFormat, "Terrain") == 0) {
      const TerrainTiler tiler(poDataset, grid);
      buildTerrain(tiler, command.outputDir);
    } else {                    // it's a GDAL format
      const GDALTiler tiler(poDataset, grid);
      buildGDAL(tiler, command.outputDir, command.outputFormat);
    }

  } catch (TerrainException &e) {
    cerr << "Error: " << e.what() << endl;
  }

  GDALClose(poDataset);

  return 0;
}
