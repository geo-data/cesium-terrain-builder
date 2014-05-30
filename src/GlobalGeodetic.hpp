/* 
   Based on gdal2tiles.py
*/

#include <cmath>

class GlobalGeodetic {
public:
  GlobalGeodetic(int tileSize = 65):
    mTileSize(tileSize),
    mInitialResolution(180.0 / tileSize)
  { }

  inline double const resolution(short int zoom) {
    return mInitialResolution / pow(2, zoom);
  }

  inline short int const zoomForResolution(double resolution) {
    return (short int) ceil(log2(mInitialResolution) - log2(resolution));
  }

  inline void const latLonToPixels(double lat, double lon, short int zoom,
                                   int& px, int& py) {
    double res = resolution(zoom);
    px = (180 + lat) / res;
    py = (90 + lon) / res;
    return;
  }

  inline void const pixelsToTile(int px, int py,
                                 int& tx, int& ty) {
    tx = (int) ceil(px / mTileSize);
    ty = (int) ceil(py / mTileSize);
    return;
  }

  inline void const latLonToTile(double lat, double lon, short int zoom,
                                 int& tx, int& ty) {
    int px, py;
    latLonToPixels(lat, lon, zoom, px, py);
    pixelsToTile(px, py, tx, ty);
    return;
  }

  inline void const tileBounds(int tx, int ty, short int zoom,
                               double& minx, double& miny, double& maxx, double& maxy) {
    double res = resolution(zoom);
    minx = tx * mTileSize * res - 180;
    miny = ty * mTileSize * res - 90;
    maxx = (tx + 1) * mTileSize * res - 180;
    maxy = (ty + 1) * mTileSize * res - 90;
  }

  inline void const terrainTileBounds(int tx, int ty, short int zoom,
                                      double& resolution,
                                      double& minx, double& miny, double& maxx, double& maxy) {
    int lTileSize = tileSize() - 1;

    tileBounds(tx, ty, zoom, minx, miny, maxx, maxy);
    resolution = (maxx - minx) / lTileSize;
    minx = minx - resolution;
    maxy = maxy + resolution;
  }

  inline int tileSize() {
    return mTileSize;
  }
  
private:
  int mTileSize;
  double mInitialResolution;
};

