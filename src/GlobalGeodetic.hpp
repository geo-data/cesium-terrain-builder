#ifndef GLOBALGEODETIC_HPP
#define GLOBALGEODETIC_HPP

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
 * @file GlobalGeodetic.hpp
 * @brief This defines and declares the `GlobalGeodetic` class
 */

#include "config.hpp"
#include "Grid.hpp"

namespace ctb {
  class GlobalGeodetic;
}

/**
 * @brief An implementation of the TMS Global Geodetic Profile
 *
 * This class models the [Tile Mapping Service Global Geodetic
 * Profile](http://wiki.osgeo.org/wiki/Tile_Map_Service_Specification#global-geodetic).
 */
class CTB_DLL ctb::GlobalGeodetic :
  public Grid {
public:

  /// Initialise the profile with a specific tile size
  GlobalGeodetic(i_tile tileSize = TILE_SIZE, bool tmsCompatible = true):
    Grid(tileSize,
         CRSBounds(-180, -90, 180, 90),
         cSRS,
         (tmsCompatible) ? 2 : 1)
  {}

protected:

  /// The EPSG:4326 spatial reference system
  static const OGRSpatialReference cSRS;
};

#endif /* GLOBALGEODETIC_HPP */
