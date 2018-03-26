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
 * @file ctb-tile.cpp
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
#include <thread>
#include <mutex>
#include <future>

#include "cpl_multiproc.h"      // for CPLGetNumCPUs
#include "cpl_vsi.h"            // for virtual filesystem
#include "gdal_priv.h"
#include "commander.hpp"        // for cli parsing
#include "concat.hpp"

#include "GlobalMercator.hpp"
#include "RasterIterator.hpp"
#include "TerrainIterator.hpp"
#include "MeshIterator.hpp"

using namespace std;
using namespace ctb;

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
    threadCount(-1),
    tileSize(0),
    startZoom(-1),
    endZoom(-1),
    verbosity(1),
    resume(false),
    meshQualityFactor(1.0),
    metadata(false)
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
  setThreadCount(command_t *command) {
    static_cast<TerrainBuild *>(Command::self(command))->threadCount = atoi(command->arg);
  }

  static void
  setTileSize(command_t *command) {
    static_cast<TerrainBuild *>(Command::self(command))->tileSize = atoi(command->arg);
  }

  static void
  setStartZoom(command_t *command) {
    static_cast<TerrainBuild *>(Command::self(command))->startZoom = atoi(command->arg);
  }

  static void
  setEndZoom(command_t *command) {
    static_cast<TerrainBuild *>(Command::self(command))->endZoom = atoi(command->arg);
  }

  static void
  setQuiet(command_t *command) {
    --(static_cast<TerrainBuild *>(Command::self(command))->verbosity);
  }

  static void
  setVerbose(command_t *command) {
    ++(static_cast<TerrainBuild *>(Command::self(command))->verbosity);
  }

  static void
  setResume(command_t* command) {
    static_cast<TerrainBuild *>(Command::self(command))->resume = true;
  }

  static void
  setResampleAlg(command_t *command) {
    GDALResampleAlg eResampleAlg;

    if (strcmp(command->arg, "nearest") == 0)
      eResampleAlg = GRA_NearestNeighbour;
    else if (strcmp(command->arg, "bilinear") == 0)
      eResampleAlg = GRA_Bilinear;
    else if (strcmp(command->arg, "cubic") == 0)
      eResampleAlg = GRA_Cubic;
    else if (strcmp(command->arg, "cubicspline") == 0)
      eResampleAlg = GRA_CubicSpline;
    else if (strcmp(command->arg, "lanczos") == 0)
      eResampleAlg = GRA_Lanczos;
    else if (strcmp(command->arg, "average") == 0)
      eResampleAlg = GRA_Average;
    else if (strcmp(command->arg, "mode") == 0)
      eResampleAlg = GRA_Mode;
    else if (strcmp(command->arg, "max") == 0)
      eResampleAlg = GRA_Max;
    else if (strcmp(command->arg, "min") == 0)
      eResampleAlg = GRA_Min;
    else if (strcmp(command->arg, "med") == 0)
      eResampleAlg = GRA_Med;
    else if (strcmp(command->arg, "q1") == 0)
      eResampleAlg = GRA_Q1;
    else if (strcmp(command->arg, "q3") == 0)
      eResampleAlg = GRA_Q3;
    else {
      cerr << "Error: Unknown resampling algorithm: " << command->arg << endl;
      static_cast<TerrainBuild *>(Command::self(command))->help(); // exit
    }

    static_cast<TerrainBuild *>(Command::self(command))->tilerOptions.resampleAlg = eResampleAlg;
  }

  static void
  addCreationOption(command_t *command) {
    static_cast<TerrainBuild *>(Command::self(command))->creationOptions.AddString(command->arg);
  }

  static void
  setErrorThreshold(command_t *command) {
    static_cast<TerrainBuild *>(Command::self(command))->tilerOptions.errorThreshold = atof(command->arg);
  }

  static void
  setWarpMemory(command_t *command) {
    static_cast<TerrainBuild *>(Command::self(command))->tilerOptions.warpMemoryLimit = atof(command->arg);
  }

  const char *
  getInputFilename() const {
    return  (command->argc == 1) ? command->argv[0] : NULL;
  }

  static void
    setMeshQualityFactor(command_t *command) {
    static_cast<TerrainBuild *>(Command::self(command))->meshQualityFactor = atof(command->arg);
  }

  static void
    setMetadata(command_t *command) {
    static_cast<TerrainBuild *>(Command::self(command))->metadata = true;
  }

  const char *outputDir,
    *outputFormat,
    *profile;

  int threadCount,
    tileSize,
    startZoom,
    endZoom,
    verbosity;

  bool resume;

  CPLStringList creationOptions;
  TilerOptions tilerOptions;

  double meshQualityFactor;
  bool metadata;
};

/**
 * Create a filename for a tile coordinate
 *
 * This also creates the tile directory structure.
 */
static string
getTileFilename(const TileCoordinate *coord, const string dirname, const char *extension) {
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

/**
 * Increment a TilerIterator whilst cooperating between threads
 *
 * This function maintains an global index on an iterator and when called
 * ensures the iterator is incremented to point to the next global index.  This
 * can therefore be called with different tiler iterators by different threads
 * to ensure all tiles are iterated over consecutively.  It assumes individual
 * tile iterators point to the same source GDAL dataset.
 */
template<typename T> int
incrementIterator(T &iter, int currentIndex) {
  static int globalIteratorIndex = 0; // keep track of where we are globally
  static mutex mutex;        // ensure iterations occur serially between threads

  lock_guard<std::mutex> lock(mutex);

  while (currentIndex < globalIteratorIndex) {
    ++iter;
    ++currentIndex;
  }
  ++globalIteratorIndex;

  return currentIndex;
}

/// Get a handle on the total number of tiles to be created
static int iteratorSize = 0;    // the total number of tiles
template<typename T> void
setIteratorSize(T &iter) {
  static mutex mutex;

  lock_guard<std::mutex> lock(mutex);

  if (iteratorSize == 0) {
    iteratorSize = iter.getSize();
  }
}

/// A thread safe wrapper around `GDALTermProgress`
static int
CPL_STDCALL termProgress(double dfComplete, const char *pszMessage, void *pProgressArg) {
  static mutex mutex;          // GDALTermProgress isn't thread safe, so lock it
  int status;

  lock_guard<std::mutex> lock(mutex);
  status = GDALTermProgress(dfComplete, pszMessage, pProgressArg);

  return status;
}

/// In a thread safe manner describe the file just created
static int
CPL_STDCALL verboseProgress(double dfComplete, const char *pszMessage, void *pProgressArg) {
  stringstream stream;
  stream << "[" << (int) (dfComplete*100) << "%] " << pszMessage << endl;
  cout << stream.str();

  return TRUE;
}

// Default to outputting using the GDAL progress meter
static GDALProgressFunc progressFunc = termProgress;

/// Output the progress of the tiling operation
int
showProgress(int currentIndex, string filename) {
  stringstream stream;
  stream << "created " << filename << " in thread " << this_thread::get_id();
  string message = stream.str();

  return progressFunc(currentIndex / (double) iteratorSize, message.c_str(), NULL);
}

static bool
fileExists(const std::string& filename) {
  VSIStatBufL statbuf;
  return VSIStatExL(filename.c_str(), &statbuf, VSI_STAT_EXISTS_FLAG) == 0;
}

/// Handle the terrain metadata
class TerrainMetadata {
public:
  TerrainMetadata() {
  }

  // Defines the valid tile indexes of a level in a Tileset
  struct LevelInfo {
  public:
    LevelInfo() {
      startX = startY = std::numeric_limits<int>::max();
      finalX = finalY = std::numeric_limits<int>::min();
    }
    int startX, startY;
    int finalX, finalY;

    inline void add(const TileCoordinate *coordinate) {
      startX = std::min(startX, (int)coordinate->x);
      startY = std::min(startY, (int)coordinate->y);
      finalX = std::max(finalX, (int)coordinate->x);
      finalY = std::max(finalY, (int)coordinate->y);
    }
    inline void add(const LevelInfo &level) {
      startX = std::min(startX, level.startX);
      startY = std::min(startY, level.startY);
      finalX = std::max(finalX, level.finalX);
      finalY = std::max(finalY, level.finalY);
    }
  };
  std::vector<LevelInfo> levels;

  // Defines the bounding box covered by the Terrain
  CRSBounds bounds;

  // Add metadata of the specified Coordinate
  void add(const Grid &grid, const TileCoordinate *coordinate) {
    CRSBounds tileBounds = grid.tileBounds(*coordinate);
    i_zoom zoom = coordinate->zoom;

    if ((1 + zoom) > levels.size()) {
      for (size_t i = 0; i <= zoom; i++) {
        levels.push_back(LevelInfo());
      }
    }
    LevelInfo &level = levels[zoom];
    level.add(coordinate);

    if (bounds.getMaxX() == bounds.getMinX()) {
      bounds = tileBounds;
    }
    else {
      bounds.setMinX(std::min(bounds.getMinX(), tileBounds.getMinX()));
      bounds.setMinY(std::min(bounds.getMinY(), tileBounds.getMinY()));
      bounds.setMaxX(std::max(bounds.getMaxX(), tileBounds.getMaxX()));
      bounds.setMaxY(std::max(bounds.getMaxY(), tileBounds.getMaxY()));
    }
  }

  // Add metadata info
  void add(const TerrainMetadata &otherMetadata) {
    if (otherMetadata.levels.size() > 0) {
      const CRSBounds &otherBounds = otherMetadata.bounds;

      for (size_t i = 0; i < (otherMetadata.levels.size() - this->levels.size()); i++) {
        levels.push_back(LevelInfo());
      }
      for (size_t i = 0; i < levels.size(); i++) {
        levels[i].add(otherMetadata.levels[i]);
      }

      bounds.setMinX(std::min(bounds.getMinX(), otherBounds.getMinX()));
      bounds.setMinY(std::min(bounds.getMinY(), otherBounds.getMinY()));
      bounds.setMaxX(std::max(bounds.getMaxX(), otherBounds.getMaxX()));
      bounds.setMaxY(std::max(bounds.getMaxY(), otherBounds.getMaxY()));
    }
  }

  /// Output the layer.json metadata file
  /// http://help.agi.com/TerrainServer/RESTAPIGuide.html
  /// Example:
  /// https://assets.agi.com/stk-terrain/v1/tilesets/world/tiles/layer.json
  void writeJsonFile(const std::string &filename, const std::string &datasetName, const std::string &outputFormat) const {
    FILE *fp = fopen(filename.c_str(), "w");

    if (fp == NULL) {
      throw CTBException("Failed to open metadata file");
    }

    fprintf(fp, "{\n");
    fprintf(fp, "  \"tilejson\": \"2.1.0\",\n");
    fprintf(fp, "  \"name\": \"%s\",\n", datasetName.c_str());
    fprintf(fp, "  \"description\": \"\",\n");
    fprintf(fp, "  \"version\": \"1.1.0\",\n");

    if (strcmp(outputFormat.c_str(), "Terrain") == 0) {
      fprintf(fp, "  \"format\": \"heightmap-1.0\",\n");
    }
    else if (strcmp(outputFormat.c_str(), "Mesh") == 0) {
      fprintf(fp, "  \"format\": \"quantized-mesh-1.0\",\n");
    }
    else {
      fprintf(fp, "  \"format\": \"GDAL\",\n");
    }
    fprintf(fp, "  \"attribution\": \"\",\n");
    fprintf(fp, "  \"schema\": \"tms\",\n");
    fprintf(fp, "  \"tiles\": [ \"{z}/{x}/{y}.terrain?v={version}\" ],\n");
    fprintf(fp, "  \"projection\": \"EPSG:4326\",\n");
    fprintf(fp, "  \"bounds\": [ %.2f, %.2f, %.2f, %.2f ],\n",
      bounds.getMinX(),
      bounds.getMinY(),
      bounds.getMaxX(),
      bounds.getMaxY());

    fprintf(fp, "  \"available\": [\n");
    for (size_t i = 0, icount = levels.size(); i < icount; i++) {
      const LevelInfo &level = levels[i];

      if (i > 0)
        fprintf(fp, "   ,[ ");
      else
        fprintf(fp, "    [ ");

      if (level.finalX >= level.startX) {
        fprintf(fp, "{ \"startX\": %li, \"startY\": %li, \"endX\": %li, \"endY\": %li }",
          level.startX,
          level.startY,
          level.finalX,
          level.finalY);
      }
      fprintf(fp, " ]\n");
    }
    fprintf(fp, "  ]\n");

    fprintf(fp, "}\n");
    fclose(fp);
  }
};

/// Output GDAL tiles represented by a tiler to a directory
static void
buildGDAL(const RasterTiler &tiler, TerrainBuild *command, TerrainMetadata *metadata) {
  GDALDriver *poDriver = GetGDALDriverManager()->GetDriverByName(command->outputFormat);

  if (poDriver == NULL) {
    throw CTBException("Could not retrieve GDAL driver");
  }

  if (poDriver->pfnCreateCopy == NULL) {
    throw CTBException("The GDAL driver must be write enabled, specifically supporting 'CreateCopy'");
  }

  const char *extension = poDriver->GetMetadataItem(GDAL_DMD_EXTENSION);
  const string dirname = string(command->outputDir) + osDirSep;
  i_zoom startZoom = (command->startZoom < 0) ? tiler.maxZoomLevel() : command->startZoom,
    endZoom = (command->endZoom < 0) ? 0 : command->endZoom;

  RasterIterator iter(tiler, startZoom, endZoom);
  int currentIndex = incrementIterator(iter, 0);
  setIteratorSize(iter);

  while (!iter.exhausted()) {
    const TileCoordinate *coordinate = iter.GridIterator::operator*();
    GDALDataset *poDstDS;
    const string filename = getTileFilename(coordinate, dirname, extension);
    if (metadata) metadata->add(tiler.grid(), coordinate);

    if( !command->resume || !fileExists(filename) ) {
      GDALTile *tile = *iter;
      const string temp_filename = concat(filename, ".tmp");
      poDstDS = poDriver->CreateCopy(temp_filename.c_str(), tile->dataset, FALSE,
                                     command->creationOptions.List(), NULL, NULL );
      delete tile;

      // Close the datasets, flushing data to destination
      if (poDstDS == NULL) {
        throw CTBException("Could not create GDAL tile");
      }

      GDALClose(poDstDS);

      if (VSIRename(temp_filename.c_str(), filename.c_str()) != 0) {
        throw new CTBException("Could not rename temporary file");
      }
    }

    currentIndex = incrementIterator(iter, currentIndex);
    showProgress(currentIndex, filename);
  }
}

/// Output terrain tiles represented by a tiler to a directory
static void
buildTerrain(const TerrainTiler &tiler, TerrainBuild *command, TerrainMetadata *metadata) {
  const string dirname = string(command->outputDir) + osDirSep;
  i_zoom startZoom = (command->startZoom < 0) ? tiler.maxZoomLevel() : command->startZoom,
    endZoom = (command->endZoom < 0) ? 0 : command->endZoom;

  TerrainIterator iter(tiler, startZoom, endZoom);
  int currentIndex = incrementIterator(iter, 0);
  setIteratorSize(iter);

  while (!iter.exhausted()) {
    const TileCoordinate *coordinate = iter.GridIterator::operator*();
    const string filename = getTileFilename(coordinate, dirname, "terrain");
    if (metadata) metadata->add(tiler.grid(), coordinate);

    if( !command->resume || !fileExists(filename) ) {
      TerrainTile *tile = *iter;
      const string temp_filename = concat(filename, ".tmp");

      tile->writeFile(temp_filename.c_str());
      delete tile;

      if (VSIRename(temp_filename.c_str(), filename.c_str()) != 0) {
        throw new CTBException("Could not rename temporary file");
      }
    }

    currentIndex = incrementIterator(iter, currentIndex);
    showProgress(currentIndex, filename);
  }
}

/// Output mesh tiles represented by a tiler to a directory
static void
buildMesh(const MeshTiler &tiler, TerrainBuild *command, TerrainMetadata *metadata) {
  const string dirname = string(command->outputDir) + osDirSep;
  i_zoom startZoom = (command->startZoom < 0) ? tiler.maxZoomLevel() : command->startZoom,
    endZoom = (command->endZoom < 0) ? 0 : command->endZoom;


  MeshIterator iter(tiler, startZoom, endZoom);
  int currentIndex = incrementIterator(iter, 0);
  setIteratorSize(iter);

  while (!iter.exhausted()) {
    const TileCoordinate *coordinate = iter.GridIterator::operator*();
    const string filename = getTileFilename(coordinate, dirname, "terrain");
    if (metadata) metadata->add(tiler.grid(), coordinate);

    if( !command->resume || !fileExists(filename) ) {
      MeshTile *tile = *iter;
      const string temp_filename = concat(filename, ".tmp");

      tile->writeFile(temp_filename.c_str());
      delete tile;

      if (VSIRename(temp_filename.c_str(), filename.c_str()) != 0) {
        throw new CTBException("Could not rename temporary file");
      }
    }

    currentIndex = incrementIterator(iter, currentIndex);
    showProgress(currentIndex, filename);
  }
}

static void
buildMetadata(const RasterTiler &tiler, TerrainBuild *command, TerrainMetadata *metadata) {
  const string dirname = string(command->outputDir) + osDirSep;
  i_zoom startZoom = (command->startZoom < 0) ? tiler.maxZoomLevel() : command->startZoom,
    endZoom = (command->endZoom < 0) ? 0 : command->endZoom;

  const std::string filename = concat(dirname, "layer.json"); 

  RasterIterator iter(tiler, startZoom, endZoom);
  int currentIndex = incrementIterator(iter, 0);
  setIteratorSize(iter);

  while (!iter.exhausted()) {
    const TileCoordinate *coordinate = iter.GridIterator::operator*();
    if (metadata) metadata->add(tiler.grid(), coordinate);

    currentIndex = incrementIterator(iter, currentIndex);
    showProgress(currentIndex, filename);
  }
}

/**
 * Perform a tile building operation
 *
 * This function is designed to be run in a separate thread.
 */
static int
runTiler(TerrainBuild *command, Grid *grid, TerrainMetadata *metadata) {
  GDALDataset  *poDataset = (GDALDataset *) GDALOpen(command->getInputFilename(), GA_ReadOnly);
  if (poDataset == NULL) {
    cerr << "Error: could not open GDAL dataset" << endl;
    return 1;
  }

  // Metadata of only this thread, it will be joined to global later
  TerrainMetadata *threadMetadata = metadata ? new TerrainMetadata() : NULL;

  try {
    if (command->metadata) {
      const RasterTiler tiler(poDataset, *grid, command->tilerOptions);
      buildMetadata(tiler, command, threadMetadata);
    } else if (strcmp(command->outputFormat, "Terrain") == 0) {
      const TerrainTiler tiler(poDataset, *grid);
      buildTerrain(tiler, command, threadMetadata);
    } else if (strcmp(command->outputFormat, "Mesh") == 0) {
      const MeshTiler tiler(poDataset, *grid, command->tilerOptions, command->meshQualityFactor);
      buildMesh(tiler, command, threadMetadata);
    } else {                    // it's a GDAL format
      const RasterTiler tiler(poDataset, *grid, command->tilerOptions);
      buildGDAL(tiler, command, threadMetadata);
    }

  } catch (CTBException &e) {
    cerr << "Error: " << e.what() << endl;
  }

  GDALClose(poDataset);

  // Pass metadata to global instance.
  if (threadMetadata) {
    static std::mutex mutex;
    std::lock_guard<std::mutex> lock(mutex);

    metadata->add(*threadMetadata);
    delete threadMetadata;
  }
  return 0;
}

int
main(int argc, char *argv[]) {
  // Specify the command line interface
  TerrainBuild command = TerrainBuild(argv[0], version.cstr);
  command.setUsage("[options] GDAL_DATASOURCE");
  command.option("-o", "--output-dir <dir>", "specify the output directory for the tiles (defaults to working directory)", TerrainBuild::setOutputDir);
  command.option("-f", "--output-format <format>", "specify the output format for the tiles. This is either `Terrain` (the default), `Mesh` (Chunked LOD mesh), or any format listed by `gdalinfo --formats`", TerrainBuild::setOutputFormat);
  command.option("-p", "--profile <profile>", "specify the TMS profile for the tiles. This is either `geodetic` (the default) or `mercator`", TerrainBuild::setProfile);
  command.option("-c", "--thread-count <count>", "specify the number of threads to use for tile generation. On multicore machines this defaults to the number of CPUs", TerrainBuild::setThreadCount);
  command.option("-t", "--tile-size <size>", "specify the size of the tiles in pixels. This defaults to 65 for terrain tiles and 256 for other GDAL formats", TerrainBuild::setTileSize);
  command.option("-s", "--start-zoom <zoom>", "specify the zoom level to start at. This should be greater than the end zoom level", TerrainBuild::setStartZoom);
  command.option("-e", "--end-zoom <zoom>", "specify the zoom level to end at. This should be less than the start zoom level and >= 0", TerrainBuild::setEndZoom);
  command.option("-r", "--resampling-method <algorithm>", "specify the raster resampling algorithm.  One of: nearest; bilinear; cubic; cubicspline; lanczos; average; mode; max; min; med; q1; q3. Defaults to average.", TerrainBuild::setResampleAlg);
  command.option("-n", "--creation-option <option>", "specify a GDAL creation option for the output dataset in the form NAME=VALUE. Can be specified multiple times. Not valid for Terrain tiles.", TerrainBuild::addCreationOption);
  command.option("-z", "--error-threshold <threshold>", "specify the error threshold in pixel units for transformation approximation. Larger values should mean faster transforms. Defaults to 0.125", TerrainBuild::setErrorThreshold);
  command.option("-m", "--warp-memory <bytes>", "The memory limit in bytes used for warp operations. Higher settings should be faster. Defaults to a conservative GDAL internal setting.", TerrainBuild::setWarpMemory);
  command.option("-R", "--resume", "Do not overwrite existing files", TerrainBuild::setResume);
  command.option("-g", "--mesh-qfactor <factor>", "specify the factor to multiply the estimated geometric error to convert heightmaps to irregular meshes. Larger values should mean minor quality. Defaults to 1.0", TerrainBuild::setMeshQualityFactor);
  command.option("-l", "--layer", "only output the layer.json metadata file", TerrainBuild::setMetadata);
  command.option("-q", "--quiet", "only output errors", TerrainBuild::setQuiet);
  command.option("-v", "--verbose", "be more noisy", TerrainBuild::setVerbose);

  // Parse and check the arguments
  command.parse(argc, argv);
  command.check();

  GDALAllRegister();

  // Set the output type
  if (command.verbosity > 1) {
    progressFunc = verboseProgress; // noisy
  } else if (command.verbosity < 1) {
    progressFunc = GDALDummyProgress; // quiet
  }

  // Check whether or not the output directory exists
  VSIStatBufL stat;
  if (VSIStatExL(command.outputDir, &stat, VSI_STAT_EXISTS_FLAG | VSI_STAT_NATURE_FLAG)) {
    cerr << "Error: The output directory does not exist: " << command.outputDir << endl;
    return 1;
  } else if (!VSI_ISDIR(stat.st_mode)) {
    cerr << "Error: The output filepath is not a directory: " << command.outputDir << endl;
    return 1;
  }

  // Define the grid we are going to use
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

  // Run the tilers in separate threads
  vector<future<int>> tasks;
  int threadCount = (command.threadCount > 0) ? command.threadCount : CPLGetNumCPUs();

  // Calculate metadata?
  const string dirname = string(command.outputDir) + osDirSep;
  const std::string filename = concat(dirname, "layer.json");
  TerrainMetadata *metadata = command.metadata || !fileExists(filename) ? new TerrainMetadata() : NULL;

  // Instantiate the threads using futures from a packaged_task
  for (int i = 0; i < threadCount ; ++i) {
    packaged_task<int(TerrainBuild *, Grid *, TerrainMetadata *)> task(runTiler); // wrap the function
    tasks.push_back(task.get_future());                        // get a future
    thread(move(task), &command, &grid, metadata).detach(); // launch on a thread
  }

  // Synchronise the completion of the threads
  for (auto &task : tasks) {
    task.wait();
  }

  // Get the value from the futures
  for (auto &task : tasks) {
    int retval = task.get();

    // return on the first encountered problem
    if (retval) {
      delete metadata;
      return retval;
    }
  }

  // Write Json metadata file?
  if (metadata) {
    std::string datasetName(command.getInputFilename());
    datasetName = datasetName.substr(datasetName.find_last_of("/\\") + 1);
    const size_t rfindpos = datasetName.rfind('.');
    if (std::string::npos != rfindpos) datasetName = datasetName.erase(rfindpos);

    metadata->writeJsonFile(filename, datasetName, std::string(command.outputFormat));
    delete metadata;
  }

  return 0;
}
