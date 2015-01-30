# The tools are not shared libraries
add_definitions(-DCPL_DISABLE_DLL)

set(TOOL_TARGETS commander ctb  ${CMAKE_THREAD_LIBS_INIT})

# Add the `ctb-tile` executable
add_executable(ctb-tile ctb-tile.cpp)
target_link_libraries(ctb-tile ${TOOL_TARGETS})

# Add the `ctb-export` executable
add_executable(ctb-export ctb-export.cpp)
target_link_libraries(ctb-export ${TOOL_TARGETS})

# Add the `ctb-info` executable
add_executable(ctb-info ctb-info.cpp)
target_link_libraries(ctb-info ${TOOL_TARGETS})

# Add the `ctb-extents` executable
add_executable(ctb-extents ctb-extents.cpp)
target_link_libraries(ctb-extents ${TOOL_TARGETS})

# Install the tools
set(TOOLS ctb-tile ctb-export ctb-info ctb-extents)
install(TARGETS ${TOOLS} DESTINATION bin)
