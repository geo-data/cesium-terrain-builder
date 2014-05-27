#include <iostream>             // for cout and cin
#include <fstream>
#include <sstream>
#include "src/GlobalGeodetic.hpp"

#include "gdal_priv.h"

using namespace std;

class GDALTiler {
public:
  GDALTiler(GDALDataset *poDataset);
  ~GDALTiler();

  //private:
  GDALDataset *poDataset;
  double bounds[4];             // minx, miny, maxx, maxy
  double resolution;
};

GDALTiler::GDALTiler(GDALDataset *poDataset):
  poDataset(poDataset)  
{
  // Need to catch NULL dataset here

  // Get the bounds of the dataset
  double adfGeoTransform[6];

  if( poDataset->GetGeoTransform( adfGeoTransform ) == CE_None ) {
    bounds[0] = adfGeoTransform[0];
    bounds[1] = adfGeoTransform[3] + (poDataset->GetRasterYSize() * adfGeoTransform[5]);
    bounds[2] = adfGeoTransform[0] + (poDataset->GetRasterXSize() * adfGeoTransform[1]);
    bounds[3] = adfGeoTransform[3];

    resolution = abs(adfGeoTransform[1]);
  }
}
  
GDALTiler::~GDALTiler() {
  GDALClose(poDataset);
}

static void printCoord(ofstream& stream, double x, double y) {
  stream << "[" << x << ", " << y << "]";
}

int main() {
  GlobalGeodetic Profile;
  /*double resolution = 0.000014677763277;
  for (i = 0; i < 19; i++) {
    cout << "zoom level: " << i << " resolution: " << Profile.resolution(i) << endl;
  }
  cout << "zoom level for " << resolution << " is " << Profile.zoomForResolution(resolution) << endl;*/
  
  GDALAllRegister();
  GDALDataset  *poDataset = (GDALDataset *) GDALOpen("./lidar-2007-filled-cut.tif", GA_ReadOnly);
  GDALTiler tiler(poDataset);

  ofstream geojson;
  for (short int zoom = Profile.zoomForResolution(tiler.resolution); zoom >= 0; zoom--) {
    int minx, miny, maxx, maxy;
    Profile.latLonToTile(tiler.bounds[0], tiler.bounds[1], zoom, minx, miny);
    Profile.latLonToTile(tiler.bounds[2], tiler.bounds[3], zoom, maxx, maxy);
    //cout << "ll tile: " << minx << "," << miny << "; ur tile: " << maxx << "," << maxy << endl;

    string filename = static_cast<ostringstream*>( &(ostringstream() << zoom << ".geojson") )->str();
    geojson.open(filename.c_str());
    
    geojson << "{ \"type\": \"FeatureCollection\", \"features\": [" << endl;

    int tx, ty;
    double minLon, minLat, maxLon, maxLat;
    for (tx = minx; tx <= maxx; tx++) {
      for (ty = miny; ty <= maxy; ty++) {
        Profile.tileBounds(tx, ty, zoom, minLon, minLat, maxLon, maxLat);

        geojson << "{ \"type\": \"Feature\", \"geometry\": { \"type\": \"Polygon\", \"coordinates\": [[";
        printCoord(geojson, minLon, minLat);
        geojson << ", ";
        printCoord(geojson, maxLon, minLat);
        geojson << ", ";
        printCoord(geojson, maxLon, maxLat);
        geojson << ", ";
        printCoord(geojson, minLon, maxLat);
        geojson << ", ";
        printCoord(geojson, minLon, minLat);
        geojson << "]]}, \"properties\": {\"tx\": " << tx << ", \"ty\": " << ty << "}}";
        if (ty != maxy)
          geojson << "," << endl;
      }
      if (tx != maxx)
        geojson << "," << endl;

    }

    geojson << "]}" << endl;
    geojson.close();
  }

  return 0;
}
