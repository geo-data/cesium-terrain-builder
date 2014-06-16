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
  GlobalGeodetic(terrain::i_tile tileSize = terrain::TILE_SIZE):
    mTileSize(tileSize),
    mInitialResolution(180.0 / tileSize)
  {}

  inline double resolution(terrain::i_zoom zoom) const {
    return mInitialResolution / pow(2, zoom);
  }

  inline terrain::i_zoom zoomForResolution(double resolution) const {
    return (terrain::i_zoom) ceil(log2(mInitialResolution) - log2(resolution));
  }

  inline terrain::PixelPoint
  latLonToPixels(const terrain::LatLon &latLon, terrain::i_zoom zoom) const {
    double res = resolution(zoom);
    terrain::i_pixel px = (180 + latLon.x) / res,
      py = (90 + latLon.y) / res;

    return terrain::PixelPoint(px, py);
  }

  inline terrain::TilePoint
  pixelsToTile(const terrain::PixelPoint &pixel) const {
    terrain::i_tile tx = (terrain::i_tile) ceil(pixel.x / mTileSize),
      ty = (terrain::i_tile) ceil(pixel.y / mTileSize);

    return terrain::TilePoint(tx, ty);
  }

  inline terrain::TileCoordinate
  latLonToTile(const terrain::LatLon &latLon, terrain::i_zoom zoom) const {
    const terrain::PixelPoint pixel = latLonToPixels(latLon, zoom);
    terrain::TilePoint tile = pixelsToTile(pixel);

    return terrain::TileCoordinate(zoom, tile);
  }

  inline terrain::LatLonBounds
  tileBounds(const terrain::TileCoordinate &coord) const {
    double res = resolution(coord.zoom);
    double minx = coord.x * mTileSize * res - 180,
      miny = coord.y * mTileSize * res - 90,
      maxx = (coord.x + 1) * mTileSize * res - 180,
      maxy = (coord.y + 1) * mTileSize * res - 90;

    return terrain::LatLonBounds(minx, miny, maxx, maxy);
  }

  inline terrain::i_tile tileSize() const {
    return mTileSize;
  }
  
private:
  terrain::i_tile mTileSize;
  double mInitialResolution;
};

#endif /* GLOBALGEODETIC_HPP */
