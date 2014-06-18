#ifndef TILECOORDINATE_HPP
#define TILECOORDINATE_HPP

/**
 * @file TileCoordinate.hpp
 * @brief This declares and defines the `TileCoordinate` class
 */

#include "types.hpp"

namespace terrain {
  class TileCoordinate;
}

/**
 * @brief A `TileCoordinate` identifies a particular tile
 *
 * An instance of this class is composed of a tile point and a zoom level:
 * together this identifies an individual tile.
 */
class terrain::TileCoordinate:
  public TilePoint {
public:

  /// Create the 0-0-0 level tile coordinate
  TileCoordinate():
    TilePoint(0, 0),
    zoom(0)
  {}

  /// The const copy constructor
  TileCoordinate(const TileCoordinate &other):
    TilePoint(other.x, other.y),
    zoom(other.zoom)
  {}

  /// Instantiate a tile coordinate from the zoom, x and y
  TileCoordinate(i_zoom zoom, i_tile x, i_tile y):
    TilePoint(x, y),
    zoom(zoom)
  {}

  /// Instantiate a tile coordinate using the zoom and a tile point
  TileCoordinate(i_zoom zoom, const TilePoint &coord):
    TilePoint(coord),
    zoom(zoom)
  {}

  /// Override the equality operator
  inline bool
  operator==(const TileCoordinate &other) const {
    return dynamic_cast<const TilePoint &>(*this) == dynamic_cast<const TilePoint &>(other)
      && zoom == other.zoom;
  }

  /// Override the assignment operator
  inline void
  operator=(const TileCoordinate &other) {
    dynamic_cast<TilePoint &>(*this) = dynamic_cast<const TilePoint &>(other);
    zoom = other.zoom;
  }

  /// Set the point
  inline void
  setPoint(const TilePoint &point) {
    dynamic_cast<TilePoint &>(*this) = point;
  }

  i_zoom zoom;                  ///< The zoom level
};

#endif /* TILECOORDINATE_HPP */
