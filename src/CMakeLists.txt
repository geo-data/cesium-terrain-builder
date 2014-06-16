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
  
add_library(terrain SHARED GDALTiler.cpp TerrainTile.cpp TileIterator.cpp)
target_link_libraries(terrain ${GDAL_LIBRARIES} ${ZLIB_LIBRARIES})

# Install libterrain
file(GLOB HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")
install(FILES ${HEADERS} DESTINATION include)
install(TARGETS terrain DESTINATION lib)
