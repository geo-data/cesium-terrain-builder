#ifndef GDALTILE_HPP
#define GDALTILE_HPP

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
 * @file GDALTile.hpp
 * @brief This declares the `GDALTile` class
 */

#include "gdal_priv.h"

#include "config.hpp"           // for CTB_DLL
#include "Tile.hpp"

namespace ctb {
  class GDALTile;
  class GDALTiler;              // forward declaration
}

/**
 * A representation of a `Tile` with a GDAL datasource
 *
 * This is composed of a GDAL VRT datasource and optionally a GDAL image
 * transformer, along with a `TileCoordinate`.  The transformer handle is
 * necessary in cases where the VRT is warped using a linear approximation
 * (`GDALApproxTransform`). In this case there is the top level transformer (the
 * linear approximation) which wraps an image transformer.  The VRT owns any top
 * level transformer, but we are responsible for the wrapped image transformer.
 */
class CTB_DLL ctb::GDALTile :
  public Tile
{
public:
  /// Take ownership of a dataset and optional transformer
  GDALTile(GDALDataset *dataset, void *transformer):
    Tile(),
    dataset(dataset),
    transformer(transformer)
  {}

  ~GDALTile();

  GDALDataset *dataset;

  /// Detach the underlying GDAL dataset
  GDALDataset *detach();

protected:
  friend class GDALTiler;

  /// The image to image transformer
  void *transformer;
};

#endif /* GDALTILE_HPP */
