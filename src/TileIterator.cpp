#include "TileIterator.hpp"
#include "GDALTiler.hpp"

TileIterator::TileIterator(const GDALTiler &tiler) :
  tiler(tiler)
{
  zoom = tiler.maxZoomLevel();
  tiler.lowerLeftTile(zoom, tminx, tminy);
  tiler.upperRightTile(zoom, tmaxx, tmaxy);
  tx = tminx;
  ty = tminy;
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

  if (++ty > tmaxy) {
    if (++tx > tmaxx) {
      if (zoom > 0) {
        zoom--;

        tiler.lowerLeftTile(zoom, tminx, tminy);
        tiler.upperRightTile(zoom, tmaxx, tmaxy);

        tx = tminx;
        ty = tminy;
      }
    } else {
      ty = tminy;
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
TileIterator::operator==(const TileIterator &b) const {
  return zoom == b.zoom
    && tx == b.tx
    && ty == b.ty
    && tiler.dataset() == b.tiler.dataset();
}

const TerrainTile *
TileIterator::operator*() const {
  return tiler.createTerrainTile(zoom, tx, ty);
}

bool
TileIterator::exhausted() const {
  return zoom == 0 && tx > tmaxx && ty > tmaxy;
}
