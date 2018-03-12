#ifndef CTBTYPES_HPP
#define CTBTYPES_HPP

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
 * @file types.hpp
 * @brief This declares basic types used by libctb
 */

#include <cstdint>              // uint16_t

#include "Bounds.hpp"
#include "Coordinate3D.hpp"

/// All terrain related data types reside in this namespace
namespace ctb {

  // Simple types
  typedef unsigned int i_pixel;       ///< A pixel value
  typedef unsigned int i_tile;        ///< A tile coordinate
  typedef unsigned short int i_zoom;  ///< A zoom level
  typedef uint16_t i_terrain_height;  ///< A terrain tile height

  // Complex types
  typedef Bounds<i_tile> TileBounds;      ///< Tile extents in tile coordinates
  typedef Coordinate<i_pixel> PixelPoint; ///< The location of a pixel
  typedef Coordinate<double> CRSPoint;    ///< A Coordinate Reference System coordinate
  typedef Coordinate3D<double> CRSVertex; ///< A 3D-Vertex of a mesh or tile in CRS coordinates
  typedef Bounds<double> CRSBounds;       ///< Extents in CRS coordinates
  typedef Coordinate<i_tile> TilePoint;   ///< The location of a tile

}

#endif /* CTBTYPES_HPP */
