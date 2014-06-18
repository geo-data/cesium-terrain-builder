#include "TileIterator.hpp"
#include "GDALTiler.hpp"

/**
 * @file TileIterator.cpp
 * @brief This defines the `TileIterator` class
 */

using namespace terrain;

terrain::TileIterator::TileIterator(const GDALTiler &tiler) :
  tiler(tiler)
{
  // Point the iterator to the lower left tile at the maximum zoom level
  i_zoom zoom = tiler.maxZoomLevel();
  bounds = tiler.tileBoundsForZoom(zoom);
  coord = TileCoordinate(zoom, bounds.getLowerLeft()); // the initial tile coordinate
}

TileIterator &
terrain::TileIterator::operator++() {
  // don't increment if exhausted
  if (exhausted())
    return *this;

  /* The statements in this function are the equivalent of the following
     `for` loops but broken down for use in the iterator:

     for (i_zoom zoom = maxZoom; zoom >= 0; zoom--) {
       tiler.lowerLeftTile(zoom, tminx, bounds.getMinY());
       tiler.upperRightTile(zoom, bounds.getMaxX(), bounds.getMaxY());

       for (int tx = tminx; tx <= bounds.getMaxX(); tx++) {
         for (int ty = bounds.getMinY(); ty <= bounds.getMaxY(); ty++) {
           TerrainTile *terrainTile = tiler.createTerrainTile(zoom, tx, ty);
         }
       }
     }

     Starting off in the lower left corner at the maximum zoom level iterate
     over the Y tiles (columns) first from left to right; if columns are
     exhausted then reset Y to the first column and increment the X to iterate
     over the next row (from bottom to top). If the rows are exhausted then we
     have iterated over that zoom level: decrease the zoom level and repeat the
     process for the new zoom level.  Do this until zoom level 0 is reached.
  */

  if (++(coord.y) > bounds.getMaxY()) {
    if (++(coord.x) > bounds.getMaxX()) {
      if (coord.zoom > 0) {
        (coord.zoom)--;

        bounds = tiler.tileBoundsForZoom(coord.zoom);
        coord.setPoint(bounds.getLowerLeft());
      }
    } else {
      coord.y = bounds.getMinY();
    }
  }

  return *this;
}

TileIterator
terrain::TileIterator::operator++(int) {
  TileIterator result(*this);   // make a copy for returning
  ++(*this);                    // use the prefix version to do the work
  return result;                // return the copy (the old) value.
}

bool
terrain::TileIterator::operator==(const TileIterator &other) const {
  return coord == other.coord
    && tiler.dataset() == other.tiler.dataset();
}

/**
 * @details use the tiler to create a terrain tile on the fly for the tile
 * coordinate currently pointed to by the iterator.
 */
TerrainTile
terrain::TileIterator::operator*() const {
  return tiler.createTerrainTile(coord);
}

bool
terrain::TileIterator::exhausted() const {
  return coord.zoom == 0 && coord.x > bounds.getMaxX() && coord.y > bounds.getMaxY();
}
