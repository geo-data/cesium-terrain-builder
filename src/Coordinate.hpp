#ifndef COORDINATE_HPP
#define COORDINATE_HPP

class Coordinate {
public:
  Coordinate():
    x(0),
    y(0)
  {}

  Coordinate(const Coordinate &other):
    x(other.x),
    y(other.y)
  {}

  Coordinate(unsigned int x, unsigned int y):
    x(x),
    y(y)
  {}

  virtual bool
  operator==(const Coordinate &other) const {
    return x == other.x
      && y == other.y;
  }

  unsigned int x;
  unsigned int y;
};

#endif /* COORDINATE_HPP */
