##
# Install Cesium Terrain Builder from within a docker container
#
# This Makefile is designed to be run from within a docker container in order to
# install Cesium Terrain Builder.  The following is an example invocation:
#
# make -C /usr/local/src/cesium-terrain-builder install clean
#

# Version related variables.
CTB_VERSION := $(shell cat ./ctb-checkout.txt)
GDAL_VERSION := $(shell cat ./gdal-checkout.txt)

# CTB dependency targets.
CTB := /usr/local/bin/ctb-tile
GDAL_CONFIG := /usr/local/bin/gdal_config
CTB_SRC := /tmp/cesium-terrain-builder-$(CTB_VERSION)

# Build tools.
WGET := /usr/bin/wget

install: $(CTB)

$(CTB): $(CTB_SRC) /tmp/ctb-build $(GDAL_CONFIG)
	cd /tmp/ctb-build \
	&& cmake $(CTB_SRC) \
	&& make -j$$(nproc) \
	&& make install \
	&& ldconfig \
	&& touch -c $(CTB)

/tmp/ctb-build:
	mkdir -p /tmp/ctb-build

$(CTB_SRC): $(WGET)
	$(WGET) --no-verbose "https://github.com/geo-data/cesium-terrain-builder/archive/$(CTB_VERSION).tar.gz" -O $(CTB_SRC).tar.gz \
	&& tar -C /tmp -xzf $(CTB_SRC).tar.gz \
	&& touch -c $(CTB_SRC)

$(GDAL_CONFIG): /usr/local/src/gdal-docker
	make -C /usr/local/src/gdal-docker install \
	&& touch -c $(GDAL_CONFIG)
/usr/local/src/gdal-docker: $(WGET)
	$(WGET) --no-verbose "https://github.com/geo-data/gdal-docker/archive/$(CTB_VERSION).tar.gz" -O /tmp/gdal-docker-$(GDAL_VERSION).tar.gz \
	&& tar -C /tmp -xzf /tmp/gdal-docker-$(GDAL_VERSION).tar.gz \
	&& mv /tmp/gdal-docker-$(GDAL_VERSION) /usr/local/src/gdal-docker \
	&& touch -c /usr/local/src/gdal-docker

$(WGET): /tmp/apt-updated
	apt-get install -y wget && touch -c $(WGET)

/tmp/apt-updated:
	apt-get update -y && touch /tmp/apt-updated

# Remove build time dependencies. Delegate to gdal-docker clean as this removes
# everything, including wget.
clean:
	make -C /usr/local/src/gdal-docker clean

.PHONY: install clean
