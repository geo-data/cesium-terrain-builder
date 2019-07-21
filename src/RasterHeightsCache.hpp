#ifndef CTBRASTERHEIGHTSCACHE_HPP
#define CTBRASTERHEIGHTSCACHE_HPP

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
 * @file RasterHeightsCache.hpp
 * @brief This declares and defines the `RasterHeightsBuff` and `RasterHeightsCache` classes
 */

#include <memory>
#include <vector>
#include <algorithm>

#include "TileCoordinate.hpp"

namespace ctb {
  class RasterHeightsBuff;
  class RasterHeightsCache;
}

/**
 * @brief Links `TileCoordinate` with a region of data of a GDAL Dataset
 *
 * This class manages the region of data of a GDAL Dataset related a `TileCoordinate`.
 */
class ctb::RasterHeightsBuff
{
public:

  RasterHeightsBuff(const ctb::TileCoordinate &coord, float *rasterHeights) : mCoord(coord), mHeights(rasterHeights) {
  }
  ~RasterHeightsBuff() {
    if (mHeights) {
      CPLFree(mHeights);
      mHeights = NULL;
    }
  }

  /// Function comparer to select the oldest `RasterHeightsBuff` class.
  static bool olderThanFunction(const std::shared_ptr<ctb::RasterHeightsBuff> &a, const std::shared_ptr<ctb::RasterHeightsBuff> &b) {
    if (a->mCoord.zoom != b->mCoord.zoom) {
      return a->mCoord.zoom > b->mCoord.zoom;
    }
    else {
      return a->mCoord.x != b->mCoord.x ? a->mCoord.x < b->mCoord.x : a->mCoord.y < b->mCoord.y;
    }
  }

  TileCoordinate mCoord;
  float *mHeights;
};

/**
 * @brief Very simple cache of `RasterHeightsBuff` class
 *
 * This class represents a cache of `RasterHeightsBuff` tiles to speed up the iteration
 * of a `MeshTiler` when it is taking care of the borders of neighbours of a tile.
 */
class ctb::RasterHeightsCache
{
public:
  RasterHeightsCache(size_t cacheSize = 3) : mCacheSize(cacheSize) {
  }

  /// Returns if exist the `RasterHeightsBuff` of the specified Coordinate, otherwise returns null.
  ctb::RasterHeightsBuff *get(const ctb::TileCoordinate &coord) const {
    for (size_t i = 0, icount = mCache.size(); i < icount; i++) {
      const std::shared_ptr<ctb::RasterHeightsBuff>& item = mCache.at(i);
      if (item->mCoord == coord) {
        return item.get();
      }
    }
    return NULL;
  }

  /// Puts in the cache the specified `RasterHeightsBuff`.
  void push(ctb::RasterHeightsBuff *heightsBuff) {
    size_t size = mCache.size();

    if (size < mCacheSize) {
      mCache.push_back(std::shared_ptr<ctb::RasterHeightsBuff>(heightsBuff));
      return;
    }
    std::size_t i = std::min_element(mCache.begin(), mCache.end(), ctb::RasterHeightsBuff::olderThanFunction) - mCache.begin();
    mCache.at(i).reset(heightsBuff);
  }

private:

  /// The size of this Cache.
  size_t mCacheSize;

  /// The cache of Heights.
  std::vector<std::shared_ptr<ctb::RasterHeightsBuff>> mCache;
};

#endif /* CTBRASTERHEIGHTSCACHE_HPP */
