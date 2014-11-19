#ifndef GLOBALMERCATOR_HPP
#define GLOBALMERCATOR_HPP

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
 * @file GlobalMercator.hpp
 * @brief This defines the `GlobalMercator` class
 */

#include "config.hpp"           // for CTB_DLL
#include "Grid.hpp"

namespace ctb {
  class GlobalMercator;
}

/**
 * @brief An implementation of the TMS Global Mercator Profile
 *
 * This class models the [Tile Mapping Service Global Mercator
 * Profile](http://wiki.osgeo.org/wiki/Tile_Map_Service_Specification#global-mercator).
 */
class CTB_DLL ctb::GlobalMercator :
  public Grid {
public:

  GlobalMercator(i_tile tileSize = 256):
    Grid(tileSize,
         CRSBounds(-cOriginShift, -cOriginShift, cOriginShift, cOriginShift),
         cSRS)
  {}

protected:

  /// The semi major axis of the WGS84 ellipsoid (the radius of the earth in
  /// meters)
  static const unsigned int cSemiMajorAxis;

  /// The circumference of the earth in meters
  static const double cEarthCircumference;

  /// The coordinate origin (the middle of the grid extent)
  static const double cOriginShift;

  /// The EPSG:3785 spatial reference system
  static const OGRSpatialReference cSRS;
};

#endif /* GLOBALMERCATOR_HPP */
