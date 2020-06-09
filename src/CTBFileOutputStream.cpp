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
 * @file CTBFileOutputStream.cpp
 * @brief This defines the `CTBFileOutputStream` class
 */

#include "CTBFileOutputStream.hpp"

using namespace ctb;

/**
 * @details 
 * Writes a sequence of memory pointed by ptr into the FILE*.
 */
uint32_t
ctb::CTBFileOutputStream::write(const void *ptr, uint32_t size) {
  return (uint32_t)fwrite(ptr, size, 1, fp);
}

/**
 * @details 
 * Writes a sequence of memory pointed by ptr into the ostream. 
 */
uint32_t
ctb::CTBStdOutputStream::write(const void *ptr, uint32_t size) {
  mstream.write((const char *)ptr, size);
  return size;
}
