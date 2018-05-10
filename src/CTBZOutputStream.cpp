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
 * @file CTBZOutputStream.cpp
 * @brief This defines the `CTBZOutputStream` and `CTBZFileOutputStream` classes
 */

#include "CTBException.hpp"
#include "CTBZOutputStream.hpp"

using namespace ctb;

/**
 * @details 
 * Writes a sequence of memory pointed by ptr into the GZFILE*.
 */
uint32_t
ctb::CTBZOutputStream::write(const void *ptr, uint32_t size) {
  if (size == 1) {
    int c = *((const char *)ptr);
    return gzputc(fp, c) == -1 ? 0 : 1;
  }
  else {
    return gzwrite(fp, ptr, size) == 0 ? 0 : size;
  }
}

ctb::CTBZFileOutputStream::CTBZFileOutputStream(const char *fileName) : CTBZOutputStream(NULL) {
  gzFile file = gzopen(fileName, "wb");

  if (file == NULL) {
    throw CTBException("Failed to open file");
  }
  fp = file;
}

ctb::CTBZFileOutputStream::~CTBZFileOutputStream() {
  close();
}

void
ctb::CTBZFileOutputStream::close() {

  // Try and close the file
  if (fp) {
    switch (gzclose(fp)) {
    case Z_OK:
      break;
    case Z_STREAM_ERROR:
    case Z_ERRNO:
    case Z_MEM_ERROR:
    case Z_BUF_ERROR:
    default:
      throw CTBException("Failed to close file");
    }
    fp = NULL;
  }
}
