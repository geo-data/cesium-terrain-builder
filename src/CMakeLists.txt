# We need GDAL
find_package(GDAL)
if(!GDAL_FOUND)
  message(FATAL_ERROR "The GDAL library cannot be found on the sytem")
endif()
include_directories(${GDAL_INCLUDE_DIRS})

# We need zlib
find_package(ZLIB)
if(!ZLIB_FOUND)
  message(FATAL_ERROR "The zlib library cannot be found on the sytem")
endif()
include_directories(${ZLIB_INCLUDE_DIRS})
  
add_library(terrain SHARED GDALTiler.cpp TerrainTile.cpp TileIterator.cpp)
target_link_libraries(terrain ${GDAL_LIBRARIES} ${ZLIB_LIBRARIES})

# Install libterrain
file(GLOB HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")
install(FILES ${HEADERS} DESTINATION include)
install(TARGETS terrain DESTINATION lib)
