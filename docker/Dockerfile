## -*- docker-image-name: "homme/cesium-terrain-builder:latest" -*-

##
# Cesium Terrain Builder
#
# This creates an image containing an installation of the Cesium Terrain
# Builder software (https://github.com/geo-data/cesium-terrain-builder).
#

FROM homme/gdal:latest
MAINTAINER Homme Zwaagstra <hrz@geodata.soton.ac.uk>

# Ensure `cmake` is present
RUN apt-get update -y
RUN apt-get install -y cmake

# Install Cesium Terrain Builder itself
ADD ./ctb-checkout.txt /tmp/ctb-checkout.txt
ADD ./install-ctb.sh /tmp/
RUN sh /tmp/install-ctb.sh

# Clean up APT when done
RUN apt-get clean && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*
