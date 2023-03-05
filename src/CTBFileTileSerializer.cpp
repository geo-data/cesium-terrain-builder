/*******************************************************************************
 * Copyright 2018 GeoData <geodata@soton.ac.uk>
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
 * @file CTBFileTileSerializer.cpp
 * @brief This defines the `CTBFileTileSerializer` class
 */

#include <stdio.h>
#include <string.h>
#include <mutex>

#include "../deps/concat.hpp"
#include "cpl_vsi.h"
#include "CTBException.hpp"
#include "CTBFileTileSerializer.hpp"

#include "CTBFileOutputStream.hpp"
#include "CTBZOutputStream.hpp"

using namespace std;
using namespace ctb;

#ifdef _WIN32
static const char *osDirSep = "\\";
#else
static const char *osDirSep = "/";
#endif


/// Create a filename for a tile coordinate
std::string
ctb::CTBFileTileSerializer::getTileFilename(const TileCoordinate *coord, const string dirname, const char *extension) {
  static mutex mutex;
  VSIStatBufL stat;
  string filename = concat(dirname, coord->zoom, osDirSep, coord->x);

  lock_guard<std::mutex> lock(mutex);

  // Check whether the `{zoom}/{x}` directory exists or not
  if (VSIStatExL(filename.c_str(), &stat, VSI_STAT_EXISTS_FLAG | VSI_STAT_NATURE_FLAG)) {
    filename = concat(dirname, coord->zoom);

    // Check whether the `{zoom}` directory exists or not
    if (VSIStatExL(filename.c_str(), &stat, VSI_STAT_EXISTS_FLAG | VSI_STAT_NATURE_FLAG)) {
      // Create the `{zoom}` directory
      if (VSIMkdir(filename.c_str(), 0755))
        throw CTBException("Could not create the zoom level directory");

    } else if (!VSI_ISDIR(stat.st_mode)) {
      throw CTBException("Zoom level file path is not a directory");
    }

    // Create the `{zoom}/{x}` directory
    filename += concat(osDirSep, coord->x);
    if (VSIMkdir(filename.c_str(), 0755))
      throw CTBException("Could not create the x level directory");

  } else if (!VSI_ISDIR(stat.st_mode)) {
    throw CTBException("X level file path is not a directory");
  }

  // Create the filename itself, adding the extension if required
  filename += concat(osDirSep, coord->y);
  if (extension != NULL) {
    filename += ".";
    filename += extension;
  }

  return filename;
}

/// Check if file exists
static bool
fileExists(const std::string& filename) {
  VSIStatBufL statbuf;
  return VSIStatExL(filename.c_str(), &statbuf, VSI_STAT_EXISTS_FLAG) == 0;
}


/**
 * @details 
 * Returns if the specified Tile Coordinate should be serialized
 */
bool ctb::CTBFileTileSerializer::mustSerializeCoordinate(const ctb::TileCoordinate *coordinate) {
  if (!mresume)
    return true;

  const string filename = getTileFilename(coordinate, moutputDir, "terrain");
  return !fileExists(filename);
}

/**
 * @details 
 * Serialize a GDALTile to the Directory store
 */
bool 
ctb::CTBFileTileSerializer::serializeTile(const ctb::GDALTile *tile, GDALDriver *driver, const char *extension, CPLStringList &creationOptions) {
  const TileCoordinate *coordinate = tile;
  const string filename = getTileFilename(coordinate, moutputDir, extension);
  const string temp_filename = concat(filename, ".tmp");

  GDALDataset *poDstDS;
  poDstDS = driver->CreateCopy(temp_filename.c_str(), tile->dataset, FALSE, creationOptions, NULL, NULL);

  // Close the datasets, flushing data to destination
  if (poDstDS == NULL) {
    throw CTBException("Could not create GDAL tile");
  }
  GDALClose(poDstDS);

  if (VSIRename(temp_filename.c_str(), filename.c_str()) != 0) {
    throw CTBException("Could not rename temporary file");
  }
  return true;
}

/**
 * @details 
 * Serialize a TerrainTile to the Directory store
 */
bool
ctb::CTBFileTileSerializer::serializeTile(const ctb::TerrainTile *tile) {
  const TileCoordinate *coordinate = tile;
  const string filename = getTileFilename(tile, moutputDir, "terrain");
  const string temp_filename = concat(filename, ".tmp");

  CTBZFileOutputStream ostream(temp_filename.c_str());
  tile->writeFile(ostream);
  ostream.close();

  if (VSIRename(temp_filename.c_str(), filename.c_str()) != 0) {
    throw CTBException("Could not rename temporary file");
  }
  return true;
}

/**
 * @details 
 * Serialize a MeshTile to the Directory store
 */
bool
ctb::CTBFileTileSerializer::serializeTile(const ctb::MeshTile *tile, bool writeVertexNormals) {
  const TileCoordinate *coordinate = tile;
  const string filename = getTileFilename(coordinate, moutputDir, "terrain");
  const string temp_filename = concat(filename, ".tmp");

  CTBZFileOutputStream ostream(temp_filename.c_str());
  tile->writeFile(ostream, writeVertexNormals);
  ostream.close();

  if (VSIRename(temp_filename.c_str(), filename.c_str()) != 0) {
    throw CTBException("Could not rename temporary file");
  }
  return true;
}
