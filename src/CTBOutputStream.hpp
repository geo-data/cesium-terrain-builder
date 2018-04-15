#ifndef CTBOUTPUTSTREAM_HPP
#define CTBOUTPUTSTREAM_HPP

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
 * @file CTBOutputStream.hpp
 * @brief This declares and defines the `CTBOutputStream` class
 */

#include "config.hpp"
#include "types.hpp"

namespace ctb {
  class CTBOutputStream;
}

/// This represents a generic CTB output stream to write raw data
class CTB_DLL ctb::CTBOutputStream {
public:

  /// Writes a sequence of memory pointed by ptr into the stream
  virtual uint32_t write(const void *ptr, uint32_t size) = 0;
};

#endif /* CTBOUTPUTSTREAM_HPP */
