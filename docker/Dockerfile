## -*- docker-image-name: "homme/cesium-terrain-builder:latest" -*-

##
# The OpenStreetMap Tile Server
#
# This creates an image containing the OpenStreetMap tile server stack as
# described at
# <http://switch2osm.org/serving-tiles/manually-building-a-tile-server-12-04/>.
#

FROM homme/gdal:latest
MAINTAINER Homme Zwaagstra <hrz@geodata.soton.ac.uk>

# Ensure `cmake` is present
RUN apt-get install -y cmake

# Install Cesium Terrain Builder itself
ADD ./checkout.txt /tmp/ctb-checkout.txt
ADD ./install-ctb.sh /tmp/
RUN sh /tmp/install-ctb.sh

# Clean up APT when done
RUN apt-get clean && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*
