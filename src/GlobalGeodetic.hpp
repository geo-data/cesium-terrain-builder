#ifndef GLOBALGEODETIC_HPP
#define GLOBALGEODETIC_HPP

/* 
   Based on gdal2tiles.py
*/

#include <cmath>

#include "config.hpp"
#include "TileCoordinate.hpp"
#include "Bounds.hpp"

namespace terrain {
  class GlobalGeodetic;
}

class terrain::GlobalGeodetic {
public:
  GlobalGeodetic(i_tile tileSize = TILE_SIZE):
    mTileSize(tileSize),
    mInitialResolution(180.0 / tileSize)
  {}

  inline double resolution(i_zoom zoom) const {
    return mInitialResolution / pow(2, zoom);
  }

  inline i_zoom zoomForResolution(double resolution) const {
    return (i_zoom) ceil(log2(mInitialResolution) - log2(resolution));
  }

  inline PixelPoint
  latLonToPixels(const LatLon &latLon, i_zoom zoom) const {
    double res = resolution(zoom);
    i_pixel px = (180 + latLon.x) / res,
      py = (90 + latLon.y) / res;

    return PixelPoint(px, py);
  }

  inline TilePoint
  pixelsToTile(const PixelPoint &pixel) const {
    i_tile tx = (i_tile) ceil(pixel.x / mTileSize),
      ty = (i_tile) ceil(pixel.y / mTileSize);

    return TilePoint(tx, ty);
  }

  inline TileCoordinate
  latLonToTile(const LatLon &latLon, i_zoom zoom) const {
    const PixelPoint pixel = latLonToPixels(latLon, zoom);
    TilePoint tile = pixelsToTile(pixel);

    return TileCoordinate(zoom, tile);
  }

  inline LatLonBounds
  tileBounds(const TileCoordinate &coord) const {
    double res = resolution(coord.zoom);
    double minx = coord.x * mTileSize * res - 180,
      miny = coord.y * mTileSize * res - 90,
      maxx = (coord.x + 1) * mTileSize * res - 180,
      maxy = (coord.y + 1) * mTileSize * res - 90;

    return LatLonBounds(minx, miny, maxx, maxy);
  }

  inline i_tile tileSize() const {
    return mTileSize;
  }
  
private:
  i_tile mTileSize;
  double mInitialResolution;
};

#endif /* GLOBALGEODETIC_HPP */
