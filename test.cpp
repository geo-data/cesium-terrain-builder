#include <iostream>             // for cout and cin
#include "src/GlobalGeodetic.hpp"

#include "gdal_priv.h"
#include "ogr_spatialref.h"
#include "gdalwarper.h"

using namespace std;

class GDALTiler {
public:
  GDALTiler(GDALDataset *poDataset);
  ~GDALTiler();

  inline double const minx() {
    return mBounds[0];
  }

  inline double const miny() {
    return mBounds[1];
  }

  inline double const maxx() {
    return mBounds[2];
  }

  inline double const maxy() {
    return mBounds[3];
  }

  inline double resolution() {
    return mResolution;
  }

  inline GDALDataset * dataset() {
    return poDataset;
  }

private:
  GDALDataset *poDataset;
  double mBounds[4];            // minx, miny, maxx, maxy
  double mResolution;
};

GDALTiler::GDALTiler(GDALDataset *poDataset):
  poDataset(poDataset)
{
  // Need to catch NULL dataset here

  // Get the bounds of the dataset
  double adfGeoTransform[6];

  if( poDataset->GetGeoTransform( adfGeoTransform ) == CE_None ) {
    mBounds[0] = adfGeoTransform[0];
    mBounds[1] = adfGeoTransform[3] + (poDataset->GetRasterYSize() * adfGeoTransform[5]);
    mBounds[2] = adfGeoTransform[0] + (poDataset->GetRasterXSize() * adfGeoTransform[1]);
    mBounds[3] = adfGeoTransform[3];

    mResolution = abs(adfGeoTransform[1]);
  }
}

GDALTiler::~GDALTiler() {
  GDALClose(poDataset);
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

  //int tx = 8145, ty = 6408;
  //int tx = 8146, ty = 6408;
  //int tx = 8145, ty = 6409;
  int tx = 8146, ty = 6409;
  short int zoom = 13;
  double resolution, minLon, minLat, maxLon, maxLat;
  Profile.terrainTileBounds(tx, ty, zoom, resolution, minLon, minLat, maxLon, maxLat);

  double adfGeoTransform[6];
  adfGeoTransform[0] = minLon;
  adfGeoTransform[1] = resolution;
  adfGeoTransform[2] = 0;
  adfGeoTransform[3] = maxLat;
  adfGeoTransform[4] = 0;
  adfGeoTransform[5] = -resolution;

  OGRSpatialReference oSRS;
  oSRS.importFromEPSG(4326);

  GDALDataType eDT = GDT_Int16;
  GDALDriverH hDriver = GDALGetDriverByName( "GTiff" );
  GDALDatasetH hSrcDS = (GDALDatasetH) tiler.dataset();
  GDALDatasetH hDstDS;

  char *pszDstWKT = NULL;
  oSRS.exportToWkt( &pszDstWKT );

  hDstDS = GDALCreate(hDriver, "13-8146-6409.gdal.tif", Profile.tileSize(), Profile.tileSize(), 1, eDT, NULL );
  GDALSetProjection( hDstDS, pszDstWKT );
  GDALSetGeoTransform( hDstDS, adfGeoTransform );

  GDALWarpOptions *psWarpOptions = GDALCreateWarpOptions();
  psWarpOptions->hSrcDS = hSrcDS;
  psWarpOptions->hDstDS = hDstDS;
  psWarpOptions->nBandCount = 1;
  psWarpOptions->panSrcBands =
    (int *) CPLMalloc(sizeof(int) * psWarpOptions->nBandCount );
  psWarpOptions->panSrcBands[0] = 1;
  psWarpOptions->panDstBands =
    (int *) CPLMalloc(sizeof(int) * psWarpOptions->nBandCount );
  psWarpOptions->panDstBands[0] = 1;

  psWarpOptions->pfnProgress = GDALTermProgress;
  psWarpOptions->pTransformerArg =
    GDALCreateGenImgProjTransformer( hSrcDS,
                                     GDALGetProjectionRef(hSrcDS),
                                     hDstDS,
                                     GDALGetProjectionRef(hDstDS),
                                     FALSE, 0.0, 1 );
  psWarpOptions->pfnTransformer = GDALGenImgProjTransform;

  GDALWarpOperation oOperation;

  oOperation.Initialize( psWarpOptions );
  oOperation.ChunkAndWarpImage( 0, 0,
                                GDALGetRasterXSize( hDstDS ),
                                GDALGetRasterYSize( hDstDS ) );

  GDALDestroyGenImgProjTransformer( psWarpOptions->pTransformerArg );
  GDALDestroyWarpOptions( psWarpOptions );

  Profile.tileBounds(tx, ty, zoom, minLon, minLat, maxLon, maxLat);
  resolution = Profile.resolution(zoom);
  adfGeoTransform[0] = minLon;
  adfGeoTransform[1] = resolution;
  adfGeoTransform[2] = 0;
  adfGeoTransform[3] = maxLat;
  adfGeoTransform[4] = 0;
  adfGeoTransform[5] = -resolution;
  GDALSetGeoTransform( hDstDS, adfGeoTransform );

  GDALClose( hDstDS );
  /*
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
  */

  return 0;
}
