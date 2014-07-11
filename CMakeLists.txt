cmake_minimum_required(VERSION 2.6)
project("Cesium Terrain Builder")

# Ensure we have a C++11 compatible compiler (see
# http://www.guyrutenberg.com/2014/01/05/enabling-c11-c0x-in-cmake/)
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)

if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
    message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

# The version number
set(TERRAIN_VERSION_MAJOR 0)
set(TERRAIN_VERSION_MINOR 2)
set(TERRAIN_VERSION_PATCH 2)

# The tile size (width and height). According to the spec this should always be
# 65
set(TERRAIN_TILE_SIZE 65)
# The tile water mask size (width and height). According to the spec this
# should always be 256
set(TERRAIN_MASK_SIZE 256)

# Configure a header file to pass some of the CMake settings to the source code
configure_file(
    "${PROJECT_SOURCE_DIR}/src/config.hpp.in"
    "${PROJECT_BINARY_DIR}/config.hpp"
)

# Add the binary tree to the search path for include files so that we will find
# `config.hpp`
include_directories("${PROJECT_BINARY_DIR}")

# Ensure the configuration is installed
install(FILES ${PROJECT_BINARY_DIR}/config.hpp DESTINATION include/terrain)

# Perform as many checks as possible on debug builds:
#    cmake -DCMAKE_BUILD_TYPE=Debug ..
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra --pedantic")
#set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wall -Wextra --pedantic")

# Build and install libterrain
include_directories("${PROJECT_SOURCE_DIR}/src")
add_subdirectory(src)

# Build and install libcommander
include_directories("${PROJECT_SOURCE_DIR}/deps")
add_subdirectory(deps)

# Build and install the tools
add_subdirectory(tools)
