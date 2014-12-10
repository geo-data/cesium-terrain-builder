# Cesium Terrain Builder

This is an Ubuntu derived image containing Cesium Terrain Builder compiled
against a GDAL installation bundled with a broad range of drivers.  It is
designed to quickly get you up and running with the Cesium Terrain Builder
command line tools and/or coding with `libctb`.

Note that the tiles generated with these tools can be visualised with the
[geodata/cesium-terrain-server](https://registry.hub.docker.com/u/geodata/cesium-terrain-server/)
image.

## Usage

The following command will open a bash shell in an Ubuntu based environment
Cesium Terrain Builder available:

    docker run -t -i homme/cesium-terrain-builder:latest /bin/bash

You can run the command line utilities from there e.g.

    ctb-tile --version

You will most likely want to work with data on the host system from within the
docker container, in which case run the container with the -v option. This
mounts a host directory inside the container; the following invocation maps the
host's /tmp to /data in the container:

    docker run -v /tmp:/data -t -i homme/cesium-terrain-builder:latest bash

You can now access and save data to the host filesystem. For example the
following command entered at the prompt provided with the previous command will
build a terrain tileset.  This assumes `/tmp/source.tiff` is present on the
host system:

    mkdir /data/tiles && ctb-tile -o /data/tiles /data/source.tiff

You should now have the tiles available on your host system in `/tmp/tiles`.
