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
 * @file ctb-export.cpp
 * @brief The terrain export tool
 *
 * This tool takes a terrain file with associated tile coordinate information
 * and converts it to a GeoTiff using the height information within the tile.
 */

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>

#include "gdal_priv.h"
#include "commander.hpp"

#include "config.hpp"
#include "CTBException.hpp"
#include "TerrainTile.hpp"
#include "GlobalGeodetic.hpp"

using namespace std;
using namespace ctb;

/// Handle the terrain export CLI options
class TerrainExport : public Command {
public:
  TerrainExport(const char *name, const char *version) :
    Command(name, version),
    inputFilename(NULL),
    outputFilename(NULL),
    zoom(0),
    tx(0),
    ty(0)
  {}

  static void
  setInputFilename(command_t *command) {
    TerrainExport *self = static_cast<TerrainExport *>(Command::self(command));
    self->inputFilename = command->arg;
    self->membersSet |= TT_INPUT;
  }

  static void
  setOutputFilename(command_t *command) {
    TerrainExport *self = static_cast<TerrainExport *>(Command::self(command));
    self->outputFilename = command->arg;
    self->membersSet |= TT_OUTPUT;
  }

  static void
  setZoomLevel(command_t *command) {
    TerrainExport *self = static_cast<TerrainExport *>(Command::self(command));
    self->zoom = atoi(command->arg);
    self->membersSet |= TT_ZOOM;
  }

  static void
  setTileX(command_t *command) {
    TerrainExport *self = static_cast<TerrainExport *>(Command::self(command));
    self->tx = atoi(command->arg);
    self->membersSet |= TT_TX;
  }

  static void
  setTileY(command_t *command) {
    TerrainExport *self = static_cast<TerrainExport *>(Command::self(command));
    self->ty = atoi(command->arg);
    self->membersSet |= TT_TY;
  }

  void
  check() const {
    bool failed = false;

    if ((membersSet & TT_INPUT) != TT_INPUT) {
      cerr << "  Error: The input filename must be specified" << endl;
      failed = true;
    }
    if ((membersSet & TT_OUTPUT) != TT_OUTPUT) {
      cerr << "  Error: The output filename must be specified" << endl;
      failed = true;
    }
    if ((membersSet & TT_ZOOM) != TT_ZOOM) {
      cerr << "  Error: The zoom level be specified" << endl;
      failed = true;
    }
    if ((membersSet & TT_TX) != TT_TX) {
      cerr << "  Error: The X tile coordinate must be specified" << endl;
      failed = true;
    }
    if ((membersSet & TT_TY) != TT_TY) {
      cerr << "  Error: The Y tile coordinate must be specified" << endl;
      failed = true;
    }

    if (failed) {
      help();                   // print help and exit
    }
  }

  const char *inputFilename;
  const char *outputFilename;
  i_zoom zoom;
  i_tile tx, ty;

private:
  char membersSet;
  enum members {
    TT_INPUT = 1,               // 2^0, bit 0
    TT_OUTPUT = 2,              // 2^1, bit 1
    TT_ZOOM = 4,                // 2^2, bit 2
    TT_TX = 8,                  // 2^3, bit 3
    TT_TY = 16                  // 2^4, bit 4
  };
};

/// Convert the terrain to the geotiff
void
terrain2tiff(TerrainTile &terrain, const char *filename) {
  GDALDatasetH hTileDS = terrain.heightsToRaster();
  GDALDatasetH hDstDS;
  GDALDriverH hDriver = GDALGetDriverByName("GTiff");

  hDstDS = GDALCreateCopy( hDriver, filename, hTileDS, FALSE,
                           NULL, NULL, NULL );
  if( hDstDS != NULL )
    GDALClose( hDstDS );
  GDALClose( hTileDS );
}

int
main(int argc, char *argv[]) {
  // Setup the command interface
  TerrainExport command = TerrainExport(argv[0], version.cstr);

  command.setUsage("-i TERRAIN_FILE -z ZOOM_LEVEL -x TILE_X -y TILE_Y -o OUTPUT_FILE ");
  command.option("-i", "--input-filename <filename>", "the terrain tile file to convert", TerrainExport::setInputFilename);
  command.option("-z", "--zoom-level <int>", "the zoom level represented by the tile", TerrainExport::setZoomLevel);
  command.option("-x", "--tile-x <int>", "the tile x coordinate", TerrainExport::setTileX);
  command.option("-y", "--tile-y <int>", "the tile y coordinate", TerrainExport::setTileY);
  command.option("-o", "--output-filename <filename>", "the output file to create", TerrainExport::setOutputFilename);

  // Parse and check the arguments
  command.parse(argc, argv);
  command.check();

  GDALAllRegister();

  // Instantiate an appropriate terrain tile
  const TileCoordinate coord(command.zoom, command.tx, command.ty);
  TerrainTile terrain(coord);

  // Read the data into the tile from the filesystem
  try {
    terrain.readFile(command.inputFilename);
  } catch (CTBException &e) {
    cerr << "Error: " << e.what() << endl;
  }

  cout << "Creating " << command.outputFilename << " using zoom " << command.zoom << " from tile " << command.tx << "," << command.ty << endl;

  // Write the data to tiff
  terrain2tiff(terrain, command.outputFilename);

  return 0;
}
