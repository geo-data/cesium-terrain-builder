#ifndef BBSPHERE_HPP
#define BBSPHERE_HPP

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

/**
 * @file BoundingSphere.hpp
 * @brief This declares and defines the `BoundingSphere` class
 * @author Alvaro Huarte <ahuarte47@yahoo.es>
 */

#include <vector>
#include <limits>

#include "Coordinate3D.hpp"
#include "types.hpp"

namespace ctb {
  template <class T> class BoundingSphere;
  template <class T> class BoundingBox;
}

/// A spherical bounding region which is defined by a center point and a radius
template <class T>
class ctb::BoundingSphere {
public:
  Coordinate3D<T> center; ///< The center of the BoundingSphere
  double radius; ///< The radius of the BoundingSphere

  /// Create an empty BoundingSphere
  BoundingSphere() {
  }
  /// Create a BoundingSphere from the specified point stream
  BoundingSphere(const std::vector<Coordinate3D<T>> &points) {
    fromPoints(points);
  }

  /// Calculate the center and radius from the specified point stream
  /// Based on Ritter's algorithm
  void fromPoints(const std::vector<Coordinate3D<T>> &points) {
    const T MAX =  std::numeric_limits<T>::infinity();
    const T MIN = -std::numeric_limits<T>::infinity();

    Coordinate3D<T> minPointX(MAX, MAX, MAX);
    Coordinate3D<T> minPointY(MAX, MAX, MAX);
    Coordinate3D<T> minPointZ(MAX, MAX, MAX);
    Coordinate3D<T> maxPointX(MIN, MIN, MIN);
    Coordinate3D<T> maxPointY(MIN, MIN, MIN);
    Coordinate3D<T> maxPointZ(MIN, MIN, MIN);

    // Store the points containing the smallest and largest component
    // Used for the naive approach
    for (int i = 0, icount = points.size(); i < icount; i++) {
      const Coordinate3D<T> &point = points[i];

      if (point.x < minPointX.x) minPointX = point;
      if (point.y < minPointY.y) minPointY = point;
      if (point.z < minPointZ.z) minPointZ = point;
      if (point.x > maxPointX.x) maxPointX = point;
      if (point.y > maxPointY.y) maxPointY = point;
      if (point.z > maxPointZ.z) maxPointZ = point;
    }

    // Squared distance between each component min and max
    T xSpan = (maxPointX - minPointX).magnitudeSquared();
    T ySpan = (maxPointY - minPointY).magnitudeSquared();
    T zSpan = (maxPointZ - minPointZ).magnitudeSquared();

    Coordinate3D<T> diameter1 = minPointX;
    Coordinate3D<T> diameter2 = maxPointX;
    T maxSpan = xSpan;
    if (ySpan > maxSpan) {
      diameter1 = minPointY;
      diameter2 = maxPointY;
      maxSpan = ySpan;
    }
    if (zSpan > maxSpan) {
      diameter1 = minPointZ;
      diameter2 = maxPointZ;
      maxSpan = zSpan;
    }

    Coordinate3D<T> ritterCenter = Coordinate3D<T>(
      (diameter1.x + diameter2.x) * 0.5,
      (diameter1.y + diameter2.y) * 0.5,
      (diameter1.z + diameter2.z) * 0.5
    );
    T radiusSquared = (diameter2 - ritterCenter).magnitudeSquared();
    T ritterRadius = std::sqrt(radiusSquared);

    // Initial center and radius (naive) get min and max box
    Coordinate3D<T> minBoxPt(minPointX.x, minPointY.y, minPointZ.z);
    Coordinate3D<T> maxBoxPt(maxPointX.x, maxPointY.y, maxPointZ.z);
    Coordinate3D<T> naiveCenter = (minBoxPt + maxBoxPt) * 0.5;
    T naiveRadius = 0;

    for (int i = 0, icount = points.size(); i < icount; i++) {
      const Coordinate3D<T> &point = points[i];

      // Find the furthest point from the naive center to calculate the naive radius.
      T r = (point - naiveCenter).magnitude();
      if (r > naiveRadius) naiveRadius = r;

      // Make adjustments to the Ritter Sphere to include all points.
      T oldCenterToPointSquared = (point - ritterCenter).magnitudeSquared();

      if (oldCenterToPointSquared > radiusSquared) {
        T oldCenterToPoint = std::sqrt(oldCenterToPointSquared);
        ritterRadius = (ritterRadius + oldCenterToPoint) * 0.5;

        // Calculate center of new Ritter sphere
        T oldToNew = oldCenterToPoint - ritterRadius;
        ritterCenter.x = (ritterRadius * ritterCenter.x + oldToNew * point.x) / oldCenterToPoint;
        ritterCenter.y = (ritterRadius * ritterCenter.y + oldToNew * point.y) / oldCenterToPoint;
        ritterCenter.z = (ritterRadius * ritterCenter.z + oldToNew * point.z) / oldCenterToPoint;
      }
    }

    // Keep the naive sphere if smaller
    if (naiveRadius < ritterRadius) {
      center = ritterCenter;
      radius = ritterRadius;
    }
    else {
      center = naiveCenter;
      radius = naiveRadius;
    }
  }
};

/// A bounding box which is defined by a pair of minimum and maximum coordinates
template <class T>
class ctb::BoundingBox {
public:
  Coordinate3D<T> min; ///< The min coordinate of the BoundingBox
  Coordinate3D<T> max; ///< The max coordinate of the BoundingBox

  /// Create an empty BoundingBox
  BoundingBox() {
  }
  /// Create a BoundingBox from the specified point stream
  BoundingBox(const std::vector<Coordinate3D<T>> &points) {
    fromPoints(points);
  }

  /// Calculate the BBOX from the specified point stream
  void fromPoints(const std::vector<Coordinate3D<T>> &points) {
    const T MAX =  std::numeric_limits<T>::infinity();
    const T MIN = -std::numeric_limits<T>::infinity();
    min.x = MAX;
    min.y = MAX;
    min.z = MAX;
    max.x = MIN;
    max.y = MIN;
    max.z = MIN;

    for (int i = 0, icount = points.size(); i < icount; i++) {
      const Coordinate3D<T> &point = points[i];

      if (point.x < min.x) min.x = point.x;
      if (point.y < min.y) min.y = point.y;
      if (point.z < min.z) min.z = point.z;
      if (point.x > max.x) max.x = point.x;
      if (point.y > max.y) max.y = point.y;
      if (point.z > max.z) max.z = point.z;
    }
  }
};

#endif /* BBSPHERE_HPP */
