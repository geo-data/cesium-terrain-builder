#ifndef TERRAIN_HPP
#define TERRAIN_HPP

/**
 * @file terrain.hpp
 * @brief All required definitions for working with `libterrain`
 *
 * @mainpage Cesium Terrain Builder Library (libterrain)
 *
 * `libterrain` is a C++ library used to create terrain tiles for use in the
 * [Cesium JavaScript library](http://cesiumjs.org).  Terrain tiles are created
 * according to the [heightmap-1.0 terrain
 * format](http://cesiumjs.org/data-and-assets/terrain/formats/heightmap-1.0.html).
 * The library does not provide a way of serving up or storing the resulting
 * tiles: this is application specific.  Its aim is simply to take a
 * [GDAL](http://www.gdal.org) compatible raster representing a Digital Terrain
 * Model (DTM) and convert this to terrain tiles.  See the tools provided with
 * the library (e.g. `terrain-build`) for an example on how the the library is
 * used to achieve this.
 *
 * To use the library include `terrain.hpp` e.g.
 *
 * \code
 * #include <iostream>
 * #include "terrain.hpp"
 *
 * using namespace std;
 *
 * int main() {
 *
 *   cout << "Using libterrain version "
 *        << terrain::version.major << "."
 *        << terrain::version.minor << "."
 *        << terrain::version.patch << endl;
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

#include "terrain/config.hpp"
#include "terrain/types.hpp"
#include "terrain/TerrainException.hpp"
#include "terrain/GlobalGeodetic.hpp"
#include "terrain/TileCoordinate.hpp"
#include "terrain/TerrainTile.hpp"
#include "terrain/TileIterator.hpp"
#include "terrain/GDALTiler.hpp"

#endif /* TERRAIN_HPP */
