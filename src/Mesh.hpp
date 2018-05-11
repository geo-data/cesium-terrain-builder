#ifndef CTBMESH_HPP
#define CTBMESH_HPP

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
 * @file Mesh.hpp
 * @brief This declares the `Mesh` class
 * @author Alvaro Huarte <ahuarte47@yahoo.es>
 */

#include <cstdint>
#include <vector>
#include <cstring>
#include "types.hpp"
#include "CTBException.hpp"

namespace ctb {
  class Mesh;
}

/**
 * @brief An abstract base class for a mesh of triangles
 */
class ctb::Mesh
{
public:

  /// Create an empty mesh
  Mesh()
  {}

public:

  /// The array of shared vertices of a mesh
  std::vector<CRSVertex> vertices;

  /// The index collection for each triangle in the mesh (3 for each triangle)
  std::vector<uint32_t> indices;

  /// Write mesh data to a WKT file
  void writeWktFile(const char *fileName) const {
    FILE *fp = fopen(fileName, "w");

    if (fp == NULL) {
      throw CTBException("Failed to open file");
    }

    char wktText[512];
    memset(wktText, 0, sizeof(wktText));

    for (int i = 0, icount = indices.size(); i < icount; i += 3) {
      CRSVertex v0 = vertices[indices[i]];
      CRSVertex v1 = vertices[indices[i+1]];
      CRSVertex v2 = vertices[indices[i+2]];

      sprintf(wktText, "(%.8f %.8f %f, %.8f %.8f %f, %.8f %.8f %f, %.8f %.8f %f)",
        v0.x, v0.y, v0.z,
        v1.x, v1.y, v1.z,
        v2.x, v2.y, v2.z,
        v0.x, v0.y, v0.z);
      fprintf(fp, "POLYGON Z(%s)\n", wktText);
    }
    fclose(fp);
  };
};

#endif /* CTBMESH_HPP */
