#ifndef CTBZOUTPUTSTREAM_HPP
#define CTBZOUTPUTSTREAM_HPP

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
 * @file CTBZOutputStream.hpp
 * @brief This declares and defines the `CTBZOutputStream` class
 */

#include "zlib.h"
#include "CTBOutputStream.hpp"

namespace ctb {
  class CTBZFileOutputStream;
  class CTBZOutputStream;
}

/// Implements CTBOutputStream for `GZFILE` object
class CTB_DLL ctb::CTBZOutputStream : public ctb::CTBOutputStream {
public:
  CTBZOutputStream(gzFile gzptr): fp(gzptr) {}

  /// Writes a sequence of memory pointed by ptr into the stream
  virtual uint32_t write(const void *ptr, uint32_t size);

protected:
  /// The underlying GZFILE*
  gzFile fp;
};

/// Implements CTBOutputStream for gzipped files
class CTB_DLL ctb::CTBZFileOutputStream : public ctb::CTBZOutputStream {
public:
  CTBZFileOutputStream(const char *fileName);
 ~CTBZFileOutputStream();

  void close();
};

#endif /* CTBZOUTPUTSTREAM_HPP */
