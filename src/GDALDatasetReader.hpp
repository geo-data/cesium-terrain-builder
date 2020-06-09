#ifndef GDALDATASETREADER_HPP
#define GDALDATASETREADER_HPP

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
 * @file GDALDatasetReader.hpp
 * @brief This declares the `GDALDatasetReader` class
 */

#include <string>
#include <vector>
#include "gdalwarper.h"

#include "TileCoordinate.hpp"
#include "GDALTiler.hpp"

namespace ctb {
  class GDALDatasetReader;
  class GDALDatasetReaderWithOverviews;
}

/**
 * @brief Read raster tiles from a GDAL Dataset
 *
 * This abstract base class is associated with a GDAL dataset. 
 * It allows to read a region of the raster according to
 * a region defined by a Tile Coordinate.
 *
 * We can define our own
 */
class CTB_DLL ctb::GDALDatasetReader {
public:
  /// Read a region of raster heights into an array for the specified Dataset and Coordinate
  static float *
  readRasterHeights(const GDALTiler &tiler, GDALDataset *dataset, const TileCoordinate &coord, ctb::i_tile tileSizeX, ctb::i_tile tileSizeY);

  /// Read a region of raster heights into an array for the specified Dataset and Coordinate
  virtual float *
  readRasterHeights(GDALDataset *dataset, const TileCoordinate &coord, ctb::i_tile tileSizeX, ctb::i_tile tileSizeY) = 0;

protected:
  /// Create a raster tile from a tile coordinate
  static GDALTile *
  createRasterTile(const GDALTiler &tiler, GDALDataset *dataset, const TileCoordinate &coord);

  /// Create a VTR raster overview from a GDALDataset
  static GDALDataset *
  createOverview(const GDALTiler &tiler, GDALDataset *dataset, const TileCoordinate &coord, int overviewIndex);
};

/** 
 * @brief Implements a GDALDatasetReader that takes care of 'Integer overflow' errors.
 * 
 * This class creates Overviews to avoid 'Integer overflow' errors when extracting 
 * raster data.
 */
class CTB_DLL ctb::GDALDatasetReaderWithOverviews : public ctb::GDALDatasetReader {
public:

  /// Instantiate a GDALDatasetReaderWithOverviews
  GDALDatasetReaderWithOverviews(const GDALTiler &tiler): 
    poTiler(tiler), 
    mOverviewIndex(0) {}

  /// The destructor
  ~GDALDatasetReaderWithOverviews();

  /// Read a region of raster heights into an array for the specified Dataset and Coordinate
  virtual float *
  readRasterHeights(GDALDataset *dataset, const TileCoordinate &coord, ctb::i_tile tileSizeX, ctb::i_tile tileSizeY) override;

  /// Releases all overviews
  void reset();

protected:
  /// The tiler to use
  const GDALTiler &poTiler;

  /// List of VRT Overviews of the underlying GDAL dataset
  std::vector<GDALDataset *> mOverviews;
  /// Current VRT Overview
  int mOverviewIndex;
};

#endif /* GDALDATASETREADER_HPP */
