#ifndef BOUNDS_HPP
#define BOUNDS_HPP

/*******************************************************************************
 * Copyright 2014 GeoData <geodata@soton.ac.uk>
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *******************************************************************************/

/**
 * @file Bounds.hpp
 * @brief This declares and defines the `Bounds` class
 */

#include "Coordinate.hpp"
#include "CTBException.hpp"

namespace ctb {
  template <class T> class Bounds;
}

/// A representation of an extent
template <class T>
class ctb::Bounds {
public:

  /// Create an empty bounds
  Bounds() {
    bounds[0] = bounds[1] = bounds[2] = bounds[3] = 0;
  }

  /// Create bounds from individual extents
  Bounds(T minx, T miny, T maxx, T maxy) {
    setBounds(minx, miny, maxx, maxy);
  }

  /// Create bounds represented by lower left and upper right coordinates
  Bounds(const Coordinate<T> &lowerLeft, const Coordinate<T> &upperRight) {
    setBounds(lowerLeft, upperRight);
  }

  /// Overload the equality operator
  virtual bool
  operator==(const Bounds<T> &other) const {
    return bounds[0] == other.bounds[0]
      && bounds[1] == other.bounds[1]
      && bounds[2] == other.bounds[2]
      && bounds[3] == other.bounds[3];
  }

  /// Set the bounds from extents
  inline void
  setBounds(T minx, T miny, T maxx, T maxy) {
    if (minx > maxx) {
      throw CTBException("The minimum X value is greater than the maximum X value");
    } else if (miny > maxy) {
      throw CTBException("The minimum Y value is greater than the maximum Y value");
    }

    bounds[0] = minx;
    bounds[1] = miny;
    bounds[2] = maxx;
    bounds[3] = maxy;
  }

  /// Set the bounds from lower left and upper right coordinates
  inline void
  setBounds(const Coordinate<T> &lowerLeft, const Coordinate<T> &upperRight) {
    setBounds(lowerLeft.x, lowerLeft.y, upperRight.x, upperRight.y);
  }

  /// Get the minimum X value
  inline T
  getMinX() const {
    return bounds[0];
  }

  /// Get the minimum Y value
  inline T
  getMinY() const {
    return bounds[1];
  }

  /// Get the maximum X value
  inline T
  getMaxX() const {
    return bounds[2];
  }

  /// Get the maximum Y value
  inline T
  getMaxY() const {
    return bounds[3];
  }

  /// Set the minimum X value
  inline void
  setMinX(T newValue) {
    if (newValue > getMaxX())
      throw CTBException("The value is greater than the maximum X value");

    bounds[0] = newValue;
  }

  /// Set the minimum Y value
  inline void
  setMinY(T newValue) {
    if (newValue > getMaxY())
      throw CTBException("The value is greater than the maximum Y value");

    bounds[1] = newValue;
  }

  /// Set the maximum X value
  inline void
  setMaxX(T newValue) {
    if (newValue < getMinX())
      throw CTBException("The value is less than the minimum X value");

    bounds[2] = newValue;
  }

  /// Set the maximum Y value
  inline void
  setMaxY(T newValue) {
    if (newValue < getMinY())
      throw CTBException("The value is less than the minimum Y value");

    bounds[3] = newValue;
  }

  /// Get the lower left corner
  inline Coordinate<T>
  getLowerLeft() const {
    return Coordinate<T>(getMinX(), getMinY());
  }

  /// Get the lower right corner
  inline Coordinate<T>
  getLowerRight() const {
    return Coordinate<T>(getMaxX(), getMinY());
  }

  /// Get the upper right corner
  inline Coordinate<T>
  getUpperRight() const {
    return Coordinate<T>(getMaxX(), getMaxY());
  }

  /// Get the upper left corner
  inline Coordinate<T>
  getUpperLeft() const {
    return Coordinate<T>(getMinX(), getMaxY());
  }

  /// Get the width
  inline T
  getWidth() const {
    return getMaxX() - getMinX();
  }

  /// Get the height
  inline T
  getHeight() const {
    return getMaxY() - getMinY();
  }

  /// Get the lower left quarter of the extents
  inline Bounds<T>
  getSW() const {
    return Bounds<T>(getMinX(),
                              getMinY(),
                              getMinX() + (getWidth() / 2),
                              getMinY() + (getHeight() / 2));
  }

  /// Get the upper left quarter of the extents
  inline Bounds<T>
  getNW() const {
    return Bounds<T>(getMinX(),
                              getMaxY() - (getHeight() / 2),
                              getMinX() + (getWidth() / 2),
                              getMaxY());
  }

  /// Get the upper right quarter of the extents
  inline Bounds<T>
  getNE() const {
    return Bounds<T>(getMaxX() - (getWidth() / 2),
                              getMaxY() - (getHeight() / 2),
                              getMaxX(),
                              getMaxY());
  }

  /// Get the lower right quarter of the extents
  inline Bounds<T>
  getSE() const {
    return Bounds<T>(getMaxX() - (getWidth() / 2),
                              getMinY(),
                              getMaxX(),
                              getMinY() + (getHeight() / 2));
  }

  /// Do these bounds overlap with another?
  inline bool
  overlaps(const Bounds<T> *other) const {
    return overlaps(*other);
  }

  /// Do these bounds overlap with another?
  inline bool
  overlaps(const Bounds<T> &other) const {
    // see
    // <http://stackoverflow.com/questions/306316/determine-if-two-rectangles-overlap-each-other>
    return getMinX() < other.getMaxX() && other.getMinX() < getMaxX() &&
           getMinY() < other.getMaxY() && other.getMinY() < getMaxY();
  }
  
private:
  /// The extents themselves as { minx, miny, maxx, maxy }
  T bounds[4];
};

#endif /* BOUNDS_HPP */
