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
 * @file MeshTile.cpp
 * @brief This defines the `MeshTile` class
 * @author Alvaro Huarte <ahuarte47@yahoo.es>
 */

#include <cmath>
#include <vector>
#include <map>
#include "cpl_conv.h"

#include "CTBException.hpp"
#include "MeshTile.hpp"
#include "BoundingSphere.hpp"
#include "CTBZOutputStream.hpp"

using namespace ctb;

////////////////////////////////////////////////////////////////////////////////
// Utility functions

// Constants taken from http://cesiumjs.org/2013/04/25/Horizon-culling
double llh_ecef_radiusX = 6378137.0;
double llh_ecef_radiusY = 6378137.0;
double llh_ecef_radiusZ = 6356752.3142451793;

double llh_ecef_rX = 1.0 / llh_ecef_radiusX;
double llh_ecef_rY = 1.0 / llh_ecef_radiusY;
double llh_ecef_rZ = 1.0 / llh_ecef_radiusZ;

// Stolen from https://github.com/bistromath/gr-air-modes/blob/master/python/mlat.py
// WGS84 reference ellipsoid constants
// http://en.wikipedia.org/wiki/Geodetic_datum#Conversion_calculations
// http://en.wikipedia.org/wiki/File%3aECEF.png
//
double llh_ecef_wgs84_a = llh_ecef_radiusX;       // Semi - major axis
double llh_ecef_wgs84_b = llh_ecef_radiusZ;       // Semi - minor axis
double llh_ecef_wgs84_e2 = 0.0066943799901975848; // First eccentricity squared

// LLH2ECEF
static inline double llh_ecef_n(double x) {
  double snx = std::sin(x);
  return llh_ecef_wgs84_a / std::sqrt(1.0 - llh_ecef_wgs84_e2 * (snx * snx));
}
static inline CRSVertex LLH2ECEF(const CRSVertex& coordinate) {
  double lon = coordinate.x * (M_PI / 180.0);
  double lat = coordinate.y * (M_PI / 180.0);
  double alt = coordinate.z;

  double x = (llh_ecef_n(lat) + alt) * std::cos(lat) * std::cos(lon);
  double y = (llh_ecef_n(lat) + alt) * std::cos(lat) * std::sin(lon);
  double z = (llh_ecef_n(lat) * (1.0 - llh_ecef_wgs84_e2) + alt) * std::sin(lat);

  return CRSVertex(x, y, z);
}

// HORIZON OCCLUSION POINT
// https://cesiumjs.org/2013/05/09/Computing-the-horizon-occlusion-point
//
static inline double ocp_computeMagnitude(const CRSVertex &position, const CRSVertex &sphereCenter) {
  double magnitudeSquared = position.magnitudeSquared();
  double magnitude = std::sqrt(magnitudeSquared);
  CRSVertex direction = position * (1.0 / magnitude);

  // For the purpose of this computation, points below the ellipsoid
  // are considered to be on it instead.
  magnitudeSquared = std::fmax(1.0, magnitudeSquared);
  magnitude = std::fmax(1.0, magnitude);

  double cosAlpha = direction.dot(sphereCenter);
  double sinAlpha = direction.cross(sphereCenter).magnitude();
  double cosBeta = 1.0 / magnitude;
  double sinBeta = std::sqrt(magnitudeSquared - 1.0) * cosBeta;

  return 1.0 / (cosAlpha * cosBeta - sinAlpha * sinBeta);
}
static inline CRSVertex ocp_fromPoints(const std::vector<CRSVertex> &points, const BoundingSphere<double> &boundingSphere) {
  const double MIN = -std::numeric_limits<double>::infinity();
  double max_magnitude = MIN;

  // Bring coordinates to ellipsoid scaled coordinates
  const CRSVertex &center = boundingSphere.center;
  CRSVertex scaledCenter = CRSVertex(center.x * llh_ecef_rX, center.y * llh_ecef_rY, center.z * llh_ecef_rZ);

  for (int i = 0, icount = points.size(); i < icount; i++) {
    const CRSVertex &point = points[i];
    CRSVertex scaledPoint(point.x * llh_ecef_rX, point.y * llh_ecef_rY, point.z * llh_ecef_rZ);

    double magnitude = ocp_computeMagnitude(scaledPoint, scaledCenter);
    if (magnitude > max_magnitude) max_magnitude = magnitude;
  }
  return scaledCenter * max_magnitude;
}

// PACKAGE IO
const double SHORT_MAX = 32767.0;
const int BYTESPLIT = 65636;

static inline int quantizeIndices(const double &origin, const double &factor, const double &value) {
  return int(std::round((value - origin) * factor));
}

// Write the edge indices of the mesh
template <typename T> int writeEdgeIndices(CTBOutputStream &ostream, const Mesh &mesh, double edgeCoord, int componentIndex) {
  std::vector<uint32_t> indices;
  std::map<uint32_t, size_t> ihash;

  for (size_t i = 0, icount = mesh.indices.size(); i < icount; i++) {
    uint32_t indice = mesh.indices[i];
    double val = mesh.vertices[indice][componentIndex];

    if (val == edgeCoord) {
      std::map<uint32_t, size_t>::iterator it = ihash.find(indice);

      if (it == ihash.end()) {
        ihash.insert(std::make_pair(indice, i));
        indices.push_back(indice);
      }
    }
  }

  int edgeCount = indices.size();
  ostream.write(&edgeCount, sizeof(int));

  for (size_t i = 0; i < edgeCount; i++) {
    T indice = (T)indices[i];
    ostream.write(&indice, sizeof(T));
  }
  return indices.size();
}

// ZigZag-Encodes a number (-1 = 1, -2 = 3, 0 = 0, 1 = 2, 2 = 4)
static inline uint16_t zigZagEncode(int n) {
  return (n << 1) ^ (n >> 31);
}

// Triangle area
static inline double triangleArea(const CRSVertex &a, const CRSVertex &b) {
  double i = std::pow(a[1] * b[2] - a[2] * b[1], 2);
  double j = std::pow(a[2] * b[0] - a[0] * b[2], 2);
  double k = std::pow(a[0] * b[1] - a[1] * b[0], 2);
  return 0.5 * sqrt(i + j + k);
}

// Constraint a value to lie between two values
static inline double clamp_value(double value, double min, double max) {
  return value < min ? min : value > max ? max : value;
}

// Converts a scalar value in the range [-1.0, 1.0] to a SNORM in the range [0, rangeMax]
static inline unsigned char snorm_value(double value, double rangeMax = 255) {
  return (unsigned char)int(std::round((clamp_value(value, -1.0, 1.0) * 0.5 + 0.5) * rangeMax));
}

/**
 * Encodes a normalized vector into 2 SNORM values in the range of [0-rangeMax] following the 'oct' encoding.
 *
 * Oct encoding is a compact representation of unit length vectors.
 * The 'oct' encoding is described in "A Survey of Efficient Representations of Independent Unit Vectors",
 * Cigolle et al 2014: {@link http://jcgt.org/published/0003/02/01/}
 */
static inline Coordinate<unsigned char> octEncode(const CRSVertex &vector, double rangeMax = 255) {
  Coordinate<double> temp;
  double llnorm = std::abs(vector.x) + std::abs(vector.y) + std::abs(vector.z);
  temp.x = vector.x / llnorm;
  temp.y = vector.y / llnorm;

  if (vector.z < 0) {
    double x = temp.x;
    double y = temp.y;
    temp.x = (1.0 - std::abs(y)) * (x < 0.0 ? -1.0 : 1.0);
    temp.y = (1.0 - std::abs(x)) * (y < 0.0 ? -1.0 : 1.0);
  }
  return Coordinate<unsigned char>(snorm_value(temp.x, rangeMax), snorm_value(temp.y, rangeMax));
}

////////////////////////////////////////////////////////////////////////////////

MeshTile::MeshTile():
  Tile(),
  mChildren(0)
{}

MeshTile::MeshTile(const TileCoordinate &coord):
  Tile(coord),
  mChildren(0)
{}

/**
 * @details This writes gzipped terrain data to a file.
 */
void 
MeshTile::writeFile(const char *fileName, bool writeVertexNormals) const {
  CTBZFileOutputStream ostream(fileName);
  writeFile(ostream);
}

/**
 * @details This writes raw terrain data to an output stream.
 */
void
MeshTile::writeFile(CTBOutputStream &ostream, bool writeVertexNormals) const {

  // Calculate main header mesh data
  std::vector<CRSVertex> cartesianVertices;
  BoundingSphere<double> cartesianBoundingSphere;
  BoundingBox<double> cartesianBounds;
  BoundingBox<double> bounds;

  cartesianVertices.resize(mMesh.vertices.size());
  for (size_t i = 0, icount = mMesh.vertices.size(); i < icount; i++) {
    const CRSVertex &vertex = mMesh.vertices[i];
    cartesianVertices[i] = LLH2ECEF(vertex);
  }
  cartesianBoundingSphere.fromPoints(cartesianVertices);
  cartesianBounds.fromPoints(cartesianVertices);
  bounds.fromPoints(mMesh.vertices);


  // # Write the mesh header data:
  // # https://github.com/AnalyticalGraphicsInc/quantized-mesh
  //
  // The center of the tile in Earth-centered Fixed coordinates.
  double centerX = cartesianBounds.min.x + 0.5 * (cartesianBounds.max.x - cartesianBounds.min.x);
  double centerY = cartesianBounds.min.y + 0.5 * (cartesianBounds.max.y - cartesianBounds.min.y);
  double centerZ = cartesianBounds.min.z + 0.5 * (cartesianBounds.max.z - cartesianBounds.min.z);
  ostream.write(&centerX, sizeof(double));
  ostream.write(&centerY, sizeof(double));
  ostream.write(&centerZ, sizeof(double));
  //
  // The minimum and maximum heights in the area covered by this tile.
  float minimumHeight = (float)bounds.min.z;
  float maximumHeight = (float)bounds.max.z;
  ostream.write(&minimumHeight, sizeof(float));
  ostream.write(&maximumHeight, sizeof(float));
  //
  // The tile's bounding sphere. The X,Y,Z coordinates are again expressed
  // in Earth-centered Fixed coordinates, and the radius is in meters.
  double boundingSphereCenterX = cartesianBoundingSphere.center.x;
  double boundingSphereCenterY = cartesianBoundingSphere.center.y;
  double boundingSphereCenterZ = cartesianBoundingSphere.center.z;
  double boundingSphereRadius  = cartesianBoundingSphere.radius;
  ostream.write(&boundingSphereCenterX, sizeof(double));
  ostream.write(&boundingSphereCenterY, sizeof(double));
  ostream.write(&boundingSphereCenterZ, sizeof(double));
  ostream.write(&boundingSphereRadius , sizeof(double));
  //
  // The horizon occlusion point, expressed in the ellipsoid-scaled Earth-centered Fixed frame.
  CRSVertex horizonOcclusionPoint = ocp_fromPoints(cartesianVertices, cartesianBoundingSphere);
  ostream.write(&horizonOcclusionPoint.x, sizeof(double));
  ostream.write(&horizonOcclusionPoint.y, sizeof(double));
  ostream.write(&horizonOcclusionPoint.z, sizeof(double));


  // # Write mesh vertices (X Y Z components of each vertex):
  int vertexCount = mMesh.vertices.size();
  ostream.write(&vertexCount, sizeof(int));
  for (int c = 0; c < 3; c++) {
    double origin = bounds.min[c];
    double factor = 0;
    if (bounds.max[c] > bounds.min[c]) factor = SHORT_MAX / (bounds.max[c] - bounds.min[c]);

    // Move the initial value
    int u0 = quantizeIndices(origin, factor, mMesh.vertices[0][c]), u1, ud;
    uint16_t sval = zigZagEncode(u0);
    ostream.write(&sval, sizeof(uint16_t));

    for (size_t i = 1, icount = mMesh.vertices.size(); i < icount; i++) {
      u1 = quantizeIndices(origin, factor, mMesh.vertices[i][c]);
      ud = u1 - u0;
      sval = zigZagEncode(ud);
      ostream.write(&sval, sizeof(uint16_t));
      u0 = u1;
    }
  }

  // # Write mesh indices:
  int triangleCount = mMesh.indices.size() / 3;
  ostream.write(&triangleCount, sizeof(int));
  if (vertexCount > BYTESPLIT) {
    uint32_t highest = 0;
    uint32_t code;

    // Write main indices
    for (size_t i = 0, icount = mMesh.indices.size(); i < icount; i++) {
      code = highest - mMesh.indices[i];
      ostream.write(&code, sizeof(uint32_t));
      if (code == 0) highest++;
    }

    // Write all vertices on the edge of the tile (W, S, E, N)
    writeEdgeIndices<uint32_t>(ostream, mMesh, bounds.min.x, 0);
    writeEdgeIndices<uint32_t>(ostream, mMesh, bounds.min.y, 1);
    writeEdgeIndices<uint32_t>(ostream, mMesh, bounds.max.x, 0);
    writeEdgeIndices<uint32_t>(ostream, mMesh, bounds.max.y, 1);
  }
  else {
    uint16_t highest = 0;
    uint16_t code;

    // Write main indices
    for (size_t i = 0, icount = mMesh.indices.size(); i < icount; i++) {
      code = highest - mMesh.indices[i];
      ostream.write(&code, sizeof(uint16_t));
      if (code == 0) highest++;
    }

    // Write all vertices on the edge of the tile (W, S, E, N)
    writeEdgeIndices<uint16_t>(ostream, mMesh, bounds.min.x, 0);
    writeEdgeIndices<uint16_t>(ostream, mMesh, bounds.min.y, 1);
    writeEdgeIndices<uint16_t>(ostream, mMesh, bounds.max.x, 0);
    writeEdgeIndices<uint16_t>(ostream, mMesh, bounds.max.y, 1);
  }

  // # Write 'Oct-Encoded Per-Vertex Normals' for Terrain Lighting:
  if (writeVertexNormals && triangleCount > 0) {
    unsigned char extensionId = 1;
    ostream.write(&extensionId, sizeof(unsigned char));
    int extensionLength = 2 * vertexCount;
    ostream.write(&extensionLength, sizeof(int));

    std::vector<CRSVertex> normalsPerVertex(vertexCount);
    std::vector<CRSVertex> normalsPerFace(triangleCount);
    std::vector<double> areasPerFace(triangleCount);

    for (size_t i = 0, icount = mMesh.indices.size(), j = 0; i < icount; i+=3, j++) {
      const CRSVertex &v0 = cartesianVertices[ mMesh.indices[i  ] ];
      const CRSVertex &v1 = cartesianVertices[ mMesh.indices[i+1] ];
      const CRSVertex &v2 = cartesianVertices[ mMesh.indices[i+2] ];

      CRSVertex normal = (v1 - v0).cross(v2 - v0);
      double area = triangleArea(v0, v1);
      normalsPerFace[j] = normal;
      areasPerFace[j] = area;
    }
    for (size_t i = 0, icount = mMesh.indices.size(), j = 0; i < icount; i+=3, j++) {
      int indexV0 = mMesh.indices[i  ];
      int indexV1 = mMesh.indices[i+1];
      int indexV2 = mMesh.indices[i+2];

      CRSVertex weightedNormal = normalsPerFace[j] * areasPerFace[j];

      normalsPerVertex[indexV0] = normalsPerVertex[indexV0] + weightedNormal;
      normalsPerVertex[indexV1] = normalsPerVertex[indexV1] + weightedNormal;
      normalsPerVertex[indexV2] = normalsPerVertex[indexV2] + weightedNormal;
    }
    for (size_t i = 0; i < vertexCount; i++) {
      Coordinate<unsigned char> xy = octEncode(normalsPerVertex[i].normalize());
      ostream.write(&xy.x, sizeof(unsigned char));
      ostream.write(&xy.y, sizeof(unsigned char));
    }
  }
}

bool
MeshTile::hasChildren() const {
  return mChildren;
}

bool
MeshTile::hasChildSW() const {
  return ((mChildren & TERRAIN_CHILD_SW) == TERRAIN_CHILD_SW);
}

bool
MeshTile::hasChildSE() const {
  return ((mChildren & TERRAIN_CHILD_SE) == TERRAIN_CHILD_SE);
}

bool
MeshTile::hasChildNW() const {
  return ((mChildren & TERRAIN_CHILD_NW) == TERRAIN_CHILD_NW);
}

bool
MeshTile::hasChildNE() const {
  return ((mChildren & TERRAIN_CHILD_NE) == TERRAIN_CHILD_NE);
}

void
MeshTile::setChildSW(bool on) {
  if (on) {
    mChildren |= TERRAIN_CHILD_SW;
  } else {
    mChildren &= ~TERRAIN_CHILD_SW;
  }
}

void
MeshTile::setChildSE(bool on) {
  if (on) {
    mChildren |= TERRAIN_CHILD_SE;
  } else {
    mChildren &= ~TERRAIN_CHILD_SE;
  }
}

void
MeshTile::setChildNW(bool on) {
  if (on) {
    mChildren |= TERRAIN_CHILD_NW;
  } else {
    mChildren &= ~TERRAIN_CHILD_NW;
  }
}

void
MeshTile::setChildNE(bool on) {
  if (on) {
    mChildren |= TERRAIN_CHILD_NE;
  } else {
    mChildren &= ~TERRAIN_CHILD_NE;
  }
}

void
MeshTile::setAllChildren(bool on) {
  if (on) {
    mChildren = TERRAIN_CHILD_SW | TERRAIN_CHILD_SE | TERRAIN_CHILD_NW | TERRAIN_CHILD_NE;
  } else {
    mChildren = 0;
  }
}

const Mesh & MeshTile::getMesh() const {
  return mMesh;
}

Mesh & MeshTile::getMesh() {
  return mMesh;
}
