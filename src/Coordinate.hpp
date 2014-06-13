#ifndef COORDINATE_HPP
#define COORDINATE_HPP

namespace terrain {
  template <class T> class Coordinate;
}

template <class T>
class terrain::Coordinate {
public:
  Coordinate():
    x(0),
    y(0)
  {}

  Coordinate(const Coordinate &other):
    x(other.x),
    y(other.y)
  {}

  Coordinate(T x, T y):
    x(x),
    y(y)
  {}

  virtual bool
  operator==(const Coordinate &other) const {
    return x == other.x
      && y == other.y;
  }

  T x, y;
};

#endif /* COORDINATE_HPP */
