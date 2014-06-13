#ifndef TILECOORDINATE_HPP
#define TILECOORDINATE_HPP

#include "types.hpp"

namespace terrain {
  class TileCoordinate;
}

class terrain::TileCoordinate:
  public terrain::TilePoint {
public:
  TileCoordinate():
    TilePoint(0, 0),
    zoom(0)
  {}

  TileCoordinate(const terrain::TileCoordinate &other):
    TilePoint(other.x, other.y),
    zoom(other.zoom)
  {}

  TileCoordinate(terrain::i_zoom zoom, terrain::i_tile x, terrain::i_tile y):
    TilePoint(x, y),
    zoom(zoom)
  {}

  TileCoordinate(terrain::i_zoom zoom, const terrain::TilePoint &coord):
    TilePoint(coord),
    zoom(zoom)
  {}

  bool
  operator==(const terrain::TileCoordinate &other) const {
    return dynamic_cast<const terrain::TilePoint &>(*this) == dynamic_cast<const terrain::TilePoint &>(other)
      && zoom == other.zoom;
  }

  void
  setPoint(const terrain::TilePoint &point) {
    x = point.x;
    y = point.y;
  }

  terrain::i_zoom zoom;
};

#endif /* TILECOORDINATE_HPP */
