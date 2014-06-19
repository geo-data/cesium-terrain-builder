#!/bin/env sh

##
# Obtain, configure and install Cesium Terrain Builder
#

checkout=`cat /tmp/ctb-checkout.txt`

# Get CTB from github
cd /tmp/ && \
    wget "https://github.com/geo-data/cesium-terrain-builder/archive/${checkout}.tar.gz" && \
    tar -xzf "${checkout}.tar.gz" && \
    cd "cesium-terrain-builder-${checkout}" && \
    mkdir build && \
    cd build && \
    cmake .. && \
    make install && \
    ldconfig || exit 1
