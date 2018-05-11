#ifndef CTBFILEOUTPUTSTREAM_HPP
#define CTBFILEOUTPUTSTREAM_HPP

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
 * @file CTBFileOutputStream.hpp
 * @brief This declares and defines the `CTBFileOutputStream` and `CTBStdOutputStream` classes
 */

#include <stdio.h>
#include <ostream>
#include "CTBOutputStream.hpp"

namespace ctb {
  class CTBFileOutputStream;
  class CTBStdOutputStream;
}

/// Implements CTBOutputStream for `FILE*` objects
class CTB_DLL ctb::CTBFileOutputStream : public ctb::CTBOutputStream {
public:
  CTBFileOutputStream(FILE *fptr): fp(fptr) {}

  /// Writes a sequence of memory pointed by ptr into the stream
  virtual uint32_t write(const void *ptr, uint32_t size);

protected:
  /// The underlying FILE*
  FILE *fp;
};

/// Implements CTBOutputStream for `std::ostream` objects
class CTB_DLL ctb::CTBStdOutputStream : public ctb::CTBOutputStream {
public:
  CTBStdOutputStream(std::ostream &stream): mstream(stream) {}

  /// Writes a sequence of memory pointed by ptr into the stream
  virtual uint32_t write(const void *ptr, uint32_t size);

protected:
  /// The underlying std::ostream
  std::ostream &mstream;
};

#endif /* CTBFILEOUTPUTSTREAM_HPP */
