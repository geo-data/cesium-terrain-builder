#include <stdio.h>
#include "src/TerrainTile.hpp"

int main() {
  TerrainTile terrain;
  FILE *terrainIn = fopen("0.terrain", "rb");

  try {
    terrain = TerrainTile(terrainIn);
  } catch (int e) {
    switch (e) {
    case 1:
      printf("Failed to read child tiles byte\n");
      return 1;
    case 2:
      printf("Failed to read water mask\n");
      return 2;
    default:
      printf("Unknown error: %d\n", e);
      return 3;
    }

    fclose(terrainIn);
    return e;
  }
  fclose(terrainIn);

  if (terrain.hasChildSW()) {
    printf("Has a SW child\n");
  }
  if (terrain.hasChildSE()) {
    printf("Has a SE child\n");
  }
  if (terrain.hasChildNW()) {
    printf("Has a NW child\n");
  }
  if (terrain.hasChildNE()) {
    printf("Has a NE child\n");
  }
  
  terrain.print();

  FILE *terrainOut = fopen("out.terrain","wb");
  terrain.writeFile(terrainOut);
  fclose(terrainOut);
  
  return 0;
}
