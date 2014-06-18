#ifndef TERRAINTYPES_HPP
#define TERRAINTYPES_HPP

/**
 * @file types.hpp
 * @brief This declares basic types used by libterrain
 */

#include "Bounds.hpp"

/// All terrain related data types reside in this namespace
namespace terrain {

  // Simple types
  typedef unsigned int i_pixel;       ///< A pixel value
  typedef unsigned int i_tile;        ///< A tile coordinate
  typedef unsigned short int i_zoom;  ///< A zoom level
  typedef short int i_terrain_height; ///< A terrain tile height

  // Complex types
  typedef Bounds<i_tile> TileBounds;      ///< Tile extents in tile coordinates
  typedef Coordinate<i_pixel> PixelPoint; ///< The location of a pixel
  typedef Coordinate<double> LatLon;      ///< A latitude and longitude
  typedef Bounds<double> LatLonBounds;    ///< Extents in latitude and longitude
  typedef Coordinate<i_tile> TilePoint;   ///< The location of a tile

}

#endif /* TERRAINTYPES_HPP */
