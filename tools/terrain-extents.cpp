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
 * @file terrain-extents.cpp
 * @brief A tool to write tile extents covered by a GDAL raster to GeoJSON
 *
 * This tool takes a GDAL raster as input, calculates the appropriate maximum
 * zoom suitable for the raster, and then generates all tiles from the maximum
 * zoom to zoom level `0` which intersect with the bounds of the raster.  The
 * tiles are written to a directory in GeoJSON format.
 */

#include <iostream>
#include <fstream>
#include <sstream>

#include "gdal_priv.h"
#include "commander.hpp"

#include "config.hpp"
#include "GlobalGeodetic.hpp"
#include "GlobalMercator.hpp"
#include "GridIterator.hpp"
#include "GDALTiler.hpp"

using namespace std;
using namespace terrain;

#ifdef _WIN32
static const char *osDirSep = "\\";
#else
static const char *osDirSep = "/";
#endif

/// Handle the terrain extents CLI options
class TerrainExtents : public Command {
public:
  TerrainExtents(const char *name, const char *version) :
    Command(name, version),
    outputDir("."),
    profile("geodetic"),
    tileSize(0)
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
    static_cast<TerrainExtents *>(Command::self(command))->outputDir = command->arg;
  }

  static void
  setProfile(command_t *command) {
    static_cast<TerrainExtents *>(Command::self(command))->profile = command->arg;
  }

  static void
  setTileSize(command_t *command) {
    static_cast<TerrainExtents *>(Command::self(command))->tileSize = atoi(command->arg);
  }

  const char *
  getInputFilename() const {
    return  (command->argc == 1) ? command->argv[0] : NULL;
  }

  const char *outputDir;
  const char *profile;
  int tileSize;
};

/// Write a GeoJSON coordinate to an output stream
static void
printCoord(ofstream& stream, const CRSPoint &coord) {
  stream << "[" << coord.x << ", " << coord.y << "]";
}

/// Write a GeoJSON tile to an output stream
static void
printTile(ofstream& stream, const GridIterator &iter, const Grid &grid) {
  if (iter.exhausted())
    return;

  const TileCoordinate currentTile = *iter;
  const CRSBounds crsBounds = grid.tileBounds(currentTile);

  stream << "{ \"type\": \"Feature\", \"geometry\": { \"type\": \"Polygon\", \"coordinates\": [[";
  printCoord(stream, crsBounds.getLowerLeft());
  stream << ", ";
  printCoord(stream, crsBounds.getLowerRight());
  stream << ", ";
  printCoord(stream, crsBounds.getUpperRight());
  stream << ", ";
  printCoord(stream, crsBounds.getUpperLeft());
  stream << ", ";
  printCoord(stream, crsBounds.getLowerLeft());
  stream << "]]}, \"properties\": {\"tx\": " << currentTile.x << ", \"ty\": " << currentTile.y << "}}";
}

/// Write the tile extents to a directory in GeoJSON format
static void
writeBounds(GDALTiler &tiler, const char *outputDir) {
  ofstream geojson;
  i_zoom maxZoom = tiler.maxZoomLevel();
  const Grid &grid = tiler.grid();
  GridIterator iter(grid, tiler.bounds(), maxZoom);
  const string dirname = string(outputDir) + osDirSep;

  // Set the precision and numeric notation on the stream
  geojson.precision(15);
  geojson.setf(std::ios::scientific, std::ios::floatfield);

  // Create a new geojson file for each zoom level
  for (short int zoom = maxZoom; zoom >= 0; zoom--) {
    const string filename = dirname + static_cast<ostringstream*>( &(ostringstream() << zoom << ".geojson") )->str();
    cout << "creating " << filename << endl;

    geojson.open(filename.c_str());
    geojson << "{ \"type\": \"FeatureCollection\", \"features\": [" << endl;

    // Iterate over the tiles in the zoom level
    iter.reset(zoom, zoom);
    printTile(geojson, iter, grid);

    for (++iter; !iter.exhausted(); ++iter) {
      geojson << "," << endl;
      printTile(geojson, iter, grid);
    }

    geojson << "]}" << endl;
    geojson.close();
  }
}

int
main(int argc, char *argv[]) {
  TerrainExtents command = TerrainExtents(argv[0], version.cstr);
  command.setUsage("GDAL_DATASET");
  command.option("-o", "--output-dir <dir>", "specify the output directory for the geojson files (defaults to working directory)", TerrainExtents::setOutputDir);
  command.option("-p", "--profile <profile>", "specify the TMS profile for the tiles. This is either `geodetic` (the default) or `mercator`", TerrainExtents::setProfile);
  command.option("-t", "--tile-size <size>", "specify the size of the tiles in pixels. This defaults to 65 for terrain tiles and 256 for other GDAL formats", TerrainExtents::setTileSize);

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
  GDALTiler tiler(poDataset, grid);

  writeBounds(tiler, command.outputDir);

  return 0;
}
