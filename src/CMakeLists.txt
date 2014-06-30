# We need GDAL
find_package(GDAL)
if(NOT GDAL_FOUND)
  message(FATAL_ERROR "The GDAL library cannot be found on the sytem")
endif()
include_directories(${GDAL_INCLUDE_DIRS})

# Ensure we have a unified GDAL: `GDALOpenEx` is only present after
# unification.
include(CheckLibraryExists)
check_library_exists(gdal GDALOpenEx "gdal.h" HAVE_UNIFIED_GDAL)
if(NOT HAVE_UNIFIED_GDAL)
    message(FATAL_ERROR "The GDAL version must be one that implements RFC 46 (GDAL/OGR unification) i.e. >= 2.0.0")
endif(NOT HAVE_UNIFIED_GDAL)

# We need zlib
find_package(ZLIB)
if(NOT ZLIB_FOUND)
  message(FATAL_ERROR "The zlib library cannot be found on the sytem")
endif()
include_directories(${ZLIB_INCLUDE_DIRS})
  
add_library(terrain SHARED
    GDALTiler.cpp
    TerrainTiler.cpp
    TerrainTile.cpp
    RasterIterator.cpp
    TerrainIterator.cpp
    GlobalMercator.cpp
    GlobalGeodetic.cpp)
target_link_libraries(terrain ${GDAL_LIBRARIES} ${ZLIB_LIBRARIES})

# Install libterrain
set(HEADERS
    Bounds.hpp
    Coordinate.hpp
    CRSBoundsIterator.hpp
    GDALTiler.hpp
    GlobalGeodetic.hpp
    GlobalMercator.hpp
    Grid.hpp
    GridIterator.hpp
    RasterIterator.hpp
    TerrainException.hpp
    TerrainIterator.hpp
    TerrainTile.hpp
    TerrainTiler.hpp
    TileCoordinate.hpp
    TileCoordinateIterator.hpp
    TilerIterator.hpp
    types.hpp)
install(FILES ${HEADERS} DESTINATION include/terrain)
install(FILES terrain.hpp DESTINATION include)

install(TARGETS terrain DESTINATION lib)
