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
 * @file GDALTile.cpp
 * @brief This defines the `GDALTile` class
 */

#include "gdalwarper.h"

#include "GDALTile.hpp"

using namespace ctb;

GDALTile::~GDALTile() {
  if (dataset != NULL) {
    GDALClose(dataset);

    if (transformer != NULL) {
      GDALDestroyGenImgProjTransformer(transformer);
    }
  }
}

/// Detach the underlying GDAL dataset
GDALDataset *GDALTile::detach() {
  if (dataset != NULL) {
    GDALDataset *poDataset = dataset;
    dataset = NULL;

    if (transformer != NULL) {
      GDALDestroyGenImgProjTransformer(transformer);
      transformer = NULL;
    }
    return poDataset;
  }
  return NULL;
}
