#include "TileIterator.hpp"
#include "GDALTiler.hpp"

TileIterator::TileIterator(const GDALTiler &tiler) :
  tiler(tiler)
{
  i_zoom zoom = tiler.maxZoomLevel();
  bounds = tiler.tileBoundsForZoom(zoom);
  coord = TileCoordinate(zoom, bounds.getLowerLeft()); // the initial tile coordinate
}

TileIterator &
TileIterator::operator++() {                // prefix ++
  if (exhausted())
    return *this;             // don't increment

  /* The statements in this function are the equivalent of the following
     `for` loops but broken down for use in the iterator:

     - for (i_zoom zoom = maxZoom; zoom >= 0; zoom--) {
     -   tiler.lowerLeftTile(zoom, tminx, bounds.getMinY());
     -   tiler.upperRightTile(zoom, bounds.getMaxX(), bounds.getMaxY());
     -
     -   for (int tx = tminx; tx <= bounds.getMaxX(); tx++) {
     -     for (int ty = bounds.getMinY(); ty <= bounds.getMaxY(); ty++) {
     -       TerrainTile *terrainTile = tiler.createTerrainTile(zoom, tx, ty);
     -     }
     -   }
     - }
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
TileIterator::operator++(int) {             // postfix ++
  TileIterator result(*this); // make a copy for result
  ++(*this);                  // use the prefix version to do the work
  return result;              // return the copy (the old) value.
}

bool
TileIterator::operator==(const TileIterator &other) const {
  return coord == other.coord
    && tiler.dataset() == other.tiler.dataset();
}

TerrainTile
TileIterator::operator*() const {
  return tiler.createTerrainTile(coord);
}

bool
TileIterator::exhausted() const {
  return coord.zoom == 0 && coord.x > bounds.getMaxX() && coord.y > bounds.getMaxY();
}
