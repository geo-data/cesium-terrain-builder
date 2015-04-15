# Ensure we have a unified GDAL: `GDALOpenEx` is only present after
# unification.
include(CheckLibraryExists)
if (${GDAL_LIBRARY_DIR})
  message("Searching GDAL library in provided ${GDAL_LIBRARY_DIR}")
endif()

check_library_exists(gdal GDALOpenEx "${GDAL_LIBRARY_DIR}" HAVE_UNIFIED_GDAL)
if(NOT MSVC AND NOT HAVE_UNIFIED_GDAL)
  message(FATAL_ERROR "The GDAL version must be one that implements RFC 46 (GDAL/OGR unification) i.e. >= 2.0.0")
endif()

# We need zlib
find_package(ZLIB)
if(NOT ZLIB_FOUND)
  message(FATAL_ERROR "The zlib library cannot be found on the system")
endif()
include_directories(${ZLIB_INCLUDE_DIRS})

add_library(ctb SHARED
  GDALTile.cpp
  GDALTiler.cpp
  TerrainTiler.cpp
  TerrainTile.cpp
  GlobalMercator.cpp
  GlobalGeodetic.cpp)
target_link_libraries(ctb ${GDAL_LIBRARIES} ${ZLIB_LIBRARIES})

# Install libctb
set(HEADERS
  Bounds.hpp
  Coordinate.hpp
  GDALTile.hpp
  GDALTiler.hpp
  GlobalGeodetic.hpp
  GlobalMercator.hpp
  Grid.hpp
  GridIterator.hpp
  RasterIterator.hpp
  RasterTiler.hpp
  CTBException.hpp
  TerrainIterator.hpp
  TerrainTile.hpp
  TerrainTiler.hpp
  Tile.hpp
  TileCoordinate.hpp
  TilerIterator.hpp
  types.hpp)
install(FILES ${HEADERS} DESTINATION include/ctb)
install(FILES ctb.hpp DESTINATION include)

install(TARGETS ctb DESTINATION lib)
