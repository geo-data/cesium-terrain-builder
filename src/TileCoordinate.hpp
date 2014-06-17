#ifndef TILECOORDINATE_HPP
#define TILECOORDINATE_HPP

#include "types.hpp"

namespace terrain {
  class TileCoordinate;
}

class terrain::TileCoordinate:
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

  TileCoordinate(i_zoom zoom, i_tile x, i_tile y):
    TilePoint(x, y),
    zoom(zoom)
  {}

  TileCoordinate(i_zoom zoom, const TilePoint &coord):
    TilePoint(coord),
    zoom(zoom)
  {}

  inline bool
  operator==(const TileCoordinate &other) const {
    return dynamic_cast<const TilePoint &>(*this) == dynamic_cast<const TilePoint &>(other)
      && zoom == other.zoom;
  }

  inline void
  setPoint(const TilePoint &point) {
    x = point.x;
    y = point.y;
  }

  i_zoom zoom;
};

#endif /* TILECOORDINATE_HPP */
