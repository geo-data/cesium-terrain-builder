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
 * @file GlobalMercator.cpp
 * @brief This defines the `GlobalMercator` class
 */

#include "GlobalMercator.hpp"

using namespace terrain;

const unsigned int GlobalMercator::mSemiMajorAxis = 6378137;
const double GlobalMercator::mEarthCircumference = 2 * M_PI * GlobalMercator::mSemiMajorAxis;
const double GlobalMercator::mOriginShift = GlobalMercator::mEarthCircumference / 2.0;
