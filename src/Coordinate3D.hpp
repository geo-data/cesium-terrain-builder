#ifndef COORDINATE3D_HPP
#define COORDINATE3D_HPP

/*******************************************************************************
 * Copyright 2018 GeoData <geodata@soton.ac.uk>
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

#include <vector>
#include <cmath>

/**
 * @file Coordinate3D.hpp
 * @brief This declares and defines the `Coordinate3D` class
 * @author Alvaro Huarte <ahuarte47@yahoo.es>
 */

namespace ctb {
  template <class T> class Coordinate3D;
}

/// A representation of a 3-dimensional point coordinate
template <class T>
class ctb::Coordinate3D {
public:
  T x, y, z; ///< The x, y and z coordinate members

  /// Create an empty coordinate
  Coordinate3D():
    x(0),
    y(0),
    z(0)
  {}

  /// The const copy constructor
  Coordinate3D(const Coordinate3D &other):
    x(other.x),
    y(other.y),
    z(other.z)
  {}

  /// Instantiate a coordinate from an x, y and z value
  Coordinate3D(T x, T y, T z):
    x(x),
    y(y),
    z(z)
  {}

  /// Overload the equality operator
  virtual bool
  operator==(const Coordinate3D &other) const {
    return x == other.x
        && y == other.y
        && z == other.z;
  }

  /// Overload the assignment operator
  virtual void
  operator=(const Coordinate3D &other) {
    x = other.x;
    y = other.y;
    z = other.z;
  }

  /// Gets a read-only index-ordinate of the coordinate
  inline virtual T operator[](const int index) const {
    return (index == 0) ? x : (index == 1 ? y : z);
  }

  /// Add operator
  inline virtual Coordinate3D operator+(const Coordinate3D& other) const {
    return Coordinate3D(x + other.x, y + other.y, z + other.z);
  }
  /// Subtract operator
  inline virtual Coordinate3D operator-(const Coordinate3D& other) const {
    return Coordinate3D(x - other.x, y - other.y, z - other.z);
  }
  /// Multiply operator
  inline virtual Coordinate3D operator*(const Coordinate3D& other) const {
    return Coordinate3D(x * other.x, y * other.y, z * other.z);
  }
  /// Divide operator
  inline virtual Coordinate3D operator/(const Coordinate3D& other) const {
    return Coordinate3D(x / other.x, y / other.y, z / other.z);
  }

  /// AddByScalar operator
  inline virtual Coordinate3D operator+(const T scalar) const {
    return Coordinate3D(x + scalar, y + scalar, z + scalar);
  }
  /// SubtractByScalar operator
  inline virtual Coordinate3D operator-(const T scalar) const {
    return Coordinate3D(x - scalar, y - scalar, z - scalar);
  }
  /// MultiplyByScalar operator
  inline virtual Coordinate3D operator*(const T scalar) const {
    return Coordinate3D(x * scalar, y * scalar, z * scalar);
  }
  /// DivideByScalar operator
  inline virtual Coordinate3D operator/(const T scalar) const {
    return Coordinate3D(x / scalar, y / scalar, z / scalar);
  }

  /// Cross product
  inline Coordinate3D<T> cross(const Coordinate3D<T> &other) const {
    return Coordinate3D((y * other.z) - (other.y * z), 
                        (z * other.x) - (other.z * x), 
                        (x * other.y) - (other.x * y));
  }
  /// Dot product
  inline double dot(const Coordinate3D<T> &other) const {
    return (x * other.x) + (y * other.y) + (z * other.z);
  }

  // Cartesian3d methods
  inline T magnitudeSquared(void) const {
    return (x * x) + (y * y) + (z * z);
  }
  inline T magnitude(void) const {
    return std::sqrt(magnitudeSquared());
  }
  inline static Coordinate3D<T> add(const Coordinate3D<T> &p1, const Coordinate3D<T> &p2) {
    return p1 + p2;
  }
  inline static Coordinate3D<T> subtract(const Coordinate3D<T> &p1, const Coordinate3D<T> &p2) {
    return p1 - p2;
  }
  inline static T distanceSquared(const Coordinate3D<T> &p1, const Coordinate3D<T> &p2) {
    T xdiff = p1.x - p2.x;
    T ydiff = p1.y - p2.y;
    T zdiff = p1.z - p2.z;
    return (xdiff * xdiff) + (ydiff * ydiff) + (zdiff * zdiff);
  }
  inline static T distance(const Coordinate3D<T> &p1, const Coordinate3D<T> &p2) {
    return std::sqrt(distanceSquared(p1, p2));
  }
  inline Coordinate3D<T> normalize(void) const {
    T mgn = magnitude();
    return Coordinate3D(x / mgn, y / mgn, z / mgn);
  }
};

#endif /* COORDINATE3D_HPP */
