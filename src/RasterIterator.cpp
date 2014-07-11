#include "RasterIterator.hpp"

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
 * @file RasterIterator.cpp
 * @brief This defines the `RasterIterator` class
 */

using namespace ctb;

ctb::RasterIterator::RasterIterator(const GDALTiler &tiler) :
  TilerIterator(tiler)
{}

ctb::RasterIterator::RasterIterator(const GDALTiler &tiler, i_zoom startZoom, i_zoom endZoom):
  TilerIterator(tiler, startZoom, endZoom)
{}

/**
 * @details The `GDALDataset *` is a pointer to a [virtual
 * raster](http://www.gdal.org/gdal_vrttut.html). It is the caller's
 * responsibility to call `GDALClose()` on the returned dataset.
 */
std::pair<const TileCoordinate &, GDALDataset *>
ctb::RasterIterator::operator*() const {
  return std::pair<const TileCoordinate &, GDALDataset *>
    (
     currentTile,
     static_cast<GDALDataset *>(tiler.createRasterTile(currentTile))
     );
}
