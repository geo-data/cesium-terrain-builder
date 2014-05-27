#include <stdio.h>

#define CHILD_SW 1  /* 2^0, bit 0 */
#define CHILD_SE 2  /* 2^1, bit 1 */
#define CHILD_NW 4  /* 2^2, bit 2 */
#define CHILD_NE 8  /* 2^3, bit 3 */

#define TILE_SIZE 65 * 65
#define MASK_SIZE 256 * 256

struct Terrain {
  short int heights[TILE_SIZE];
  char children;
  char mask[MASK_SIZE];
};
typedef struct Terrain TERRAIN;

short int terrain_read(TERRAIN *terrain, const char * file) {
  unsigned char bytes[2];
  FILE *fp = fopen(file,"rb");
  int count = 0;

  while ( count < TILE_SIZE && fread(bytes, 2, 1, fp) != 0) {
    /* adapted from
       <http://stackoverflow.com/questions/13001183/how-to-read-little-endian-integers-from-file-in-c> */
    
    terrain->heights[count++] = bytes[0] | (bytes[1]<<8);
  }
  
  if ( fread(&(terrain->children), 1, 1, fp) != 1) {
    fclose(fp);
    return 1;
  }

  if ( fread(terrain->mask, 1, MASK_SIZE, fp) != MASK_SIZE ) {
    fclose(fp);
    return 2;
  }

  fclose(fp);
  return 0;
}

short int terrain_write(TERRAIN *terrain, const char * file) {
  FILE *fp = fopen(file,"wb");
  fwrite(&(terrain->heights), TILE_SIZE * 2, 1, fp);
  fwrite(&terrain->children, 1, 1, fp);
  fwrite(terrain->mask, MASK_SIZE, 1, fp);
  fclose(fp);
  return 0;
}

int main() {
  TERRAIN terrain;
  int count = 0;

  switch (terrain_read(&terrain, "0.terrain")) {
  case 1:
    printf("Failed to read child tiles byte\n");
    return 1;
  case 2:
    printf("Failed to read water mask\n");
    return 2;
  default:
    break;
  }

  /* for a discussion on bitflags see
     <http://www.dylanleigh.net/notes/c-cpp-tricks.html#Using_"Bitflags"> */
  if ((terrain.children & CHILD_SW) == CHILD_SW) {
    printf("Has a SW child\n");
  }
  if ((terrain.children & CHILD_SE) == CHILD_SE) {
    printf("Has a SE child\n");
  }
  if ((terrain.children & CHILD_NW) == CHILD_NW) {
    printf("Has a NW child\n");
  }
  if ((terrain.children & CHILD_NE) == CHILD_NE) {
    printf("Has a NE child\n");
  }
  
  for (count = 0; count < TILE_SIZE; count++) {
    printf("height: %d\n", terrain.heights[count]);
  }

  for (count = 0; count < MASK_SIZE; count++) {
    printf("mask: %d\n", terrain.mask[count] & 0xFF);
  }

  terrain_write(&terrain, "out.terrain");
  
  return 0;
}
