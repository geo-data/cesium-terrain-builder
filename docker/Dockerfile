## -*- docker-image-name: "homme/cesium-terrain-builder:latest" -*-

##
# Cesium Terrain Builder
#
# This creates an image containing an installation of the Cesium Terrain
# Builder software (https://github.com/geo-data/cesium-terrain-builder).
#

FROM homme/gdal:latest
MAINTAINER Homme Zwaagstra <hrz@geodata.soton.ac.uk>

# Upgrade from gcc 4.6 to 4.7 (see
# http://askubuntu.com/questions/113291/how-do-i-install-gcc-4-7)
RUN add-apt-repository -y ppa:ubuntu-toolchain-r/test
RUN apt-get update -y
RUN apt-get install -y gcc-4.7 g++-4.7
RUN apt-get remove -y gcc-4.6
RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-4.7 40 --slave /usr/bin/g++ g++ /usr/bin/g++-4.7

# Ensure `cmake` is present
RUN apt-get install -y cmake

# Install Cesium Terrain Builder itself
ADD ./ctb-checkout.txt /tmp/ctb-checkout.txt
ADD ./install-ctb.sh /tmp/
RUN sh /tmp/install-ctb.sh

# Clean up APT when done
RUN apt-get clean && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*
