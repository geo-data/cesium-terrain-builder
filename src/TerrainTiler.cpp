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

#include "CTBException.hpp"
#include "TerrainTiler.hpp"

#include <iostream>

using namespace ctb;

TerrainTile *
ctb::TerrainTiler::createTile(const TileCoordinate &coord) const {
  // Get a terrain tile represented by the tile coordinate
  TerrainTile *terrainTile = new TerrainTile(coord);
  GDALTile *rasterTile = createRasterTile(coord); // the raster associated with this tile coordinate

  if (rasterTile == nullptr) {
	  terrainTile->setIsValid(false);
	  return terrainTile;
  }

  GDALRasterBand *heightsBand = rasterTile->dataset->GetRasterBand(1);

  // Copy the raster data into an array
  float rasterHeights[TerrainTile::TILE_CELL_SIZE];
  float bandNoDataValue = heightsBand->GetNoDataValue();

  if (heightsBand->RasterIO(GF_Read, 0, 0, TILE_SIZE, TILE_SIZE,
	  (void *)rasterHeights, TILE_SIZE, TILE_SIZE, GDT_Float32,
	  0, 0) != CE_None) {
	  throw CTBException("Could not read heights from raster");
  }

  bool isInvalid = true;

  for (int i = 0; i < TerrainTile::TILE_CELL_SIZE; i++) {
	  const bool heightIsNoData = rasterHeights[i] == bandNoDataValue;
	  isInvalid &= heightIsNoData;
  }

  delete rasterTile;

  // Convert the raster data into the terrain tile heights.  This assumes the
  // input raster data represents meters above sea level. Each terrain height
  // value is the number of 1/5 meter units above -1000 meters.
  // TODO: try doing this using a VRT derived band:
  // (http://www.gdal.org/gdal_vrttut.html)
  for (unsigned short int i = 0; i < TerrainTile::TILE_CELL_SIZE; i++) {
    terrainTile->mHeights[i] = (i_terrain_height) ((rasterHeights[i] + 1000) * 5);
  }

  // If we are not at the maximum zoom level we need to set child flags on the
  // tile where child tiles overlap the dataset bounds.
  if (coord.zoom != maxZoomLevel()) {
    CRSBounds tileBounds = mGrid.tileBounds(coord);

    if (! (bounds().overlaps(tileBounds))) {
      terrainTile->setAllChildren(false);
    } else {
      if (bounds().overlaps(tileBounds.getSW())) {
        terrainTile->setChildSW();
      }
      if (bounds().overlaps(tileBounds.getNW())) {
        terrainTile->setChildNW();
      }
      if (bounds().overlaps(tileBounds.getNE())) {
        terrainTile->setChildNE();
      }
      if (bounds().overlaps(tileBounds.getSE())) {
        terrainTile->setChildSE();
      }
    }
  }

  return terrainTile;
}

// --------------------------------------------------------------
// Transformation World to Pixel
// trfm - Parameter array read from gdal raster dataset
// x, y - World postion
// col, row - Pixel postions (return values)
// return true if the calculation is valid and 0 if there is a
// division by zero
// -------------------------------------------------------------
int calcWorldToPixel(double *trfm,
	double x, double y,
	long   *col, long *row) {
	double div = (trfm[2] * trfm[4] - trfm[1] * trfm[5]);
	if (div<DBL_EPSILON * 2) return 0;
	double dcol = -(trfm[2] * (trfm[3] - y) + trfm[5] * x - trfm[0] * trfm[5]) / div;
	double drow = (trfm[1] * (trfm[3] - y) + trfm[4] * x - trfm[0] * trfm[4]) / div;
	*col = round(dcol); *row = round(drow);
	return 1;
}


GDALTile *
ctb::TerrainTiler::createRasterTile(const TileCoordinate &coord) const {
  // Ensure we have some data from which to create a tile
  if (poDataset && poDataset->GetRasterCount() < 1) {
    throw CTBException("At least one band must be present in the GDAL dataset");
  }

  // Get the bounds and resolution for a tile coordinate which represents the
  // data overlap requested by the terrain specification.
  double resolution;
  CRSBounds tileBounds = terrainTileBounds(coord, resolution);

  double ulX = tileBounds.getMinX();
  double ulY = tileBounds.getMaxY();
  double lrX = tileBounds.getMaxX();
  double lrY = tileBounds.getMinY();

  long ulCol, ulRow, lrCol, lrRow;

  double datasetTransform[6];

  GDALGetGeoTransform(poDataset, datasetTransform);

  int imgWidth = GDALGetRasterXSize(poDataset);
  int imgHeight = GDALGetRasterYSize(poDataset);

  double datasetUlLon = datasetTransform[0];
  double datasetUlLat = datasetTransform[3];

  double datasetLrLon = datasetUlLon + (datasetTransform[1] * imgWidth);
  double datasetLrLat = datasetUlLat + (datasetTransform[5] * imgHeight);
  


  // Convert the tile bounds into a geo transform
  double adfGeoTransform[6];
  adfGeoTransform[0] = tileBounds.getMinX(); // min longitude
  adfGeoTransform[1] = resolution;
  adfGeoTransform[2] = 0;
  adfGeoTransform[3] = tileBounds.getMaxY(); // max latitude
  adfGeoTransform[4] = 0;
  adfGeoTransform[5] = -resolution;
  
  calcWorldToPixel(datasetTransform, ulX, ulY, &ulCol, &ulRow);
  calcWorldToPixel(datasetTransform, lrX, lrY, &lrCol, &lrRow);

  int xOffset = (int)ulCol;
  int yOffset = (int)ulRow;
  int xSize = lrCol  -ulCol;
  int ySize = lrRow - ulRow;

  uint16_t nBandCount = poDataset->GetRasterCount();

  bool allBandsValid = true;

  //extern int validTiles;
  //extern int emptyTiles;

  

  for (int i = 0; i < nBandCount; i++) {
	  int bGotNoData = FALSE;
	  GDALRasterBand* band = dataset()->GetRasterBand(i + 1);
	  double noDataValue = band->GetNoDataValue(&bGotNoData);
	  if (!bGotNoData) noDataValue = -32768;

	  double percentCoverage = 0;

	  bool isValidWindow = (xOffset >= 0 && yOffset >= 0 && xOffset + xSize <= imgWidth && yOffset + ySize <= imgHeight);

	  bool blockIsEmpty = false;

	  if (isValidWindow) {
		  int bandCoverageStatus = band->GetDataCoverageStatus(xOffset, yOffset, xSize, ySize, 0, &percentCoverage);

		  blockIsEmpty = bandCoverageStatus == GDAL_DATA_COVERAGE_STATUS_EMPTY;
	  }
	  
	  allBandsValid &= !blockIsEmpty;

	  if (blockIsEmpty) {
		  //return nullptr;
		  //emptyTiles++;
		  //std::cout << "Empty tile: " << coord.zoom << "," << coord.x << "," << coord.y << std::endl;
	  }
	  else {
		  //validTiles++;
	  }

	  /*
	  double percentCoverageCenter = 0;
	  int centerCoverageStatus = band->GetDataCoverageStatus(imgWidth / 2, imgHeight / 2, 1, 1, 0, &percentCoverageCenter);
	  bool centerIsEmpty = centerCoverageStatus == GDAL_DATA_COVERAGE_STATUS_EMPTY;

	  int q = 42;
	  int z = q;
	  */
  }

  if (!allBandsValid) {
	  return nullptr;
  }

  GDALTile *tile = GDALTiler::createRasterTile(adfGeoTransform);

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
  if (GDALSetGeoTransform(tile->dataset, adfGeoTransform) != CE_None) {
    throw CTBException("Could not set geo transform on VRT");
  }

  return tile;
}

TerrainTiler &
ctb::TerrainTiler::operator=(const TerrainTiler &other) {
  GDALTiler::operator=(other);

  return *this;
}
