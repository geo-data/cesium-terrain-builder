#include <stdio.h>
#include <iostream>
#include <vector>
#include "src/TerrainTile.hpp"

/*void terrain2tiff(TerrainTile &terrain, double minx, double miny, double maxx, double maxy) {
  double resolution = (maxx - minx) / 65;
  }*/

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

  std::vector<short int> heights = terrain.heights();
  std::vector<short int>::iterator heightsIterator;
  for(heightsIterator = heights.begin();
      heightsIterator != heights.end();
      heightsIterator++)
    {
      std::cout << "height: " << *heightsIterator << std::endl;
    }

  std::vector<bool> mask = terrain.mask();
  std::vector<bool>::iterator maskIterator;
  for(maskIterator = mask.begin();
      maskIterator != mask.end();
      maskIterator++)
    {
      std::cout << "mask: " << *maskIterator << std::endl;
    }

  FILE *terrainOut = fopen("out.terrain","wb");
  terrain.writeFile(terrainOut);
  fclose(terrainOut);

  return 0;
}
