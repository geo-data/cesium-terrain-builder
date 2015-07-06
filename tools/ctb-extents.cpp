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
 * @file ctb-extents.cpp
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
#include "concat.hpp"

#include "GlobalMercator.hpp"
#include "RasterTiler.hpp"
#include "GridIterator.hpp"

using namespace std;
using namespace ctb;

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
    tileSize(0),
    startZoom(-1),
    endZoom(-1)
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

    help();                     // print help and exit
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

  static void
  setStartZoom(command_t *command) {
    static_cast<TerrainExtents *>(Command::self(command))->startZoom = atoi(command->arg);
  }

  static void
  setEndZoom(command_t *command) {
    static_cast<TerrainExtents *>(Command::self(command))->endZoom = atoi(command->arg);
  }

  const char *
  getInputFilename() const {
    return  (command->argc == 1) ? command->argv[0] : NULL;
  }

  static bool
  cmpZoomLevels (i_zoom i, i_zoom j) {
    return (i > j);
  }

  const char *outputDir;
  const char *profile;
  int tileSize;
  int startZoom;
  int endZoom;
};

/// Write a GeoJSON coordinate to an output stream
static void
printCoord(ofstream& stream, const CRSPoint &coord) {
  stream << "[" << coord.x << ", " << coord.y << "]";
}

/// Write a GeoJSON tile to an output stream
static void
printTile(ofstream& stream, const GridIterator &iter) {
  if (iter.exhausted())
    return;

  const TileCoordinate *coord = *iter;
  const CRSBounds bounds = iter.getGrid().tileBounds(*coord);

  stream << "{ \"type\": \"Feature\", \"geometry\": { \"type\": \"Polygon\", \"coordinates\": [[";
  printCoord(stream, bounds.getLowerLeft());
  stream << ", ";
  printCoord(stream, bounds.getLowerRight());
  stream << ", ";
  printCoord(stream, bounds.getUpperRight());
  stream << ", ";
  printCoord(stream, bounds.getUpperLeft());
  stream << ", ";
  printCoord(stream, bounds.getLowerLeft());
  stream << "]]}, \"properties\": {\"tx\": " << coord->x << ", \"ty\": " << coord->y << "}}";
}

/// Output the tile extent for a particular zoom level
static bool
writeBoundsForZoom(ofstream &geojson, const string &dirname, GridIterator &iter, i_zoom zoom) {
  const string filename = concat(dirname, zoom, ".geojson");
  cout << "creating " << filename << endl;

  geojson.open(filename.c_str(), ofstream::trunc);
  if (!geojson) {
    cerr << "File could not be opened: " << strerror(errno) << endl; // Get some info as to why
    return false;
  }

  geojson << "{ \"type\": \"FeatureCollection\", \"features\": [" << endl;

  // Iterate over the tiles in the zoom level
  iter.reset(zoom, zoom);
  printTile(geojson, iter);

  for (++iter; !iter.exhausted(); ++iter) {
    geojson << "," << endl;
    printTile(geojson, iter);
  }

  geojson << "]}" << endl;
  geojson.close();

  return true;
}

/// Write the tile extents to a directory in GeoJSON format
static void
writeBounds(GDALTiler &tiler, const char *outputDir, int startZoom, int endZoom) {
  ofstream geojson;
  const Grid &grid = tiler.grid();
  const string dirname = string(outputDir) + osDirSep;

  if (startZoom < 0)
    startZoom = tiler.maxZoomLevel();
  if (endZoom < 0)
    endZoom = 0;

  GridIterator iter(grid, tiler.bounds(), startZoom, endZoom);

  // Set the precision and numeric notation on the stream
  geojson.precision(15);
  geojson.setf(ios::scientific, ios::floatfield);

  // Iterate over all zoom selected levels
  for (; startZoom >= endZoom; --startZoom) {
    if (!writeBoundsForZoom(geojson, dirname, iter, startZoom))
      return;
  }
}

int
main(int argc, char *argv[]) {
  TerrainExtents command = TerrainExtents(argv[0], version.cstr);
  command.setUsage("GDAL_DATASET");
  command.option("-o", "--output-dir <dir>", "specify the output directory for the geojson files (defaults to working directory)", TerrainExtents::setOutputDir);
  command.option("-p", "--profile <profile>", "specify the TMS profile for the tiles. This is either `geodetic` (the default) or `mercator`", TerrainExtents::setProfile);
  command.option("-t", "--tile-size <size>", "specify the size of the tiles in pixels. This defaults to 65 for terrain tiles and 256 for other GDAL formats", TerrainExtents::setTileSize);
  command.option("-s", "--start-zoom <zoom>", "specify the zoom level to start at. This should be greater than the end zoom level", TerrainExtents::setStartZoom);
  command.option("-e", "--end-zoom <zoom>", "specify the zoom level to end at. This should be less than the start zoom level and >= 0", TerrainExtents::setEndZoom);

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
  RasterTiler tiler(poDataset, grid);

  writeBounds(tiler, command.outputDir, command.startZoom, command.endZoom);

  return 0;
}
