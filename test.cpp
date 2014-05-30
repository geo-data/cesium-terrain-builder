#include <iostream>             // for cout and cin
#include "src/GlobalGeodetic.hpp"
#include "src/TerrainTile.hpp"

#include "gdal_priv.h"
#include "ogr_spatialref.h"
#include "gdalwarper.h"

using namespace std;

class GDALTiler {
public:
  GDALTiler(GDALDataset *poDataset);
  ~GDALTiler();

  GDALDatasetH createRasterTile(short int zoom, int tx, int ty);
  TerrainTile *createTerrainTile(short int zoom, int tx, int ty);

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
  GlobalGeodetic mProfile;
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

TerrainTile *GDALTiler::createTerrainTile(short int zoom, int tx, int ty) {
  TerrainTile *terrainTile = new TerrainTile();
  GDALDataset *rasterTile = (GDALDataset *) createRasterTile(zoom, tx, ty);
  GDALRasterBand *heightsBand = rasterTile->GetRasterBand(1);

  float *heights = new float[TILE_SIZE];

  heightsBand->RasterIO(GF_Write, 0, 0, 65, 1,
                        (void *) heights, 65, 1, GDT_Float32,
                        0, 0);

  for(int i = 0; i < TILE_SIZE; i++) {
    terrainTile->mHeights[i] = (short int) ((heights[i] + 1000) * 5);
  }
  delete [] heights;

  GDALClose((GDALDatasetH) rasterTile);

  return terrainTile;
}

GDALDatasetH GDALTiler::createRasterTile(short int zoom, int tx, int ty) {
  double resolution, minLon, minLat, maxLon, maxLat;
  mProfile.terrainTileBounds(tx, ty, zoom, resolution, minLon, minLat, maxLon, maxLat);

  double adfGeoTransform[6];
  adfGeoTransform[0] = minLon;
  adfGeoTransform[1] = resolution;
  adfGeoTransform[2] = 0;
  adfGeoTransform[3] = maxLat;
  adfGeoTransform[4] = 0;
  adfGeoTransform[5] = -resolution;

  OGRSpatialReference oSRS;
  oSRS.importFromEPSG(4326);

  GDALDriverH hDriver = GDALGetDriverByName( "MEM" );
  GDALDatasetH hSrcDS = (GDALDatasetH) dataset();
  GDALDataType eDT = GDALGetRasterDataType(GDALGetRasterBand(hSrcDS,1));
  GDALDatasetH hDstDS;

  char *pszDstWKT = NULL;
  oSRS.exportToWkt( &pszDstWKT );

  hDstDS = GDALCreate(hDriver, "", mProfile.tileSize(), mProfile.tileSize(), 1, eDT, NULL );
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

  //psWarpOptions->pfnProgress = GDALTermProgress;
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

  mProfile.tileBounds(tx, ty, zoom, minLon, minLat, maxLon, maxLat);
  resolution = mProfile.resolution(zoom);
  adfGeoTransform[0] = minLon;
  adfGeoTransform[1] = resolution;
  adfGeoTransform[2] = 0;
  adfGeoTransform[3] = maxLat;
  adfGeoTransform[4] = 0;
  adfGeoTransform[5] = -resolution;
  GDALSetGeoTransform( hDstDS, adfGeoTransform );

  return hDstDS;
}

int main() {
  GDALAllRegister();

  GDALDataset  *poDataset = (GDALDataset *) GDALOpen("./lidar-2007-filled-cut.tif", GA_ReadOnly);
  GDALTiler tiler(poDataset);

  int tx = 8146, ty = 6409;
  short int zoom = 13;

  GDALDatasetH hTileDS = tiler.createRasterTile(zoom, tx, ty);
  GDALDatasetH hDstDS;
  GDALDriverH hDriver = GDALGetDriverByName("GTiff");

  hDstDS = GDALCreateCopy( hDriver, "13-8146-6409.gdal.tif", hTileDS, FALSE,
                           NULL, NULL, NULL );

  /* Once we're done, close properly the dataset */
  if( hDstDS != NULL )
    GDALClose( hDstDS );
  GDALClose( hTileDS );

  TerrainTile *terrainTile = tiler.createTerrainTile(zoom, tx, ty);
  FILE *terrainOut = fopen("13-8146-6409.terrain", "wb");
  terrainTile->writeFile(terrainOut);
  delete terrainTile;
  fclose(terrainOut);

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
