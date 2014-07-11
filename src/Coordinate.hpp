#ifndef COORDINATE_HPP
#define COORDINATE_HPP

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
 * @file Coordinate.hpp
 * @brief This declares and defines the `Coordinate` class
 */

namespace ctb {
  template <class T> class Coordinate;
}

/// A representation of a point coordinate
template <class T>
class ctb::Coordinate {
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
