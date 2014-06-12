#include "TerrainException.hpp"
#include "GDALTiler.hpp"

GDALTiler::GDALTiler(GDALDataset *poDataset):
  poDataset(poDataset)
{
  if (poDataset != NULL) {
    poDataset->Reference();     // increase the refcount of the dataset

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
}

GDALTiler::GDALTiler(const GDALTiler &other):
  mProfile(other.mProfile),
  poDataset(other.poDataset),
  mBounds(other.mBounds),
  mResolution(other.mResolution)
{
  if (poDataset != NULL) {
    poDataset->Reference();     // increase the refcount of the dataset
  }
}

GDALTiler::GDALTiler(GDALTiler &other):
  mProfile(other.mProfile),
  poDataset(other.poDataset),
  mBounds(other.mBounds),
  mResolution(other.mResolution)
{
  if (poDataset != NULL) {
    poDataset->Reference();     // increase the refcount of the dataset
  }
}

GDALTiler & GDALTiler::operator=(const GDALTiler &other) {
  mProfile = other.mProfile;
  closeDataset();
  poDataset = other.poDataset;
  mBounds = other.mBounds;
  mResolution = other.mResolution;

  return *this;
}

GDALTiler::~GDALTiler() {
  closeDataset();
}

TerrainTile GDALTiler::createTerrainTile(const TileCoordinate &coord) const {
  TerrainTile terrainTile(coord);
  GDALDataset *rasterTile = (GDALDataset *) createRasterTile(coord);
  GDALRasterBand *heightsBand = rasterTile->GetRasterBand(1);

  float heights[TILE_SIZE];

  if (heightsBand->RasterIO(GF_Read, 0, 0, 65, 65,
                            (void *) heights, 65, 65, GDT_Float32,
                            0, 0) != CE_None) {
    GDALClose(rasterTile);
    throw TerrainException("Could not read heights from raster");
  }

  // TODO: try doing this using a VRT derived band:
  // (http://www.gdal.org/gdal_vrttut.html)
  for (unsigned short int i = 0; i < TILE_SIZE; i++) {
    terrainTile.mHeights[i] = (short int) ((heights[i] + 1000) * 5);
  }

  GDALClose(rasterTile);

  if (coord.zoom != maxZoomLevel()) {
    Bounds tileBounds = mProfile.tileBounds(coord);

    if (! (bounds().overlaps(tileBounds))) {
      terrainTile.setAllChildren(false);
    } else {
      if (bounds().overlaps(tileBounds.getSW())) {
        terrainTile.setChildSW();
      }
      if (bounds().overlaps(tileBounds.getNW())) {
        terrainTile.setChildNW();
      }
      if (bounds().overlaps(tileBounds.getNE())) {
        terrainTile.setChildNE();
      }
      if (bounds().overlaps(tileBounds.getSE())) {
        terrainTile.setChildSE();
      }
    }
  }

  return terrainTile;
}

GDALDatasetH GDALTiler::createRasterTile(const TileCoordinate &coord) const {
  if (poDataset == NULL) {
    throw TerrainException("No GDAL dataset is set");
  }

  double resolution;
  Bounds tileBounds = terrainTileBounds(coord, resolution);

  double adfGeoTransform[6];
  adfGeoTransform[0] = tileBounds.getMinX(); // min longitude
  adfGeoTransform[1] = resolution;
  adfGeoTransform[2] = 0;
  adfGeoTransform[3] = tileBounds.getMaxY(); // max latitude
  adfGeoTransform[4] = 0;
  adfGeoTransform[5] = -resolution;

  OGRSpatialReference oSRS;
  if (oSRS.importFromEPSG(4326) != OGRERR_NONE) {
    throw TerrainException("Could not create EPSG:4326 spatial reference");
  }

  GDALDatasetH hSrcDS = (GDALDatasetH) dataset();
  GDALDatasetH hDstDS;

  char *pszDstWKT = NULL;
  const char *pszSrcWKT = GDALGetProjectionRef(hSrcDS);
  const OGRSpatialReference dstSRS(pszSrcWKT);

  if (!oSRS.IsSame(&dstSRS)) {
    if (oSRS.exportToWkt( &pszDstWKT ) != OGRERR_NONE) {
      CPLFree( pszDstWKT );
      throw TerrainException("Could not create EPSG:4326 WKT string");
    }
  } else {
    pszSrcWKT = NULL;    // we don't need to perform any reprojections
  }

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

  psWarpOptions->pfnTransformer = GDALGenImgProjTransform;
  psWarpOptions->pTransformerArg =
        GDALCreateGenImgProjTransformer( hSrcDS, pszSrcWKT,
                                         NULL, pszDstWKT,
                                         FALSE, 0.0, 1 );

  if( psWarpOptions->pTransformerArg == NULL ) {
    GDALDestroyWarpOptions( psWarpOptions );
    CPLFree( pszDstWKT );
    throw TerrainException("Could not create image to image transformer");
  }

  GDALSetGenImgProjTransformerDstGeoTransform( psWarpOptions->pTransformerArg, adfGeoTransform );

  hDstDS = GDALCreateWarpedVRT(hSrcDS, mProfile.tileSize(), mProfile.tileSize(), adfGeoTransform, psWarpOptions);
  GDALDestroyWarpOptions( psWarpOptions );

  if (hDstDS == NULL) {
    CPLFree( pszDstWKT );
    throw TerrainException("Could not create warped VRT");
  }

  if (GDALSetProjection( hDstDS, pszDstWKT ) != CE_None) {
    CPLFree( pszDstWKT );
    throw TerrainException("Could not set projection on VRT");
  }
  CPLFree( pszDstWKT );

  tileBounds = mProfile.tileBounds(coord);
  resolution = mProfile.resolution(coord.zoom);
  adfGeoTransform[0] = tileBounds.getMinX(); // min longitude
  adfGeoTransform[1] = resolution;
  adfGeoTransform[2] = 0;
  adfGeoTransform[3] = tileBounds.getMaxY(); // max latitude
  adfGeoTransform[4] = 0;
  adfGeoTransform[5] = -resolution;

  if (GDALSetGeoTransform( hDstDS, adfGeoTransform ) != CE_None) {
      throw TerrainException("Could not set projection on VRT");
  }

  // If uncommenting the following line for debug purposes, you must also `#include "vrtdataset.h"`
  //std::cout << "VRT: " << CPLSerializeXMLTree(((VRTWarpedDataset *) hDstDS)->SerializeToXML(NULL)) << std::endl;

  return hDstDS;
}

void GDALTiler::closeDataset() {
  if (poDataset != NULL) {
    poDataset->Dereference();

    if (poDataset->GetRefCount() < 1) {
      GDALClose(poDataset);
    }

    poDataset = NULL;
  }
}
