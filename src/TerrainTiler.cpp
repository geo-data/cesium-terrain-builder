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
 * @file TerrainTiler.cpp
 * @brief This defines the `TerrainTiler` class
 */

#include "TerrainException.hpp"
#include "TerrainTiler.hpp"

using namespace terrain;

TerrainTile
terrain::TerrainTiler::createTerrainTile(const TileCoordinate &coord) const {
  TerrainTile terrainTile(coord); // a terrain tile represented by the tile coordinate
  GDALDataset *rasterTile = (GDALDataset *) createRasterTile(coord); // the raster associated with this tile coordinate
  GDALRasterBand *heightsBand = rasterTile->GetRasterBand(1);

  // Copy the raster data into an array
  float rasterHeights[TerrainTile::TILE_CELL_SIZE];
  if (heightsBand->RasterIO(GF_Read, 0, 0, TILE_SIZE, TILE_SIZE,
                            (void *) rasterHeights, TILE_SIZE, TILE_SIZE, GDT_Float32,
                            0, 0) != CE_None) {
    GDALClose(rasterTile);
    throw TerrainException("Could not read heights from raster");
  }

  // Copy the raster data into the terrain tile heights
  // TODO: try doing this using a VRT derived band:
  // (http://www.gdal.org/gdal_vrttut.html)
  for (unsigned short int i = 0; i < TerrainTile::TILE_CELL_SIZE; i++) {
    terrainTile.mHeights[i] = (i_terrain_height) ((rasterHeights[i] + 1000) * 5);
  }

  GDALClose(rasterTile);

  // If we are not at the maximum zoom level we need to set child flags on the
  // tile where child tiles overlap the dataset bounds.
  if (coord.zoom != maxZoomLevel()) {
    CRSBounds tileBounds = mGrid.tileBounds(coord);

    if (! (bounds().overlaps(tileBounds))) {
      terrainTile.setAllChildren(false);
    } else {
      if (bounds().overlaps(tileBounds.getSW())) {
        terrainTile.setChildSW();
      }
      if (bounds().overlaps(tileBounds.getNW())) {
        terrainTile.setChildNW();
      }
      if (bounds().overlaps(tileBounds.getNE())) {
        terrainTile.setChildNE();
      }
      if (bounds().overlaps(tileBounds.getSE())) {
        terrainTile.setChildSE();
      }
    }
  }

  return terrainTile;
}

/**
 * @details This method is the heart of the tiler.  A `TileCoordinate` is used
 * to obtain the geospatial extent associated with that tile as related to the
 * underlying GDAL dataset. This mapping may require a reprojection if the
 * underlying dataset is not in the EPSG:4326 projection.  This information is
 * then encapsulated as a GDAL virtual raster (VRT) dataset and returned to the
 * caller.
 */
GDALDatasetH
terrain::TerrainTiler::createRasterTile(const TileCoordinate &coord) const {
  // Get the bounds and resolution for a tile coordinate which represents the
  // data overlap requested by the terrain specification.
  double resolution;
  CRSBounds tileBounds = terrainTileBounds(coord, resolution);

  // Convert the tile bounds into a geo transform
  double adfGeoTransform[6];
  adfGeoTransform[0] = tileBounds.getMinX(); // min longitude
  adfGeoTransform[1] = resolution;
  adfGeoTransform[2] = 0;
  adfGeoTransform[3] = tileBounds.getMaxY(); // max latitude
  adfGeoTransform[4] = 0;
  adfGeoTransform[5] = -resolution;

  GDALDatasetH hDstDS = GDALTiler::createRasterTile(adfGeoTransform);

  // The previous geotransform represented the data with an overlap as required
  // by the terrain specification.  This now needs to be overwritten so that
  // the data is shifted to the bounds defined by tile itself.
  tileBounds = mGrid.tileBounds(coord);
  resolution = mGrid.resolution(coord.zoom);
  adfGeoTransform[0] = tileBounds.getMinX(); // min longitude
  adfGeoTransform[1] = resolution;
  adfGeoTransform[2] = 0;
  adfGeoTransform[3] = tileBounds.getMaxY(); // max latitude
  adfGeoTransform[4] = 0;
  adfGeoTransform[5] = -resolution;

  // Set the shifted geo transform to the VRT
  if (GDALSetGeoTransform( hDstDS, adfGeoTransform ) != CE_None) {
    GDALClose(hDstDS);
    throw TerrainException("Could not set geo transform on VRT");
  }

  return hDstDS;
}

TerrainTiler &
terrain::TerrainTiler::operator=(const TerrainTiler &other) {
  GDALTiler::operator=(other);

  return *this;
}
