#ifndef MESHTILER_HPP
#define MESHTILER_HPP

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
 * @file MeshTiler.hpp
 * @brief This declares the `MeshTiler` class
 * @author Alvaro Huarte <ahuarte47@yahoo.es>
 */

#include "MeshTile.hpp"
#include "TerrainTiler.hpp"

namespace ctb {
  class MeshTiler;
}

/**
 * @brief Create `MeshTile`s from a GDAL Dataset
 *
 * This class derives from `GDALTiler` and `TerrainTiler` enabling `MeshTile`s 
 * to be created for a specific `TileCoordinate`.
 */
class CTB_DLL ctb::MeshTiler :
  public TerrainTiler
{
public:

  /// Instantiate a tiler with all required arguments
  MeshTiler(GDALDataset *poDataset, const Grid &grid, const TilerOptions &options, double meshQualityFactor = 1.0):
    TerrainTiler(poDataset, grid, options),
    mMeshQualityFactor(meshQualityFactor) {}

  /// Instantiate a tiler with an empty GDAL dataset
  MeshTiler(double meshQualityFactor = 1.0):
    TerrainTiler(),
    mMeshQualityFactor(meshQualityFactor) {}

  /// Instantiate a tiler with a dataset and grid but no options
  MeshTiler(GDALDataset *poDataset, const Grid &grid, double meshQualityFactor = 1.0):
    TerrainTiler(poDataset, grid, TilerOptions()),
    mMeshQualityFactor(meshQualityFactor) {}

  /// Overload the assignment operator
  MeshTiler &
  operator=(const MeshTiler &other);

  /// Create a mesh from a tile coordinate
  MeshTile *
  createMesh(GDALDataset *dataset, const TileCoordinate &coord) const;

  /// Create a mesh from a tile coordinate
  MeshTile *
  createMesh(GDALDataset *dataset, const TileCoordinate &coord, GDALDatasetReader *reader) const;

protected:

  // Specifies the factor of the quality to convert terrain heightmaps to meshes.
  double mMeshQualityFactor;

  // Determines an appropriate geometric error estimate when the geometry comes from a heightmap.
  static double getEstimatedLevelZeroGeometricErrorForAHeightmap(
    double maximumRadius, 
    double heightmapTerrainQuality, 
    int tileWidth, 
    int numberOfTilesAtLevelZero);

  /// Assigns settings of Tile just to use.
  void prepareSettingsOfTile(MeshTile *tile, GDALDataset *dataset, const TileCoordinate &coord, float *rasterHeights, ctb::i_tile tileSizeX, ctb::i_tile tileSizeY) const;
};

#endif /* MESHTILER_HPP */
