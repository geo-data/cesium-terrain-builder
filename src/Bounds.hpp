#ifndef BOUNDS_HPP
#define BOUNDS_HPP

#include "Coordinate.hpp"

namespace terrain {
  template <class T> class Bounds;
}

template <class T>
class terrain::Bounds {
public:
  Bounds() {
    bounds[0] = bounds[1] = bounds[2] = bounds[3] = 0;
  }

  Bounds(T minx, T miny, T maxx, T maxy) {
    setBounds(minx, miny, maxx, maxy);
  }

  Bounds(const terrain::Coordinate<T> &lowerLeft, const terrain::Coordinate<T> &upperRight) {
    setBounds(lowerLeft, upperRight);
  }

  inline void setBounds(T minx, T miny, T maxx, T maxy) {
    bounds[0] = minx;
    bounds[1] = miny;
    bounds[2] = maxx;
    bounds[3] = maxy;
  }

  inline void setBounds(const terrain::Coordinate<T> &lowerLeft, const terrain::Coordinate<T> &upperRight) {
    bounds[0] = lowerLeft.x;
    bounds[1] = lowerLeft.y;
    bounds[2] = upperRight.x;
    bounds[3] = upperRight.y;
  }

  inline T getMinX() const {
    return bounds[0];
  }
  inline T getMinY() const {
    return bounds[1];
  }
  inline T getMaxX() const {
    return bounds[2];
  }
  inline T getMaxY() const {
    return bounds[3];
  }

  inline void setMinX(T newValue) {
    bounds[0] = newValue;
  }
  inline void setMinY(T newValue) {
    bounds[1] = newValue;
  }
  inline void setMaxX(T newValue) {
    bounds[2] = newValue;
  }
  inline void setMaxY(T newValue) {
    bounds[3] = newValue;
  }

  inline terrain::Coordinate<T>
  getLowerLeft() const {
    return Coordinate<T>(getMinX(), getMinY());
  }
  inline terrain::Coordinate<T>
  getLowerRight() const {
    return Coordinate<T>(getMaxX(), getMinY());
  }
  inline terrain::Coordinate<T>
  getUpperRight() const {
    return Coordinate<T>(getMaxX(), getMaxY());
  }
  inline terrain::Coordinate<T>
  getUpperLeft() const {
    return terrain::Coordinate<T>(getMinX(), getMaxY());
  }

  inline T getWidth() const {
    return getMaxX() - getMinX();
  }

  inline T getHeight() const {
    return getMaxY() - getMinY();
  }

  inline terrain::Bounds<T> * getSW() const {
    return new terrain::Bounds<T>(getMinX(),
                      getMinY(),
                      getMinX() + (getWidth() / 2),
                      getMinY() + (getHeight() / 2));
  }
  inline terrain::Bounds<T> * getNW() const {
    return new terrain::Bounds<T>(getMinX(),
                      getMaxY() - (getHeight() / 2),
                      getMinX() + (getWidth() / 2),
                      getMaxY());
  }
  inline terrain::Bounds<T> * getNE() const {
    return new terrain::Bounds<T>(getMaxX() - (getWidth() / 2),
                      getMaxY() - (getHeight() / 2),
                      getMaxX(),
                      getMaxY());
  }
  inline terrain::Bounds<T> * getSE() const {
    return new terrain::Bounds<T>(getMaxX() - (getWidth() / 2),
                      getMinY(),
                      getMaxX(),
                      getMinY() + (getHeight() / 2));
  }
  
  inline bool overlaps(const terrain::Bounds<T> *other) const {
    return overlaps(*other);
  }

  inline bool overlaps(const terrain::Bounds<T> &other) const {
    // see
    // <http://stackoverflow.com/questions/306316/determine-if-two-rectangles-overlap-each-other>
    return getMinX() < other.getMaxX() && other.getMinX() < getMaxX() &&
           getMinY() < other.getMaxY() && other.getMinX() < getMaxY();
  }
  
private:
  T bounds[4];
};

#endif /* BOUNDS_HPP */
