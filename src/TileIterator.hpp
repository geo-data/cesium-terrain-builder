#ifndef TILEITERATOR_HPP
#define TILEITERATOR_HPP

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
 * @file TileIterator.hpp
 * @brief This declares and defines the `TileIterator` class
 */

#include <iterator>

#include "TileCoordinate.hpp"

namespace terrain {
  template <class T1, class T2> class TileIterator;
}

/**
 * @brief A `TileIterator` forward iterates over all tiles in a `GDALTiler`
 *
 * Instances of this class take a `GDALTiler` (or derived class) in the
 * constructor and are used to forward iterate over all tiles in the tiler,
 * starting from the maximum zoom level and moving up to level `0` e.g.
 *
 * \code
 *    for(TileIterator iter(tiler); !iter.exhausted(); ++iter) {
 *      Tile tile = *iter;
 *      // do stuff with tile
 *    }
 * \endcode
 *
 * The code above is pseudo code: this class is designed to be subclassed,
 * specifically having the `operator*` method overridden to return the contents
 * of a tile.
 */
template <class T1, class T2>
class terrain::TileIterator :
  public std::iterator<std::input_iterator_tag, T1>
{
public:

  /// Instantiate an iterator with a tiler
  TileIterator(const T2 &tiler) :
    tiler(tiler)
  {
    // Point the iterator to the lower left tile at the maximum zoom level
    i_zoom zoom = tiler.maxZoomLevel();
    bounds = tiler.tileBoundsForZoom(zoom);
    coord = TileCoordinate(zoom, bounds.getLowerLeft()); // the initial tile coordinate
  }

  /// Override the ++prefix operator
  TileIterator<T1, T2> &
  operator++() {
    // don't increment if exhausted
    if (exhausted())
      return *this;

    /*The statements in this function are the equivalent of the following `for`
       loops but broken down for use in the iterator:

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
       exhausted then reset Y to the first column and increment the X to
       iterate over the next row (from bottom to top). If the rows are
       exhausted then we have iterated over that zoom level: decrease the zoom
       level and repeat the process for the new zoom level.  Do this until zoom
       level 0 is reached.
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

  /// Override the postfix++ operator
  TileIterator<T1, T2>
  operator++(int) {
    TileIterator result(*this);   // make a copy for returning
    ++(*this);                    // use the prefix version to do the work
    return result;                // return the copy (the old) value.
  }

  /// Override the equality operator
  bool
  operator==(const TileIterator<T1, T2> &other) const {
    return coord == other.coord
      && tiler.dataset() == other.tiler.dataset();
  }

  /// Override the inequality operator
  bool
  operator!=(const TileIterator<T1, T2> &other) const {
    return !operator==(other);
  }

  /// Override the dereference operator to return a tile
  T1
  operator*() const;

  /// Return `true` if the iterator is at the end
  bool
  exhausted() const {
    return coord.zoom == 0 && coord.x > bounds.getMaxX() && coord.y > bounds.getMaxY();
  }

protected:

  TileCoordinate coord; ///< The identity of the current tile being pointed to
  const T2 &tiler;      ///< The tiler we are iterating over

private:

  TileBounds bounds;       ///< The extent of the currently iterated zoom level
};

#endif /* TILEITERATOR_HPP */
