#ifndef GDALTILER_HPP
#define GDALTILER_HPP

#include <cmath>                // for `abs()`

#include "gdal_priv.h"
#include "ogr_spatialref.h"
#include "gdalwarper.h"

#include "TileCoordinate.hpp"
#include "GlobalGeodetic.hpp"
#include "TerrainTile.hpp"
#include "Bounds.hpp"

class GDALTiler {
public:
  GDALTiler(): poDataset(NULL) {}
  GDALTiler(GDALDataset *poDataset);
  GDALTiler(const GDALTiler &other);
  GDALTiler(GDALTiler &other);
  GDALTiler &operator=(const GDALTiler &other);

  ~GDALTiler();

  GDALDatasetH createRasterTile(const TileCoordinate &coord) const;
  TerrainTile createTerrainTile(const TileCoordinate &coord) const;

  inline unsigned short int maxZoomLevel() const {
    return mProfile.zoomForResolution(resolution());
  }

  inline TileCoordinate
  lowerLeftTile(unsigned short int zoom) const {
    return mProfile.latLonToTile(mBounds.getLowerLeft(), zoom);
  }

  inline TileCoordinate
  upperRightTile(unsigned short int zoom) const {
    return mProfile.latLonToTile(mBounds.getUpperRight(), zoom);
  }

  inline TileBounds
  tileBoundsForZoom(unsigned short int zoom) const {
    TileCoordinate ll = mProfile.latLonToTile(mBounds.getLowerLeft(), zoom),
      ur = mProfile.latLonToTile(mBounds.getUpperRight(), zoom);

    return TileBounds(ll, ur);
  }

  inline double resolution() const {
    return mResolution;
  }

  inline GDALDataset * dataset() const {
    return poDataset;
  }

  inline const GlobalGeodetic & profile() const {
    return mProfile;
  }

  inline const LatLonBounds & bounds() const {
    return const_cast<const LatLonBounds &>(mBounds);
  }

protected:
  void closeDataset();

  inline LatLonBounds
  terrainTileBounds(const TileCoordinate &coord,
                    double& resolution) const {
    unsigned int lTileSize = mProfile.tileSize() - 1;
    LatLonBounds tile = mProfile.tileBounds(coord);
    resolution = (tile.getMaxX() - tile.getMinX()) / lTileSize;
    tile.setMinX(tile.getMinX() - resolution);
    tile.setMaxY(tile.getMaxY() + resolution);

    return tile;
  }

private:
  GlobalGeodetic mProfile;
  GDALDataset *poDataset;
  LatLonBounds mBounds;
  double mResolution;
};

#endif /* GDALTILER_HPP */
