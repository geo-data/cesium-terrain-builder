#include <vector>
#include <string.h>             // for memcpy

#include "zlib.h"

#include "gdal_priv.h"
#include "ogr_spatialref.h"

#define TILE_SIZE 65 * 65
#define MASK_SIZE 256 * 256

// The maximum byte size of an uncompressed terrain tile (heights + child flags
// + water mask)
#define MAX_TERRAIN_SIZE ( TILE_SIZE * 2 ) + 1 + MASK_SIZE

enum TerrainChildren {
  TC_SW = 1,                    // 2^0, bit 0
  TC_SE = 2,                    // 2^1, bit 1
  TC_NW = 4,                    // 2^2, bit 2
  TC_NE = 8                     // 2^3, bit 3
};

class TerrainTile {
public:
  TerrainTile():
    mHeights(TILE_SIZE),
    mChildren(0)
  {
    setIsLand();
  };

  TerrainTile(const char *fileName):
    mHeights(TILE_SIZE)
  {
    unsigned char inflateBuffer[MAX_TERRAIN_SIZE];
    unsigned int inflatedBytes;
    gzFile terrainFile = gzopen(fileName, "rb");

    if (terrainFile == NULL) {
      throw 1;
    }

    inflatedBytes = gzread(terrainFile, inflateBuffer, MAX_TERRAIN_SIZE);
    if (gzread(terrainFile, inflateBuffer, 1) != 0) {
      gzclose(terrainFile);
      throw 2;
    }
    gzclose(terrainFile);

    switch(inflatedBytes) {
    case MAX_TERRAIN_SIZE:      // a water mask is present
      mMaskLength = MASK_SIZE;
      break;
    case (TILE_SIZE * 2) + 2:   // there is no water mask
      mMaskLength = 1;
      break;
    default:                    // it can't be a terrain file
      throw 3;
    }

    short int byteCount = 0;
    for (short int i = 0; i < TILE_SIZE; i++, byteCount = i * 2) {
      mHeights[i] = inflateBuffer[byteCount] | (inflateBuffer[byteCount + 1]<<8);
    }

    mChildren = inflateBuffer[byteCount]; // byte 8451

    memcpy(mMask, &(inflateBuffer[++byteCount]), mMaskLength);
  }

  TerrainTile(FILE *fp):
    mHeights(TILE_SIZE)
  {
    unsigned char bytes[2];
    int count = 0;

    while ( count < TILE_SIZE && fread(bytes, 2, 1, fp) != 0) {
      /* adapted from
         <http://stackoverflow.com/questions/13001183/how-to-read-little-endian-integers-from-file-in-c> */
      mHeights[count++] = bytes[0] | (bytes[1]<<8);
    }

    if ( fread(&(mChildren), 1, 1, fp) != 1 ) {
      throw 1;
    }

    mMaskLength = fread(mMask, 1, MASK_SIZE, fp);
    switch (mMaskLength) {
    case MASK_SIZE:
      break;
    case 1:
      break;
    default:
      throw 2;
    }
  }

  void writeFile(FILE *fp) {
    fwrite(mHeights.data(), TILE_SIZE * 2, 1, fp);

    fwrite(&mChildren, 1, 1, fp);
    fwrite(mMask, mMaskLength, 1, fp);
  }

  void writeFile(const char *fileName) {
    gzFile terrainFile = gzopen(fileName, "wb");

    if (terrainFile == NULL) {
      throw 1;
    }

    if (gzwrite(terrainFile, mHeights.data(), TILE_SIZE * 2) == 0) {
      gzclose(terrainFile);
      throw 2;
    }

    if (gzputc(terrainFile, mChildren) == -1) {
      gzclose(terrainFile);
      throw 3;
    }

    if (gzwrite(terrainFile, mMask, mMaskLength) == 0) {
      gzclose(terrainFile);
      throw 4;
    }

    switch (gzclose(terrainFile)) {
    case Z_OK:
      break;
    case Z_STREAM_ERROR:
    case Z_ERRNO:
    case Z_MEM_ERROR:
    case Z_BUF_ERROR:
    default:
      throw 5;
    }
  }

  std::vector<bool> mask() {
    std::vector<bool> mask;
    mask.assign(mMask, mMask + mMaskLength);
    return mask;
  }

  /* for a discussion on bitflags see
     <http://www.dylanleigh.net/notes/c-cpp-tricks.html#Using_"Bitflags"> */
  inline bool hasChildren() {
    return mChildren;
  }
  inline bool hasChildSW() {
    return ((mChildren & TC_SW) == TC_SW);
  }
  inline bool hasChildSE() {
    return ((mChildren & TC_SE) == TC_SE);
  }
  inline bool hasChildNW() {
    return ((mChildren & TC_NW) == TC_NW);
  }
  inline bool hasChildNE() {
    return ((mChildren & TC_NE) == TC_NE);
  }

  inline void setChildSW(bool on = true) {
    if (on) {
      mChildren |= TC_SW;
    } else {
      mChildren &= ~TC_SW;
    }
  }
  inline void setChildSE(bool on = true) {
    if (on) {
      mChildren |= TC_SE;
    } else {
      mChildren &= ~TC_SE;
    }
  }
  inline void setChildNW(bool on = true) {
    if (on) {
      mChildren |= TC_NW;
    } else {
      mChildren &= ~TC_NW;
    }
  }
  inline void setChildNE(bool on = true) {
    if (on) {
      mChildren |= TC_NE;
    } else {
      mChildren &= ~TC_NE;
    }
  }

  inline void setAllChildren(bool on = true) {
    if (on) {
      mChildren = TC_SW | TC_SE | TC_NW | TC_NE;
    } else {
      mChildren = 0;
    }
  }

  inline void setIsWater() {
    mMask[0] = 1;
    mMaskLength = 1;
  }
  inline bool isWater() {
    return mMaskLength == 1 && (bool) mMask[0];
  }

  inline void setIsLand() {
    mMask[0] = 0;
    mMaskLength = 1;
  }
  inline bool isLand() {
    return mMaskLength == 1 && ! (bool) mMask[0];
  }
  inline bool hasWaterMask() {
    return mMaskLength == MASK_SIZE;
  }

  GDALDatasetH heightsToRaster(double minx, double miny, double maxx, double maxy);

  std::vector<short int> mHeights;

private:
  char mChildren;
  char mMask[MASK_SIZE];
  size_t mMaskLength;
};


GDALDatasetH TerrainTile::heightsToRaster(double minx, double miny, double maxx, double maxy) {
  double resolution = (maxx - minx) / 65;
  double adfGeoTransform[6] = { minx, resolution, 0, maxy, 0, -resolution };

  OGRSpatialReference oSRS;
  oSRS.importFromEPSG(4326);

  GDALDriverH hDriver = GDALGetDriverByName( "MEM" );
  GDALDatasetH hDstDS;
  GDALRasterBandH hBand;

  char *pszDstWKT = NULL;
  oSRS.exportToWkt( &pszDstWKT );

  hDstDS = GDALCreate(hDriver, "", 65, 65, 1, GDT_Int16, NULL );
  GDALSetProjection( hDstDS, pszDstWKT );
  CPLFree( pszDstWKT );
  GDALSetGeoTransform( hDstDS, adfGeoTransform );

  hBand = GDALGetRasterBand( hDstDS, 1 );
  GDALRasterIO( hBand, GF_Write, 0, 0, 65, 65,
                mHeights.data(), 65, 65, GDT_Int16, 0, 0 );

  return hDstDS;
}
