#include "TileIterator.hpp"
#include "GDALTiler.hpp"

TileIterator::TileIterator(const GDALTiler &tiler) :
  tiler(tiler)
{
  unsigned short int zoom = tiler.maxZoomLevel();
  TileCoordinate lowerLeft = tiler.lowerLeftTile(zoom);
  TileCoordinate upperRight = tiler.upperRightTile(zoom);
  tminx = lowerLeft.x;
  tminy = lowerLeft.y;
  tmaxx = upperRight.x;
  tmaxy = upperRight.y;

  coord = TileCoordinate(zoom, tminx, tminy); // the initial tile coordinate
}

TileIterator &
TileIterator::operator++() {                // prefix ++
  if (exhausted())
    return *this;             // don't increment

  /* The statements in this function are the equivalent of the following
     `for` loops but broken down for use in the iterator:

     - for (short int zoom = maxZoom; zoom >= 0; zoom--) {
     -   tiler.lowerLeftTile(zoom, tminx, tminy);
     -   tiler.upperRightTile(zoom, tmaxx, tmaxy);
     -
     -   for (int tx = tminx; tx <= tmaxx; tx++) {
     -     for (int ty = tminy; ty <= tmaxy; ty++) {
     -       TerrainTile *terrainTile = tiler.createTerrainTile(zoom, tx, ty);
     -     }
     -   }
     - }
  */

  if (++(coord.y) > tmaxy) {
    if (++(coord.x) > tmaxx) {
      if (coord.zoom > 0) {
        (coord.zoom)--;

        TileCoordinate lowerLeft = tiler.lowerLeftTile(coord.zoom);
        TileCoordinate upperRight = tiler.upperRightTile(coord.zoom);
        tminx = lowerLeft.x;
        tminy = lowerLeft.y;
        tmaxx = upperRight.x;
        tmaxy = upperRight.y;

        coord.x = tminx;
        coord.y = tminy;
      }
    } else {
      coord.y = tminy;
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
  return coord.zoom == 0 && coord.x > tmaxx && coord.y > tmaxy;
}
