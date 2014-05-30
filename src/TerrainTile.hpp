#include <vector>

#include "gdal_priv.h"
#include "ogr_spatialref.h"

#define TILE_SIZE 65 * 65
#define MASK_SIZE 256 * 256

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

  std::vector<bool> mask() {
    std::vector<bool> mask;
    mask.assign(mMask, mMask + mMaskLength);
    return mask;
  }

  /* for a discussion on bitflags see
     <http://www.dylanleigh.net/notes/c-cpp-tricks.html#Using_"Bitflags"> */
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
  inline bool isMixed() {
    return mMaskLength > 1;
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
