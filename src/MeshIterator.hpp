#ifndef MESHITERATOR_HPP
#define MESHITERATOR_HPP

/*******************************************************************************
 * Copyright 2018 GeoData <geodata@soton.ac.uk>
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
 * @file MeshIterator.hpp
 * @brief This declares the `MeshIterator` class
 * @author Alvaro Huarte <ahuarte47@yahoo.es>
 */

#include "MeshTiler.hpp"
#include "GridIterator.hpp"

namespace ctb {
  class MeshIterator;
}

/**
 * @brief This forward iterates over all `MeshTile`s in a `MeshTiler`
 *
 * Instances of this class take a `MeshTiler` in the constructor and are used
 * to forward iterate over all tiles in the tiler, returning a `MeshTile *`
 * when dereferenced.  It is the caller's responsibility to call `delete` on the
 * tile.
 */
class ctb::MeshIterator :
  public GridIterator
{
public:

  /// Instantiate an iterator with a tiler
  MeshIterator(const MeshTiler &tiler) :
    MeshIterator(tiler, tiler.maxZoomLevel(), 0)
  {}

  MeshIterator(const MeshTiler &tiler, i_zoom startZoom, i_zoom endZoom = 0) :
    GridIterator(tiler.grid(), tiler.bounds(), startZoom, endZoom),
    tiler(tiler)
  {}

  /// Override the dereference operator to return a Tile
  virtual MeshTile *
  operator*() const {
    return tiler.createMesh(tiler.dataset(), *(GridIterator::operator*()));
  }

  virtual MeshTile *
  operator*(ctb::GDALDatasetReader *reader) const {
    return tiler.createMesh(tiler.dataset(), *(GridIterator::operator*()), reader);
  }

protected:

  const MeshTiler &tiler;              ///< The tiler we are iterating over
};

#endif /* MESHITERATOR_HPP */
