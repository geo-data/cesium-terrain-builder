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
  TerrainTile *createTerrainTile(const TileCoordinate &coord) const;

  inline unsigned short int maxZoomLevel() const {
    return mProfile.zoomForResolution(resolution());
  }

  inline TileCoordinate
  lowerLeftTile(unsigned short int zoom) const {
    return mProfile.latLonToTile(mBounds.getMinX(), mBounds.getMinY(), zoom);
  }

  inline TileCoordinate
  upperRightTile(unsigned short int zoom) const {
    return mProfile.latLonToTile(mBounds.getMaxX(), mBounds.getMaxY(), zoom);
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

  inline const Bounds & bounds() const {
    return const_cast<const Bounds &>(mBounds);
  }

protected:
  void closeDataset();

private:
  GlobalGeodetic mProfile;
  GDALDataset *poDataset;
  Bounds mBounds;
  double mResolution;
};

#endif /* GDALTILER_HPP */
