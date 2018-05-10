#ifndef TERRAINTILER_HPP
#define TERRAINTILER_HPP

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
 * @file TerrainTiler.hpp
 * @brief This declares the `TerrainTiler` class
 */

#include "TerrainTile.hpp"
#include "GDALTiler.hpp"

namespace ctb {
  class TerrainTiler;
}

/**
 * @brief Create `TerrainTile`s from a GDAL Dataset
 *
 * This class derives from `GDALTiler` and adds the
 * `GDALTiler::createTerrainTile` method enabling `TerrainTile`s to be created
 * for a specific `TileCoordinate`.
 */
class CTB_DLL ctb::TerrainTiler :
  public GDALTiler
{
public:

  /// Instantiate a tiler with all required arguments
  TerrainTiler(GDALDataset *poDataset, const Grid &grid, const TilerOptions &options):
    GDALTiler(poDataset, grid, options) {}

  /// Instantiate a tiler with an empty GDAL dataset
  TerrainTiler():
    GDALTiler() {}

  /// Instantiate a tiler with a dataset and grid but no options
  TerrainTiler(GDALDataset *poDataset, const Grid &grid):
    TerrainTiler(poDataset, grid, TilerOptions()) {}

  /// Overload the assignment operator
  TerrainTiler &
  operator=(const TerrainTiler &other);

  /// Override to return a covariant data type
  TerrainTile *
  createTile(GDALDataset *dataset, const TileCoordinate &coord) const override;

  /// Create a tile from a tile coordinate
  TerrainTile *
  createTile(GDALDataset *dataset, const TileCoordinate &coord, GDALDatasetReader *reader) const;

protected:

  /// Create a `GDALTile` representing the required terrain tile data
  virtual GDALTile *
  createRasterTile(GDALDataset *dataset, const TileCoordinate &coord) const override;

  /**
   * @brief Get terrain bounds shifted to introduce a pixel overlap
   *
   * Given a `TileCoordinate`, this sets the resolution and returns latitude
   * and longitude bounds for a tile which include a pixel's worth of data
   * outside the actual tile bounds to both the east and the north.  This is
   * used to satisfy the terrain heightmap specification of terrain tiles
   * including a pixel's worth of data from surrounding tiles.
   *
   * @param coord The tile coordinate identifying the tile in question
   * @param resolution The resolution of the modified extent is set here
   */
  inline CRSBounds
  terrainTileBounds(const TileCoordinate &coord,
                    double &resolution) const {
    // The actual tile size accounting for a border
    i_tile lTileSize = mGrid.tileSize() - 1;
    CRSBounds tile = mGrid.tileBounds(coord); // the actual tile bounds

    // Get the resolution for the dataset without a border
    resolution = (tile.getMaxX() - tile.getMinX()) / lTileSize;

    // extend the easting by one pixel's worth
    tile.setMinX(tile.getMinX() - resolution);

    // extend the northing by one pixel's worth
    tile.setMaxY(tile.getMaxY() + resolution);

    return tile;
  }

  /// Assigns settings of Tile just to use.
  void prepareSettingsOfTile(TerrainTile *tile, const TileCoordinate &coord, float *rasterHeights, ctb::i_tile tileSizeX, ctb::i_tile tileSizeY) const;
};

#endif /* TERRAINTILER_HPP */
