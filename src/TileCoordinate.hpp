#ifndef TILECOORDINATE_HPP
#define TILECOORDINATE_HPP

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

class TileCoordinate:
  public Coordinate {
public:
  TileCoordinate():
    Coordinate(0, 0),
    zoom(0)
  {}

  TileCoordinate(const TileCoordinate &other):
    Coordinate(other.x, other.y),
    zoom(other.zoom)
  {}

  TileCoordinate(unsigned short int zoom, unsigned int x, unsigned int y):
    Coordinate(x, y),
    zoom(zoom)
  {}

  TileCoordinate(unsigned short int zoom, const Coordinate &coord):
    Coordinate(coord),
    zoom(zoom)
  {}

  bool
  operator==(const TileCoordinate &other) const {
    return dynamic_cast<const Coordinate &>(*this) == dynamic_cast<const Coordinate &>(other)
      && zoom == other.zoom;
  }


  unsigned short int zoom;
};

#endif /* TILECOORDINATE_HPP */
