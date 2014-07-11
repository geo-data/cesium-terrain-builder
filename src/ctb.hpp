#ifndef CTB_HPP
#define CTB_HPP

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
 * @file ctb.hpp
 * @brief All required definitions for working with `libctb`
 *
 * @mainpage Cesium Terrain Builder Library (libctb)
 *
 * `libctb` is a C++ library used to create terrain tiles for use in the
 * [Cesium JavaScript library](http://cesiumjs.org).  Terrain tiles are created
 * according to the [heightmap-1.0 terrain
 * format](http://cesiumjs.org/data-and-assets/terrain/formats/heightmap-1.0.html).
 * The library does not provide a way of serving up or storing the resulting
 * tiles: this is application specific.  Its aim is simply to take a
 * [GDAL](http://www.gdal.org) compatible raster representing a Digital Terrain
 * Model (DTM) and convert this to terrain tiles.  See the tools provided with
 * the library (e.g. `ctb-tile`) for an example on how the the library is used
 * to achieve this.
 *
 * To use the library include `ctb.hpp` e.g.
 *
 * \code
 * #include <iostream>
 * #include "ctb.hpp"
 *
 * using namespace std;
 *
 * int main() {
 *
 *   cout << "Using libctb version "
 *        << ctb::version.major << "."
 *        << ctb::version.minor << "."
 *        << ctb::version.patch << endl;
 *
 *   return 0;
 * }
 * \endcode
 *
 * Assuming the library is installed on the system and you are using the `g++`
 * compiler, the above can be compiled using:
 *
 * \code{.sh}
 * g++ -lterrain -o test test.cpp
 * \endcode
 *
 * See the `README.md` file distributed with the source code for further
 * details.
 */

#include "ctb/Bounds.hpp"
#include "ctb/Coordinate.hpp"
#include "ctb/CRSBoundsIterator.hpp"
#include "ctb/GDALTiler.hpp"
#include "ctb/GlobalGeodetic.hpp"
#include "ctb/GlobalMercator.hpp"
#include "ctb/Grid.hpp"
#include "ctb/GridIterator.hpp"
#include "ctb/RasterIterator.hpp"
#include "ctb/CTBException.hpp"
#include "ctb/TerrainIterator.hpp"
#include "ctb/TerrainTile.hpp"
#include "ctb/TerrainTiler.hpp"
#include "ctb/TileCoordinate.hpp"
#include "ctb/TileCoordinateIterator.hpp"
#include "ctb/TilerIterator.hpp"
#include "ctb/types.hpp"

#endif /* CTB_HPP */
