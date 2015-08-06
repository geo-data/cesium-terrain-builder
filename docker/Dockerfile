## -*- docker-image-name: "homme/cesium-terrain-builder:latest" -*-

##
# Cesium Terrain Builder
#
# This creates an image containing an installation of the Cesium Terrain
# Builder software (https://github.com/geo-data/cesium-terrain-builder).
#

# Ubuntu 14.04 Trusty Tahyr
FROM ubuntu:trusty
MAINTAINER Homme Zwaagstra <hrz@geodata.soton.ac.uk>

# Install the application.
ADD . /usr/local/src/cesium-terrain-builder/
RUN apt-get update -y && \
    apt-get install -y make && \
    make -C /usr/local/src/cesium-terrain-builder install clean && \
    apt-get purge -y make
