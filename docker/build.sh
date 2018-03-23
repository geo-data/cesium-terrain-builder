#!/bin/sh

##
# Install Cesium Terrain Builder from within a docker container.
#

# Exit on any error.
set -e

# Install Cesium Terrain Builder.
mkdir /tmp/ctb-build
cd /tmp/ctb-build
cmake /usr/local/src/cesium-terrain-builder
make -j$(nproc)
make install
ldconfig

# Clean up.
apt-get autoremove -y
apt-get clean
rm -rf /var/lib/apt/lists/partial/* /tmp/* /var/tmp/*
