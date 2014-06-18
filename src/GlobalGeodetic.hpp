#ifndef GLOBALGEODETIC_HPP
#define GLOBALGEODETIC_HPP

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
 * @file GlobalGeodetic.hpp
 * @brief This defines and declares the `GlobalGeodetic` class
 */

#include <cmath>

#include "config.hpp"
#include "TileCoordinate.hpp"
#include "Bounds.hpp"

namespace terrain {
  class GlobalGeodetic;
}

/**
 * @brief An implementation of the TMS Global Geodetic Profile
 *
 * This class models the [Tile Mapping Service Global Geodetic
 * Profile](http://wiki.osgeo.org/wiki/Tile_Map_Service_Specification#global-geodetic).
 * It provides functionality such as relating a latitude and longitude to a
 * tile (see `GlobalGeodetic::latLonToTile`) and getting the latitude and
 * longitude bounds associated with a tile (see `GlobalGeodetic::tileBounds`).
 *
 * The code here is adapted from the logic in the `gdal2tiles.py` script
 * available with the GDAL library.
 */
class terrain::GlobalGeodetic {
public:

  /// Initialise the profile with a specific tile size
  GlobalGeodetic(i_tile tileSize = TILE_SIZE):
    mTileSize(tileSize),
    mInitialResolution(180.0 / tileSize)
  {}

  /// Get the resolution for a particular zoom level
  inline double
  resolution(i_zoom zoom) const {
    return mInitialResolution / pow(2, zoom);
  }

  /**
   * @brief Get the zoom level for a particular resolution
   *
   * If the resolution does not exactly match a zoom level then the zoom level
   * is 'rounded up' to the next level.
   */
  inline i_zoom
  zoomForResolution(double resolution) const {
    return (i_zoom) ceil(log2(mInitialResolution) - log2(resolution));
  }

  /// Get the pixel location for a specific latitude, longitude and zoom
  inline PixelPoint
  latLonToPixels(const LatLon &latLon, i_zoom zoom) const {
    double res = resolution(zoom);
    i_pixel px = (180 + latLon.x) / res,
      py = (90 + latLon.y) / res;

    return PixelPoint(px, py);
  }

  /// Get the tile x and y associated with a pixel location
  inline TilePoint
  pixelsToTile(const PixelPoint &pixel) const {
    i_tile tx = (i_tile) ceil(pixel.x / mTileSize),
      ty = (i_tile) ceil(pixel.y / mTileSize);

    return TilePoint(tx, ty);
  }

  /// Get the tile coordinate in which a location falls at a specific zoom level
  inline TileCoordinate
  latLonToTile(const LatLon &latLon, i_zoom zoom) const {
    const PixelPoint pixel = latLonToPixels(latLon, zoom);
    TilePoint tile = pixelsToTile(pixel);

    return TileCoordinate(zoom, tile);
  }

  /// Get the bounds in latitude and longitude of a particular tile
  inline LatLonBounds
  tileBounds(const TileCoordinate &coord) const {
    double res = resolution(coord.zoom);
    double minx = coord.x * mTileSize * res - 180,
      miny = coord.y * mTileSize * res - 90,
      maxx = (coord.x + 1) * mTileSize * res - 180,
      maxy = (coord.y + 1) * mTileSize * res - 90;

    return LatLonBounds(minx, miny, maxx, maxy);
  }

  /// Get the tile size associated with this profile
  inline i_tile
  tileSize() const {
    return mTileSize;
  }
  
private:
  /// The tile size associated with this profile
  i_tile mTileSize;

  /// The initial resolution of this particular profile
  double mInitialResolution;
};

#endif /* GLOBALGEODETIC_HPP */
