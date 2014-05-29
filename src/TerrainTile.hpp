#include <stdio.h>
#include <vector>

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
  TerrainTile() {};

  TerrainTile(FILE *fp) {
    unsigned char bytes[2];
    int count = 0;

    while ( count < TILE_SIZE && fread(bytes, 2, 1, fp) != 0) {
      /* adapted from
         <http://stackoverflow.com/questions/13001183/how-to-read-little-endian-integers-from-file-in-c> */

      mHeights[count++] = bytes[0] | (bytes[1]<<8);
    }

    if ( fread(&(mChildren), 1, 1, fp) != 1) {
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
      fwrite(&mHeights, TILE_SIZE * 2, 1, fp);
      fwrite(&mChildren, 1, 1, fp);
      fwrite(mMask, mMaskLength, 1, fp);
  }

  std::vector<short int> heights() {
    std::vector<short int> heights;
    heights.assign(mHeights, mHeights + TILE_SIZE);
    return heights;
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

  inline bool isWater() {
    return mMaskLength == 1 && (bool) mMask[0];
  }
  inline bool isLand() {
    return mMaskLength == 1 && ! (bool) mMask[0];
  }
  inline bool isMixed() {
    return mMaskLength > 1;
  }

  short int mHeights[TILE_SIZE];
private:
  char mChildren;
  char mMask[MASK_SIZE];
  size_t mMaskLength;
};
