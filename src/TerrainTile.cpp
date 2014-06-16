#include <string.h>             // for memcpy

#include "zlib.h"
#include "ogr_spatialref.h"

#include "TerrainException.hpp"
#include "TerrainTile.hpp"
#include "GlobalGeodetic.hpp"
#include "Bounds.hpp"

using namespace terrain;

Terrain::Terrain():
  mHeights(TILE_CELL_SIZE),
  mChildren(0)
{
  setIsLand();
}

Terrain::Terrain(const char *fileName):
  mHeights(TILE_CELL_SIZE)
{
  readFile(fileName);
}

Terrain::Terrain(FILE *fp):
  mHeights(TILE_CELL_SIZE)
{
  unsigned char bytes[2];
  int count = 0;

  while ( count < TILE_CELL_SIZE && fread(bytes, 2, 1, fp) != 0) {
    /* adapted from
       <http://stackoverflow.com/questions/13001183/how-to-read-little-endian-integers-from-file-in-c> */
    mHeights[count++] = bytes[0] | (bytes[1]<<8);
  }

  if (count+1 != TILE_CELL_SIZE) {
    throw TerrainException("Not enough height data");
  }
  
  if ( fread(&(mChildren), 1, 1, fp) != 1 ) {
    throw TerrainException("Could not read child tile byte");
  }

  mMaskLength = fread(mMask, 1, MASK_CELL_SIZE, fp);
  switch (mMaskLength) {
  case MASK_CELL_SIZE:
    break;
  case 1:
    break;
  default:
    throw TerrainException("Not contain enough water mask data");
  }
}

void
Terrain::readFile(const char *fileName) {
  unsigned char inflateBuffer[MAX_TERRAIN_SIZE];
  unsigned int inflatedBytes;
  gzFile terrainFile = gzopen(fileName, "rb");

  if (terrainFile == NULL) {
    throw TerrainException("Failed to open file");
  }

  inflatedBytes = gzread(terrainFile, inflateBuffer, MAX_TERRAIN_SIZE);
  if (gzread(terrainFile, inflateBuffer, 1) != 0) {
    gzclose(terrainFile);
    throw TerrainException("File has too many bytes to be a valid terrain");
  }
  gzclose(terrainFile);

  switch(inflatedBytes) {
  case MAX_TERRAIN_SIZE:      // a water mask is present
    mMaskLength = MASK_CELL_SIZE;
    break;
  case (TILE_CELL_SIZE * 2) + 2:   // there is no water mask
    mMaskLength = 1;
    break;
  default:                    // it can't be a terrain file
    throw TerrainException("File has wrong file size to be a valid terrain");
  }

  short int byteCount = 0;
  for (short int i = 0; i < TILE_CELL_SIZE; i++, byteCount = i * 2) {
    mHeights[i] = inflateBuffer[byteCount] | (inflateBuffer[byteCount + 1]<<8);
  }

  mChildren = inflateBuffer[byteCount]; // byte 8451

  memcpy(mMask, &(inflateBuffer[++byteCount]), mMaskLength);
}

void
Terrain::writeFile(FILE *fp) const {
  fwrite(mHeights.data(), TILE_CELL_SIZE * 2, 1, fp);

  fwrite(&mChildren, 1, 1, fp);
  fwrite(mMask, mMaskLength, 1, fp);
}

void 
Terrain::writeFile(const char *fileName) const {
  gzFile terrainFile = gzopen(fileName, "wb");

  if (terrainFile == NULL) {
    throw TerrainException("Failed to open file");
  }

  if (gzwrite(terrainFile, mHeights.data(), TILE_CELL_SIZE * 2) == 0) {
    gzclose(terrainFile);
    throw TerrainException("Failed to write height data");
  }

  if (gzputc(terrainFile, mChildren) == -1) {
    gzclose(terrainFile);
    throw TerrainException("Failed to write child flags");
  }

  if (gzwrite(terrainFile, mMask, mMaskLength) == 0) {
    gzclose(terrainFile);
    throw TerrainException("Failed to write water mask");
  }

  switch (gzclose(terrainFile)) {
  case Z_OK:
    break;
  case Z_STREAM_ERROR:
  case Z_ERRNO:
  case Z_MEM_ERROR:
  case Z_BUF_ERROR:
  default:
    throw TerrainException("Failed to close file");
  }
}

std::vector<bool> Terrain::mask() {
  std::vector<bool> mask;
  mask.assign(mMask, mMask + mMaskLength);
  return mask;
}

/* for a discussion on bitflags see
   <http://www.dylanleigh.net/notes/c-cpp-tricks.html#Using_"Bitflags"> */
bool Terrain::hasChildren() const {
  return mChildren;
}

bool Terrain::hasChildSW() const {
  return ((mChildren & TERRAIN_CHILD_SW) == TERRAIN_CHILD_SW);
}

bool Terrain::hasChildSE() const {
  return ((mChildren & TERRAIN_CHILD_SE) == TERRAIN_CHILD_SE);
}

bool Terrain::hasChildNW() const {
  return ((mChildren & TERRAIN_CHILD_NW) == TERRAIN_CHILD_NW);
}

bool Terrain::hasChildNE() const {
  return ((mChildren & TERRAIN_CHILD_NE) == TERRAIN_CHILD_NE);
}

void Terrain::setChildSW(bool on) {
  if (on) {
    mChildren |= TERRAIN_CHILD_SW;
  } else {
    mChildren &= ~TERRAIN_CHILD_SW;
  }
}

void Terrain::setChildSE(bool on) {
  if (on) {
    mChildren |= TERRAIN_CHILD_SE;
  } else {
    mChildren &= ~TERRAIN_CHILD_SE;
  }
}

void Terrain::setChildNW(bool on) {
  if (on) {
    mChildren |= TERRAIN_CHILD_NW;
  } else {
    mChildren &= ~TERRAIN_CHILD_NW;
  }
}

void Terrain::setChildNE(bool on) {
  if (on) {
    mChildren |= TERRAIN_CHILD_NE;
  } else {
    mChildren &= ~TERRAIN_CHILD_NE;
  }
}

void Terrain::setAllChildren(bool on) {
  if (on) {
    mChildren = TERRAIN_CHILD_SW | TERRAIN_CHILD_SE | TERRAIN_CHILD_NW | TERRAIN_CHILD_NE;
  } else {
    mChildren = 0;
  }
}

void Terrain::setIsWater() {
  mMask[0] = 1;
  mMaskLength = 1;
}

bool Terrain::isWater() const {
  return mMaskLength == 1 && (bool) mMask[0];
}

void Terrain::setIsLand() {
  mMask[0] = 0;
  mMaskLength = 1;
}

bool Terrain::isLand() const {
  return mMaskLength == 1 && ! (bool) mMask[0];
}

bool Terrain::hasWaterMask() const {
  return mMaskLength == MASK_CELL_SIZE;
}

const std::vector<i_terrain_height> &
Terrain::getHeights() const {
  return mHeights;
}

std::vector<i_terrain_height> &
Terrain::getHeights() {
  return mHeights;
}

TerrainTile::TerrainTile(TileCoordinate coord):
  coord(coord) 
{}

TerrainTile::TerrainTile(const char *fileName, TileCoordinate coord):
  Terrain(fileName),
  coord(coord)
{}

TerrainTile::TerrainTile(const Terrain &terrain, TileCoordinate coord):
  Terrain(terrain),
  coord(coord)
{}

GDALDatasetH TerrainTile::heightsToRaster() const {
  const GlobalGeodetic profile;
  const LatLonBounds tileBounds = profile.tileBounds(coord);
  const i_tile tileSize = profile.tileSize();
  
  const double resolution = tileBounds.getWidth() / tileSize;
  double adfGeoTransform[6] = { tileBounds.getMinX(), resolution, 0, tileBounds.getMaxY(), 0, -resolution };

  OGRSpatialReference oSRS;
  if (oSRS.importFromEPSG(4326) != OGRERR_NONE) {
    throw TerrainException("Could not create EPSG:4326 spatial reference");
  }

  GDALDriverH hDriver = GDALGetDriverByName( "MEM" );
  GDALDatasetH hDstDS;
  GDALRasterBandH hBand;

  char *pszDstWKT = NULL;
  if (oSRS.exportToWkt( &pszDstWKT ) != OGRERR_NONE) {
    throw TerrainException("Could not create EPSG:4326 WKT string");
  }

  hDstDS = GDALCreate(hDriver, "", tileSize, tileSize, 1, GDT_Int16, NULL );
  if (hDstDS == NULL) {
    CPLFree( pszDstWKT );
    throw TerrainException("Could not create in memory raster");
  }

  if (GDALSetProjection( hDstDS, pszDstWKT ) != CE_None) {
    GDALClose(hDstDS);
    CPLFree( pszDstWKT );
    throw TerrainException("Could not set projection on in memory raster");
  }
  CPLFree( pszDstWKT );

  if (GDALSetGeoTransform( hDstDS, adfGeoTransform ) != CE_None) {
    GDALClose(hDstDS);
    throw TerrainException("Could not set projection on VRT");
  }

  hBand = GDALGetRasterBand( hDstDS, 1 );
  if (GDALRasterIO( hBand, GF_Write, 0, 0, tileSize, tileSize,
                    (void *) mHeights.data(), tileSize, tileSize, GDT_Int16, 0, 0 ) != CE_None) {
    GDALClose(hDstDS);
    throw TerrainException("Could not write heights to in memory raster");
  }

  return hDstDS;
}
