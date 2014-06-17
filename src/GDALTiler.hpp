#ifndef GDALTILER_HPP
#define GDALTILER_HPP

#include <string>
#include <cmath>                // for `abs()`

#include "gdal_priv.h"
#include "ogr_spatialref.h"
#include "gdalwarper.h"

#include "TileCoordinate.hpp"
#include "GlobalGeodetic.hpp"
#include "TerrainTile.hpp"
#include "Bounds.hpp"

namespace terrain {
  class GDALTiler;
}

class terrain::GDALTiler {
public:
  GDALTiler(): poDataset(NULL) {}
  GDALTiler(GDALDataset *poDataset);
  GDALTiler(const GDALTiler &other);
  GDALTiler(GDALTiler &other);
  GDALTiler &operator=(const GDALTiler &other);

  ~GDALTiler();

  GDALDatasetH
  createRasterTile(const TileCoordinate &coord) const;

  TerrainTile
  createTerrainTile(const TileCoordinate &coord) const;

  inline i_zoom maxZoomLevel() const {
    return mProfile.zoomForResolution(resolution());
  }

  inline TileCoordinate
  lowerLeftTile(i_zoom zoom) const {
    return mProfile.latLonToTile(mBounds.getLowerLeft(), zoom);
  }

  inline TileCoordinate
  upperRightTile(i_zoom zoom) const {
    return mProfile.latLonToTile(mBounds.getUpperRight(), zoom);
  }

  inline TileBounds
  tileBoundsForZoom(i_zoom zoom) const {
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

  inline const GlobalGeodetic &
  profile() const {
    return mProfile;
  }

  inline const LatLonBounds &
  bounds() const {
    return const_cast<const LatLonBounds &>(mBounds);
  }

  inline bool requiresReprojection() const {
    return wgs84WKT.size() > 0;
  }

protected:
  void closeDataset();

  inline LatLonBounds
  terrainTileBounds(const TileCoordinate &coord,
                    double& resolution) const {
    i_tile lTileSize = mProfile.tileSize() - 1;
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
  std::string wgs84WKT;
};

#endif /* GDALTILER_HPP */
