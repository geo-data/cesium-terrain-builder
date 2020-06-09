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
 * @file GlobalGeodetic.cpp
 * @brief This defines the `GlobalGeodetic` class
 */

#include "GlobalGeodetic.hpp"

using namespace ctb;

// Set the spatial reference
static OGRSpatialReference
setSRS(void) {
  OGRSpatialReference srs;
  
  #if ( GDAL_VERSION_MAJOR >= 3 )
  srs.SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);
  #endif
  
  srs.importFromEPSG(4326);
  return srs;
}

const OGRSpatialReference GlobalGeodetic::cSRS = setSRS();
