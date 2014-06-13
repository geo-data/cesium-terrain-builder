#ifndef GLOBALGEODETIC_HPP
#define GLOBALGEODETIC_HPP

/* 
   Based on gdal2tiles.py
*/

#include <cmath>

#include "geo-types.hpp"
#include "TileCoordinate.hpp"
#include "Bounds.hpp"

typedef Coordinate<unsigned int> PixelPoint;

class GlobalGeodetic {
public:
  GlobalGeodetic(unsigned int tileSize = 65):
    mTileSize(tileSize),
    mInitialResolution(180.0 / tileSize)
  {}

  inline double resolution(unsigned short int zoom) const {
    return mInitialResolution / pow(2, zoom);
  }

  inline unsigned short int zoomForResolution(double resolution) const {
    return (unsigned short int) ceil(log2(mInitialResolution) - log2(resolution));
  }

  inline PixelPoint
  latLonToPixels(const LatLon &latLon, unsigned short int zoom) const {
    double res = resolution(zoom);
    unsigned int px = (180 + latLon.x) / res,
      py = (90 + latLon.y) / res;

    return PixelPoint(px, py);
  }

  inline TilePoint
  pixelsToTile(const PixelPoint &pixel) const {
    unsigned int tx = (unsigned int) ceil(pixel.x / mTileSize),
      ty = (unsigned int) ceil(pixel.y / mTileSize);

    return TilePoint(tx, ty);
  }

  inline TileCoordinate
  latLonToTile(const LatLon &latLon, unsigned short int zoom) const {
    const PixelPoint pixel = latLonToPixels(latLon, zoom);
    TilePoint tile = pixelsToTile(pixel);

    return TileCoordinate(zoom, tile);
  }

  inline Bounds
  tileBounds(const TileCoordinate &coord) const {
    double res = resolution(coord.zoom);
    double minx = coord.x * mTileSize * res - 180,
      miny = coord.y * mTileSize * res - 90,
      maxx = (coord.x + 1) * mTileSize * res - 180,
      maxy = (coord.y + 1) * mTileSize * res - 90;

    return Bounds(minx, miny, maxx, maxy);
  }

  inline unsigned int tileSize() const {
    return mTileSize;
  }
  
private:
  unsigned int mTileSize;
  double mInitialResolution;
};

#endif /* GLOBALGEODETIC_HPP */
