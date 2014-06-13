set(TOOL_TARGETS commander terrain)

# Add the `gdal2terrain` executable
add_executable(gdal2terrain gdal2terrain.cpp)
target_link_libraries(gdal2terrain ${TOOL_TARGETS})

# Add the `terrain2tiff` executable
add_executable(terrain2tiff terrain2tiff.cpp)
target_link_libraries(terrain2tiff ${TOOL_TARGETS})

# Add the `terrain-info` executable
add_executable(terrain-info terrain-info.cpp)
target_link_libraries(terrain-info ${TOOL_TARGETS})

# Add the `terrain-tile-bounds` executable
add_executable(terrain-tile-bounds terrain-tile-bounds.cpp)
target_link_libraries(terrain-tile-bounds ${TOOL_TARGETS})

# Install the tools
set(TOOLS gdal2terrain terrain2tiff terrain-info terrain-tile-bounds)
install(TARGETS ${TOOLS} DESTINATION bin)
