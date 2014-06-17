set(TOOL_TARGETS commander terrain)

# Add the `terrain-build` executable
add_executable(terrain-build terrain-build.cpp)
target_link_libraries(terrain-build ${TOOL_TARGETS})

# Add the `terrain-export` executable
add_executable(terrain-export terrain-export.cpp)
target_link_libraries(terrain-export ${TOOL_TARGETS})

# Add the `terrain-info` executable
add_executable(terrain-info terrain-info.cpp)
target_link_libraries(terrain-info ${TOOL_TARGETS})

# Add the `terrain-extents` executable
add_executable(terrain-extents terrain-extents.cpp)
target_link_libraries(terrain-extents ${TOOL_TARGETS})

# Install the tools
set(TOOLS terrain-build terrain-export terrain-info terrain-extents)
install(TARGETS ${TOOLS} DESTINATION bin)
