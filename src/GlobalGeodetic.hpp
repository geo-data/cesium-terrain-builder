/* 
   Based on gdal2tiles.py
*/

#include <cmath>

class GlobalGeodetic {
public:
  GlobalGeodetic(int tileSize = 65):
    tileSize(tileSize),
    initialResolution(180.0 / tileSize)
  { }

  inline double const resolution(short int zoom) {
    return initialResolution / pow(2, zoom);
  }

  inline short int const zoomForResolution(double resolution) {
    return (short int) ceil(log2(initialResolution) - log2(resolution));
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
    tx = (int) ceil(px / tileSize);
    ty = (int) ceil(py / tileSize);
    return;
  }

  inline void const latLonToTile(double lat, double lon, short int zoom,
                                 int& tx, int& ty) {
    int px, py;
    latLonToPixels(lat, lon, zoom, px, py);
    pixelsToTile(px, py, tx, ty);
    return;
  }

  inline void const tileBounds(int tx, int ty, short int zoom, double& minx,
                               double& miny, double& maxx, double& maxy) {
    double res = resolution(zoom);
    minx = tx * tileSize * res - 180;
    miny = ty * tileSize * res - 90;
    maxx = (tx + 1) * tileSize * res - 180;
    maxy = (ty + 1) * tileSize * res - 90;
  }
  
private:
  int tileSize;
  double initialResolution;
};

