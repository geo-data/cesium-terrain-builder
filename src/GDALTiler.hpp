#include <cmath>                // for `abs()`

#include "gdal_priv.h"
#include "ogr_spatialref.h"
#include "gdalwarper.h"

#include "GlobalGeodetic.hpp"
#include "TerrainTile.hpp"
#include "Bounds.hpp"

class GDALTiler {
public:
  GDALTiler(GDALDataset *poDataset);
  ~GDALTiler();

  GDALDatasetH createRasterTile(short int zoom, int tx, int ty);
  TerrainTile *createTerrainTile(short int zoom, int tx, int ty);

  inline short int maxZoomLevel() {
    return mProfile.zoomForResolution(resolution());
  }

  inline void lowerLeftTile(short int zoom, int &tx, int &ty) {
    mProfile.latLonToTile(mBounds.getMinX(), mBounds.getMinY(), zoom, tx, ty);
  }

  inline void upperRightTile(short int zoom, int &tx, int &ty) {
    mProfile.latLonToTile(mBounds.getMaxX(), mBounds.getMaxY(), zoom, tx, ty);
  }

  inline double resolution() {
    return mResolution;
  }

  inline GDALDataset * dataset() {
    return poDataset;
  }

  inline const GlobalGeodetic & profile() {
    return mProfile;
  }

  inline Bounds & bounds() {
    return mBounds;
  }

private:
  GlobalGeodetic mProfile;
  GDALDataset *poDataset;
  Bounds mBounds;
  double mResolution;
};

GDALTiler::GDALTiler(GDALDataset *poDataset):
  poDataset(poDataset)
{
  // Need to catch NULL dataset here

  // Get the bounds of the dataset
  double adfGeoTransform[6];

  if( poDataset->GetGeoTransform( adfGeoTransform ) == CE_None ) {
    mBounds = Bounds(adfGeoTransform[0],
                     adfGeoTransform[3] + (poDataset->GetRasterYSize() * adfGeoTransform[5]),
                     adfGeoTransform[0] + (poDataset->GetRasterXSize() * adfGeoTransform[1]),
                     adfGeoTransform[3]);

    mResolution = std::abs(adfGeoTransform[1]);
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

  if (heightsBand->RasterIO(GF_Read, 0, 0, 65, 65,
                            (void *) heights, 65, 65, GDT_Float32,
                            0, 0) != CE_None) {
    GDALClose((GDALDatasetH) rasterTile);
    delete terrainTile;
    delete [] heights;
    return NULL;
  }

  // TODO: try doing this using a VRT derived band:
  // (http://www.gdal.org/gdal_vrttut.html)
  for (int i = 0; i < TILE_SIZE; i++) {
    terrainTile->mHeights[i] = (short int) ((heights[i] + 1000) * 5);
  }
  delete [] heights;

  GDALClose((GDALDatasetH) rasterTile);

  if (zoom != maxZoomLevel()) {
    double minLon, minLat, maxLon, maxLat;
    mProfile.tileBounds(tx, ty, zoom, minLon, minLat, maxLon, maxLat);
    Bounds *tileBounds = new Bounds(minLon, minLat, maxLon, maxLat);

    if (! (bounds().overlaps(tileBounds))) {
      terrainTile->setAllChildren(false);
    } else {
      if (bounds().overlaps(tileBounds->getSW())) {
        terrainTile->setChildSW();
      }
      if (bounds().overlaps(tileBounds->getNW())) {
        terrainTile->setChildNW();
      }
      if (bounds().overlaps(tileBounds->getNE())) {
        terrainTile->setChildNE();
      }
      if (bounds().overlaps(tileBounds->getSE())) {
        terrainTile->setChildSE();
      }
    }
  }

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

  GDALDatasetH hSrcDS = (GDALDatasetH) dataset();
  GDALDatasetH hDstDS;

  char *pszDstWKT = NULL;
  oSRS.exportToWkt( &pszDstWKT );

  // set the warp options
  GDALWarpOptions *psWarpOptions = GDALCreateWarpOptions();
  //psWarpOptions->eResampleAlg = eResampleAlg;
  psWarpOptions->hSrcDS = hSrcDS;
  psWarpOptions->nBandCount = 1;
  psWarpOptions->panSrcBands =
    (int *) CPLMalloc(sizeof(int) * psWarpOptions->nBandCount );
  psWarpOptions->panSrcBands[0] = 1;
  psWarpOptions->panDstBands =
    (int *) CPLMalloc(sizeof(int) * psWarpOptions->nBandCount );
  psWarpOptions->panDstBands[0] = 1;

  // create the transformer
  // TODO: if src.srs == dst.srs then try setting both srs WKT to NULL
  psWarpOptions->pfnTransformer = GDALGenImgProjTransform;
  psWarpOptions->pTransformerArg =
        GDALCreateGenImgProjTransformer( hSrcDS, GDALGetProjectionRef(hSrcDS),
                                         NULL, pszDstWKT,
                                         FALSE, 0.0, 1 );

  if( psWarpOptions->pTransformerArg == NULL ) {
    GDALDestroyWarpOptions( psWarpOptions );
    return NULL;
  }

  GDALSetGenImgProjTransformerDstGeoTransform( psWarpOptions->pTransformerArg, adfGeoTransform );

  hDstDS = GDALCreateWarpedVRT(hSrcDS, mProfile.tileSize(), mProfile.tileSize(), adfGeoTransform, psWarpOptions);

  GDALDestroyWarpOptions( psWarpOptions );

  GDALSetProjection( hDstDS, pszDstWKT );

  mProfile.tileBounds(tx, ty, zoom, minLon, minLat, maxLon, maxLat);
  resolution = mProfile.resolution(zoom);
  adfGeoTransform[0] = minLon;
  adfGeoTransform[1] = resolution;
  adfGeoTransform[2] = 0;
  adfGeoTransform[3] = maxLat;
  adfGeoTransform[4] = 0;
  adfGeoTransform[5] = -resolution;
  GDALSetGeoTransform( hDstDS, adfGeoTransform );

  // If uncommenting the following line for debug purposes, you must also `#include "vrtdataset.h"`
  //std::cout << "VRT: " << CPLSerializeXMLTree(((VRTWarpedDataset *) hDstDS)->SerializeToXML(NULL)) << std::endl;

  return hDstDS;
}
