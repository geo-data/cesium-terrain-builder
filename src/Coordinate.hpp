#ifndef COORDINATE_HPP
#define COORDINATE_HPP

/**
 * @file Coordinate.hpp
 * @brief This declares and defines the `Coordinate` class
 */

namespace terrain {
  template <class T> class Coordinate;
}

/// A representation of a point coordinate
template <class T>
class terrain::Coordinate {
public:

  /// Create an empty coordinate
  Coordinate():
    x(0),
    y(0)
  {}

  /// The const copy constructor
  Coordinate(const Coordinate &other):
    x(other.x),
    y(other.y)
  {}

  /// Instantiate a coordinate from an x and y value
  Coordinate(T x, T y):
    x(x),
    y(y)
  {}

  /// Overload the equality operator
  virtual bool
  operator==(const Coordinate &other) const {
    return x == other.x
      && y == other.y;
  }

  /// Overload the assignment operator
  virtual void
  operator=(const Coordinate &other) {
    x = other.x;
    y = other.y;
  }

  T x, y;                       ///< The x and y coordinate members
};

#endif /* COORDINATE_HPP */
