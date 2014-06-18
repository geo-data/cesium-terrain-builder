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
    static_cast<TerrainExtents *>(Command::self(command))->outputDir = command->arg;
  }

  const char *
  getInputFilename() const {
    return  (command->argc == 1) ? command->argv[0] : NULL;
  }

  const char *outputDir;
};

/// Write a GeoJSON coordinate to an output stream
static void
printCoord(ofstream& stream, const LatLon &coord) {
  stream << "[" << coord.x << ", " << coord.y << "]";
}

/// Write the tile extents to a directory in GeoJSON format
static void
writeBounds(GDALTiler &tiler, const char *outputDir) {
  ofstream geojson;
  i_zoom maxZoom = tiler.maxZoomLevel();
  GlobalGeodetic profile = tiler.profile();
  const string dirname = string(outputDir) + osDirSep;

  for (short int zoom = maxZoom; zoom >= 0; zoom--) {
    TileBounds tileBounds = tiler.tileBoundsForZoom(zoom);
    TileCoordinate currentTile(zoom, tileBounds.getLowerLeft());

    const string filename = dirname + static_cast<ostringstream*>( &(ostringstream() << zoom << ".geojson") )->str();
    cout << "creating " << filename << endl;

    geojson.open(filename.c_str());
    geojson << "{ \"type\": \"FeatureCollection\", \"features\": [" << endl;

    for (/* currentTile.x = tminx */; currentTile.x <= tileBounds.getMaxX(); currentTile.x++) {
      for (currentTile.y = tileBounds.getMinY(); currentTile.y <= tileBounds.getMaxY(); currentTile.y++) {
        LatLonBounds latLonBounds = profile.tileBounds(currentTile);

        geojson << "{ \"type\": \"Feature\", \"geometry\": { \"type\": \"Polygon\", \"coordinates\": [[";
        printCoord(geojson, latLonBounds.getLowerLeft());
        geojson << ", ";
        printCoord(geojson, latLonBounds.getLowerRight());
        geojson << ", ";
        printCoord(geojson, latLonBounds.getUpperRight());
        geojson << ", ";
        printCoord(geojson, latLonBounds.getUpperLeft());
        geojson << ", ";
        printCoord(geojson, latLonBounds.getLowerLeft());
        geojson << "]]}, \"properties\": {\"tx\": " << currentTile.x << ", \"ty\": " << currentTile.y << "}}";
        if (currentTile.y != tileBounds.getMaxY())
          geojson << "," << endl;
      }
      if (currentTile.x != tileBounds.getMaxX())
        geojson << "," << endl;

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

  // Parse and check the arguments
  command.parse(argc, argv);
  command.check();

  GDALAllRegister();

  GDALDataset  *poDataset = (GDALDataset *) GDALOpen(command.getInputFilename(), GA_ReadOnly);
  GDALTiler tiler(poDataset);

  writeBounds(tiler, command.outputDir);

  return 0;
}
