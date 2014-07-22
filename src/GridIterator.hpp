#ifndef GRIDITERATOR_HPP
#define GRIDITERATOR_HPP

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
 * @file GridIterator.hpp
 * @brief This declares and defines the `GridIterator` class
 */

#include <iterator>

#include "TileCoordinate.hpp"
#include "Grid.hpp"

namespace ctb {
  class GridIterator;
}

/**
 * @brief A `GridIterator` forward iterates over tiles in a `Grid`
 *
 * Instances of this class take a `Grid` (or derived class) in the constructor
 * and are used to forward iterate over all tiles contained in the grid,
 * starting from a specified maximum zoom level and moving up to a specified
 * minimum zoom level e.g.
 *
 * \code
 *    for(GridIterator iter(tiler); !iter.exhausted(); ++iter) {
 *      TileCoordinate tile = *iter;
 *      // do stuff with tile coordinate
 *    }
 * \endcode
 *
 * By default the iterator iterates over the full extent represented by the
 * grid, but alternative extents can be passed in to the constructor, acting as
 * a spatial filter.
 */
class ctb::GridIterator :
  public std::iterator<std::input_iterator_tag, TileCoordinate *>
{
public:

  /// Instantiate an iterator with a grid
  GridIterator(const Grid &grid, i_zoom startZoom, i_zoom endZoom = 0) :
    grid(grid),
    startZoom(startZoom),
    endZoom(endZoom),
    gridExtent(grid.getExtent()),
    bounds(grid.getTileExtent(startZoom)),
    currentTile(TileCoordinate(startZoom, bounds.getLowerLeft())) // the initial tile coordinate
  {
    if (startZoom < endZoom)
      throw CTBException("Iterating from a starting zoom level that is less than the end zoom level");
  }

  /// Instantiate an iterator with a grid and separate bounds
  GridIterator(const Grid &grid, const CRSBounds &extent, i_zoom startZoom, i_zoom endZoom = 0) :
    grid(grid),
    startZoom(startZoom),
    endZoom(endZoom),
    gridExtent(extent)
  {
    if (startZoom < endZoom)
      throw CTBException("Iterating from a starting zoom level that is less than the end zoom level");

    currentTile.zoom = startZoom;
    setTileBounds();
  }

  /// Override the ++prefix operator
  GridIterator &
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

    if (++(currentTile.y) > bounds.getMaxY()) {
      if (++(currentTile.x) > bounds.getMaxX()) {
        if (currentTile.zoom > endZoom) {
          (currentTile.zoom)--;

          setTileBounds();
        }
      } else {
        currentTile.y = bounds.getMinY();
      }
    }

    return *this;
  }

  /// Override the postfix++ operator
  GridIterator
  operator++(int) {
    GridIterator result(*this);   // make a copy for returning
    ++(*this);                    // use the prefix version to do the work
    return result;                // return the copy (the old) value.
  }

  /// Override the equality operator
  bool
  operator==(const GridIterator &other) const {
    return currentTile == other.currentTile
      && startZoom == other.startZoom
      && endZoom == other.endZoom
      && bounds == other.bounds
      && gridExtent == other.gridExtent
      && grid == other.grid;
  }

  /// Override the inequality operator
  bool
  operator!=(const GridIterator &other) const {
    return !operator==(other);
  }

  /// Dereference the iterator to retrieve a `TileCoordinate`
  virtual const TileCoordinate *
  operator*() const {
    return &currentTile;
  }

  /// Return `true` if the iterator is at the end
  bool
  exhausted() const {
    return currentTile.zoom == endZoom && currentTile.x > bounds.getMaxX() && currentTile.y > bounds.getMaxY();
  }

  /// Reset the iterator to a certain point
  void
  reset(i_zoom start, i_zoom end) {
    if (start < end)
      throw CTBException("Starting zoom level cannot be less than the end zoom level");

    currentTile.zoom = startZoom = start;
    endZoom = end;

    setTileBounds();
  }

  /// Get the total number of elements in the iterator
  i_tile
  getSize() const {
    i_tile size = 0;
    for (i_zoom zoom = endZoom; zoom <= startZoom; ++zoom) {
      TileCoordinate ll = grid.crsToTile(gridExtent.getLowerLeft(), zoom),
        ur = grid.crsToTile(gridExtent.getUpperRight(), zoom);

      TileBounds zoomBound(ll, ur);
      size += (zoomBound.getWidth() + 1) * (zoomBound.getHeight() + 1);
    }

    return size;
  }

  /// Get the grid we are iterating over
  const Grid &
  getGrid() const {
    return grid;
  }

protected:

  /// Set the tile bounds of the grid for the current zoom level
  void
  setTileBounds() {
    TileCoordinate ll = grid.crsToTile(gridExtent.getLowerLeft(), currentTile.zoom),
      ur = grid.crsToTile(gridExtent.getUpperRight(), currentTile.zoom);

    // set the bounds
    bounds = TileBounds(ll, ur);

    // set the current tile
    currentTile.setPoint(ll);
  }

  const Grid &grid;      ///< The grid we are iterating over
  i_zoom startZoom;      ///< The starting zoom level
  i_zoom endZoom;        ///< The final zoom level
  CRSBounds gridExtent;  ///< The extent of the underlying grid to iterate over
  TileBounds bounds;     ///< The extent of the currently iterated zoom level
  TileCoordinate currentTile; ///< The identity of the current tile being pointed to
};

#endif /* GRIDITERATOR_HPP */
