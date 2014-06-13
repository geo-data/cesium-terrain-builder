#ifndef TILECOORDINATE_HPP
#define TILECOORDINATE_HPP

#include "Coordinate.hpp"

typedef Coordinate<unsigned int> TilePoint;

class TileCoordinate:
  public TilePoint {
public:
  TileCoordinate():
    TilePoint(0, 0),
    zoom(0)
  {}

  TileCoordinate(const TileCoordinate &other):
    TilePoint(other.x, other.y),
    zoom(other.zoom)
  {}

  TileCoordinate(unsigned short int zoom, unsigned int x, unsigned int y):
    TilePoint(x, y),
    zoom(zoom)
  {}

  TileCoordinate(unsigned short int zoom, const TilePoint &coord):
    TilePoint(coord),
    zoom(zoom)
  {}

  bool
  operator==(const TileCoordinate &other) const {
    return dynamic_cast<const TilePoint &>(*this) == dynamic_cast<const TilePoint &>(other)
      && zoom == other.zoom;
  }

  void
  setPoint(const TilePoint &point) {
    x = point.x;
    y = point.y;
  }

  unsigned short int zoom;
};

#endif /* TILECOORDINATE_HPP */
