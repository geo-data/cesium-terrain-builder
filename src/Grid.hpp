#ifndef CTBGRID_HPP
#define CTBGRID_HPP

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
 * @file Grid.hpp
 * @brief This defines and declares the `Grid` class
 */

#include <cmath>

#include "ogr_spatialref.h"

#include "types.hpp"
#include "TileCoordinate.hpp"

namespace ctb {
  class Grid;
}

/**
 * @brief A generic grid for cutting tile sets
 *
 * This class models a grid for use in cutting up an area into zoom levels and
 * tiles.  It provides functionality such as relating a coordinate in a native
 * coordinate reference system (CRS) to a tile (see `Grid::crsToTile`) and
 * getting the CRS bounds of a tile (see `Grid::tileBounds`).
 *
 * The `Grid` class should be able to model most grid systems. The
 * `GlobalMercator` and `GlobalGeodetic` subclasses implement the specific Tile
 * Mapping Service grid profiles.
 *
 * The code here generalises the logic in the `gdal2tiles.py` script available
 * with the GDAL library.
 */
class ctb::Grid {
public:

  /// An empty grid
  Grid() {}

  /// Initialise a grid tile
  Grid(i_tile tileSize,
       const CRSBounds extent,
       const OGRSpatialReference srs,
       unsigned short int rootTiles = 1,
       float zoomFactor = 2):
    mTileSize(tileSize),
    mExtent(extent),
    mSRS(srs),
    mInitialResolution((extent.getWidth() / rootTiles) / tileSize ),
    mXOriginShift(extent.getWidth() / 2),
    mYOriginShift(extent.getHeight() / 2),
    mZoomFactor(zoomFactor)
  {
    #if ( GDAL_VERSION_MAJOR >= 3 )
    mSRS.SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);
    #endif
  }

  /// Overload the assignment operator
  Grid &
  operator=(const Grid &other) {
    mTileSize = other.mTileSize;
    mExtent = other.mExtent;
    mSRS = other.mSRS;
    mInitialResolution = other.mInitialResolution;
    mXOriginShift = other.mXOriginShift;
    mYOriginShift = other.mYOriginShift;
    mZoomFactor = other.mZoomFactor;

    return *this;
  }

  /// Override the equality operator
  bool
  operator==(const Grid &other) const {
    return mTileSize == other.mTileSize
      && mExtent == other.mExtent
      && mSRS.IsSame(&(other.mSRS))
      && mInitialResolution == other.mInitialResolution
      && mXOriginShift == other.mXOriginShift
      && mYOriginShift == other.mYOriginShift
      && mZoomFactor == other.mZoomFactor;
  }

  /// Get the resolution for a particular zoom level
  inline double
  resolution(i_zoom zoom) const {
    return mInitialResolution / pow(mZoomFactor, zoom);
  }

  /**
   * @brief Get the zoom level for a particular resolution
   *
   * If the resolution does not exactly match a zoom level then the zoom level
   * is 'rounded up' to the next level.
   */
  inline i_zoom
  zoomForResolution(double resolution) const {
    // if mZoomFactor == 2 the following is the same as using:
    // log2(mInitialResolution) - log2(resolution)
    return (i_zoom) ceil((log(mInitialResolution)/log(mZoomFactor)) - (log(resolution)/log(mZoomFactor)));
  }

  /// Get the tile covering a pixel location
  inline TilePoint
  pixelsToTile(const PixelPoint &pixel) const {
    i_tile tx = (i_tile) (pixel.x / mTileSize),
      ty = (i_tile) (pixel.y / mTileSize);

    return TilePoint(tx, ty);
  }

  /// Convert pixel coordinates at a given zoom level to CRS coordinates
  inline CRSPoint
  pixelsToCrs(const PixelPoint &pixel, i_zoom zoom) const {
    double res = resolution(zoom);

    return CRSPoint((pixel.x * res) - mXOriginShift,
                    (pixel.y * res) - mYOriginShift);
  }

  /// Get the pixel location represented by a CRS point and zoom level
  inline PixelPoint
  crsToPixels(const CRSPoint &coord, i_zoom zoom) const {
    double res = resolution(zoom);
    i_pixel px = (mXOriginShift + coord.x) / res,
      py = (mYOriginShift + coord.y) / res;

    return PixelPoint(px, py);
  }

  /// Get the tile coordinate in which a location falls at a specific zoom level
  inline TileCoordinate
  crsToTile(const CRSPoint &coord, i_zoom zoom) const {
    const PixelPoint pixel = crsToPixels(coord, zoom);
    TilePoint tile = pixelsToTile(pixel);

    return TileCoordinate(zoom, tile);
  }

  /// Get the CRS bounds of a particular tile
  inline CRSBounds
  tileBounds(const TileCoordinate &coord) const {
    // get the pixels coordinates representing the tile bounds
    const PixelPoint pxLowerLeft(coord.x * mTileSize, coord.y * mTileSize),
      pxUpperRight((coord.x + 1) * mTileSize, (coord.y + 1) * mTileSize);

    // convert pixels to native coordinates
    const CRSPoint lowerLeft = pixelsToCrs(pxLowerLeft, coord.zoom),
      upperRight = pixelsToCrs(pxUpperRight, coord.zoom);

    return CRSBounds(lowerLeft, upperRight);
  }

  /// Get the tile size associated with this grid
  inline i_tile
  tileSize() const {
    return mTileSize;
  }

  /// Get the tile size associated with this grid
  inline const OGRSpatialReference &
  getSRS() const {
    return mSRS;
  }

  /// Get the extent covered by the grid in CRS coordinates
  inline const CRSBounds &
  getExtent() const {
    return mExtent;
  }

  /// Get the extent covered by the grid in tile coordinates for a zoom level
  inline TileBounds
  getTileExtent(i_zoom zoom) const {
    TileCoordinate ll = crsToTile(mExtent.getLowerLeft(), zoom),
      ur = crsToTile(mExtent.getUpperRight(), zoom);

    return TileBounds(ll, ur);
  }

protected:

  /// The tile size associated with this grid
  i_tile mTileSize;

  /// The area covered by the grid
  CRSBounds mExtent;

  /// The spatial reference system covered by the grid
  OGRSpatialReference mSRS;

  double mInitialResolution, ///< The initial resolution of this particular profile
    mXOriginShift, ///< The shift in CRS coordinates to get to the origin from minx
    mYOriginShift; ///< The shift in CRS coordinates to get to the origin from miny

  /// By what factor will the scale increase at each zoom level?
  float mZoomFactor;
};

#endif /* CTBGRID_HPP */
