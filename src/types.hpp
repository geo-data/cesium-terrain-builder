#include "Bounds.hpp"

// Simple types
typedef unsigned int i_pixel;   // a pixel value
typedef unsigned int i_tile;    // a tile coordinate
typedef unsigned short int i_zoom; // a zoom level
typedef short int i_terrain_height; // a terrain tile height

// Complex types
typedef Bounds<i_tile> TileBounds;
typedef Coordinate<i_pixel> PixelPoint;
typedef Coordinate<double> LatLon;
typedef Bounds<double> LatLonBounds;
typedef Coordinate<i_tile> TilePoint;
