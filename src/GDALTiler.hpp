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
  createRasterTile(const terrain::TileCoordinate &coord) const;

  terrain::TerrainTile
  createTerrainTile(const terrain::TileCoordinate &coord) const;

  inline i_zoom maxZoomLevel() const {
    return mProfile.zoomForResolution(resolution());
  }

  inline terrain::TileCoordinate
  lowerLeftTile(i_zoom zoom) const {
    return mProfile.latLonToTile(mBounds.getLowerLeft(), zoom);
  }

  inline terrain::TileCoordinate
  upperRightTile(i_zoom zoom) const {
    return mProfile.latLonToTile(mBounds.getUpperRight(), zoom);
  }

  inline terrain::TileBounds
  tileBoundsForZoom(i_zoom zoom) const {
    terrain::TileCoordinate ll = mProfile.latLonToTile(mBounds.getLowerLeft(), zoom),
      ur = mProfile.latLonToTile(mBounds.getUpperRight(), zoom);

    return TileBounds(ll, ur);
  }

  inline double resolution() const {
    return mResolution;
  }

  inline GDALDataset * dataset() const {
    return poDataset;
  }

  inline const terrain::GlobalGeodetic &
  profile() const {
    return mProfile;
  }

  inline const terrain::LatLonBounds &
  bounds() const {
    return const_cast<const terrain::LatLonBounds &>(mBounds);
  }

protected:
  void closeDataset();

  inline terrain::LatLonBounds
  terrainTileBounds(const terrain::TileCoordinate &coord,
                    double& resolution) const {
    i_tile lTileSize = mProfile.tileSize() - 1;
    terrain::LatLonBounds tile = mProfile.tileBounds(coord);
    resolution = (tile.getMaxX() - tile.getMinX()) / lTileSize;
    tile.setMinX(tile.getMinX() - resolution);
    tile.setMaxY(tile.getMaxY() + resolution);

    return tile;
  }

private:
  terrain::GlobalGeodetic mProfile;
  GDALDataset *poDataset;
  terrain::LatLonBounds mBounds;
  double mResolution;
};

#endif /* GDALTILER_HPP */
