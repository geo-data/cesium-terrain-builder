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

#include "GlobalMercator.hpp"
#include "RasterIterator.hpp"
#include "TerrainIterator.hpp"

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
    verbosity(1)
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

  const char *outputDir,
    *outputFormat,
    *profile;

  int threadCount,
    tileSize,
    startZoom,
    endZoom,
    verbosity;

  CPLStringList creationOptions;
  TilerOptions tilerOptions;
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
  string filename = dirname + static_cast<ostringstream*>
    (
     &(ostringstream()
       << coord->zoom
       << osDirSep
       << coord->x)
     )->str();

  lock_guard<std::mutex> lock(mutex);

  // Check whether the `{zoom}/{x}` directory exists or not
  if (VSIStatExL(filename.c_str(), &stat, VSI_STAT_EXISTS_FLAG | VSI_STAT_NATURE_FLAG)) {
    filename = dirname + static_cast<ostringstream*>(&(ostringstream() << coord->zoom))->str();

    // Check whether the `{zoom}` directory exists or not
    if (VSIStatExL(filename.c_str(), &stat, VSI_STAT_EXISTS_FLAG | VSI_STAT_NATURE_FLAG)) {
      // Create the `{zoom}` directory
      if (VSIMkdir(filename.c_str(), 0755))
        throw CTBException("Could not create the zoom level directory");

    } else if (!VSI_ISDIR(stat.st_mode)) {
      throw CTBException("Zoom level file path is not a directory");
    }

    // Create the `{zoom}/{x}` directory
    filename += static_cast<ostringstream*>(&(ostringstream() << osDirSep << coord->x))->str();
    if (VSIMkdir(filename.c_str(), 0755))
      throw CTBException("Could not create the x level directory");

  } else if (!VSI_ISDIR(stat.st_mode)) {
    throw CTBException("X level file path is not a directory");
  }

  // Create the filename itself, adding the extension if required
  filename += static_cast<ostringstream*>(&(ostringstream() << osDirSep << coord->y))->str();
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
termProgress(double dfComplete, const char *pszMessage, void *pProgressArg) {
  static mutex mutex;          // GDALTermProgress isn't thread safe, so lock it
  int status;

  lock_guard<std::mutex> lock(mutex);
  status = GDALTermProgress(dfComplete, pszMessage, pProgressArg);

  return status;
}

/// In a thread safe manner describe the file just created
static int
verboseProgress(double dfComplete, const char *pszMessage, void *pProgressArg) {
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

/// Output GDAL tiles represented by a tiler to a directory
static void
buildGDAL(const RasterTiler &tiler, TerrainBuild *command) {
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
    GDALTile *tile = *iter;
    GDALDataset *poDstDS;
    const string filename = getTileFilename(tile, dirname, extension);

    poDstDS = poDriver->CreateCopy(filename.c_str(), tile->dataset, FALSE,
                                   command->creationOptions.List(), NULL, NULL );
    delete tile;

    // Close the datasets, flushing data to destination
    if (poDstDS == NULL) {
      throw CTBException("Could not create GDAL tile");
    }

    GDALClose(poDstDS);

    currentIndex = incrementIterator(iter, currentIndex);
    showProgress(currentIndex, filename);
  }
}

/// Output terrain tiles represented by a tiler to a directory
static void
buildTerrain(const TerrainTiler &tiler, TerrainBuild *command) {
  const string dirname = string(command->outputDir) + osDirSep;
  i_zoom startZoom = (command->startZoom < 0) ? tiler.maxZoomLevel() : command->startZoom,
    endZoom = (command->endZoom < 0) ? 0 : command->endZoom;

  TerrainIterator iter(tiler, startZoom, endZoom);
  int currentIndex = incrementIterator(iter, 0);
  setIteratorSize(iter);

  while (!iter.exhausted()) {
    TerrainTile *tile = *iter;
    const string filename = getTileFilename(tile, dirname, "terrain");

    tile->writeFile(filename.c_str());
    delete tile;

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
runTiler(TerrainBuild *command, Grid *grid) {
  GDALDataset  *poDataset = (GDALDataset *) GDALOpen(command->getInputFilename(), GA_ReadOnly);
  if (poDataset == NULL) {
    cerr << "Error: could not open GDAL dataset" << endl;
    return 1;
  }

  try {
    if (strcmp(command->outputFormat, "Terrain") == 0) {
      const TerrainTiler tiler(poDataset, *grid);
      buildTerrain(tiler, command);
    } else {                    // it's a GDAL format
      const RasterTiler tiler(poDataset, *grid, command->tilerOptions);
      buildGDAL(tiler, command);
    }

  } catch (CTBException &e) {
    cerr << "Error: " << e.what() << endl;
  }

  GDALClose(poDataset);

  return 0;
}

int
main(int argc, char *argv[]) {
  // Specify the command line interface
  TerrainBuild command = TerrainBuild(argv[0], version.cstr);
  command.setUsage("[options] GDAL_DATASOURCE");
  command.option("-o", "--output-dir <dir>", "specify the output directory for the tiles (defaults to working directory)", TerrainBuild::setOutputDir);
  command.option("-f", "--output-format <format>", "specify the output format for the tiles. This is either `Terrain` (the default) or any format listed by `gdalinfo --formats`", TerrainBuild::setOutputFormat);
  command.option("-p", "--profile <profile>", "specify the TMS profile for the tiles. This is either `geodetic` (the default) or `mercator`", TerrainBuild::setProfile);
  command.option("-c", "--thread-count <count>", "specify the number of threads to use for tile generation. On multicore machines this defaults to the number of CPUs", TerrainBuild::setThreadCount);
  command.option("-t", "--tile-size <size>", "specify the size of the tiles in pixels. This defaults to 65 for terrain tiles and 256 for other GDAL formats", TerrainBuild::setTileSize);
  command.option("-s", "--start-zoom <zoom>", "specify the zoom level to start at. This should be greater than the end zoom level", TerrainBuild::setStartZoom);
  command.option("-e", "--end-zoom <zoom>", "specify the zoom level to end at. This should be less than the start zoom level and >= 0", TerrainBuild::setEndZoom);
  command.option("-n", "--creation-option <option>", "specify a GDAL creation option for the output dataset in the form NAME=VALUE. Can be specified multiple times. Not valid for Terrain tiles.", TerrainBuild::addCreationOption);
  command.option("-z", "--error-threshold <threshold>", "specify the error threshold in pixel units for transformation approximation. Larger values should mean faster transforms. Defaults to 0.125", TerrainBuild::setErrorThreshold);
  command.option("-m", "--warp-memory <bytes>", "The memory limit in bytes used for warp operations. Higher settings should be faster. Defaults to a conservative GDAL internal setting.", TerrainBuild::setWarpMemory);
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

  // Instantiate the threads using futures from a packaged_task
  for (int i = 0; i < threadCount ; ++i) {
    packaged_task<int(TerrainBuild *, Grid *)> task(runTiler); // wrap the function
    tasks.push_back(task.get_future());                        // get a future
    thread(move(task), &command, &grid).detach(); // launch on a thread
  }

  // Synchronise the completion of the threads
  for (auto &task : tasks) {
    task.wait();
  }

  // Get the value from the futures
  for (auto &task : tasks) {
    int retval = task.get();

    // return on the first encountered problem
    if (retval)
      return retval;
  }

  return 0;
}
