#ifndef GLOBALGEODETIC_HPP
#define GLOBALGEODETIC_HPP

/* 
   Based on gdal2tiles.py
*/

#include <cmath>

#include "TileCoordinate.hpp"
#include "Bounds.hpp"

class GlobalGeodetic {
public:
  GlobalGeodetic(unsigned int tileSize = 65):
    mTileSize(tileSize),
    mInitialResolution(180.0 / tileSize)
  { }

  inline double resolution(unsigned short int zoom) const {
    return mInitialResolution / pow(2, zoom);
  }

  inline unsigned short int zoomForResolution(double resolution) const {
    return (unsigned short int) ceil(log2(mInitialResolution) - log2(resolution));
  }

  inline Coordinate
  latLonToPixels(double lat, double lon, unsigned short int zoom) const {
    double res = resolution(zoom);
    unsigned int px = (180 + lat) / res,
      py = (90 + lon) / res;

    return Coordinate(px, py);
  }

  inline Coordinate
  pixelsToTile(Coordinate &pixel) const {
    unsigned int tx = (unsigned int) ceil(pixel.x / mTileSize),
      ty = (unsigned int) ceil(pixel.y / mTileSize);

    return Coordinate(tx, ty);
  }

  inline TileCoordinate
  latLonToTile(double lat, double lon, unsigned short int zoom) const {
    Coordinate pixel = latLonToPixels(lat, lon, zoom);
    Coordinate tile = pixelsToTile(pixel);

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

  inline Bounds
  terrainTileBounds(const TileCoordinate &coord,
                    double& resolution) const {
    unsigned int lTileSize = tileSize() - 1;
    Bounds tile = tileBounds(coord);
    resolution = (tile.getMaxX() - tile.getMinX()) / lTileSize;
    tile.setMinX(tile.getMinX() - resolution);
    tile.setMaxY(tile.getMaxY() + resolution);

    return tile;
  }

  inline unsigned int tileSize() const {
    return mTileSize;
  }
  
private:
  unsigned int mTileSize;
  double mInitialResolution;
};

#endif /* GLOBALGEODETIC_HPP */
