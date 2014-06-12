#ifndef GDALTILER_HPP
#define GDALTILER_HPP

#include <cmath>                // for `abs()`

#include "gdal_priv.h"
#include "ogr_spatialref.h"
#include "gdalwarper.h"

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

  GDALDatasetH createRasterTile(short int zoom, int tx, int ty) const;
  TerrainTile *createTerrainTile(short int zoom, int tx, int ty) const;

  inline short int maxZoomLevel() const {
    return mProfile.zoomForResolution(resolution());
  }

  inline void lowerLeftTile(short int zoom, int &tx, int &ty) const {
    mProfile.latLonToTile(mBounds.getMinX(), mBounds.getMinY(), zoom, tx, ty);
  }

  inline void upperRightTile(short int zoom, int &tx, int &ty) const {
    mProfile.latLonToTile(mBounds.getMaxX(), mBounds.getMaxY(), zoom, tx, ty);
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
