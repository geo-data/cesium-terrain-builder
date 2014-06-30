# Cesium Terrain Builder

This is a C++ library and associated command line tools designed to create
terrain tiles for use in the [Cesium JavaScript](http://cesiumjs.org) library.

Cesium can create interactive 3D globes (à la Google Earth) in your web browser
whereby imagery is draped over a model of the underlying terrain.  Cesium
provides a number of
[different sources](http://cesiumjs.org/2013/02/15/Cesium-Terrain-Tutorial) for
the terrain data, one of which is height map data for use with the
[CesiumTerrainProvider](http://cesiumjs.org/Cesium/Build/Documentation/CesiumTerrainProvider.html)
JavaScript class.  Cesium Terrain Builder can be used to create the tilesets
that sit behind a terrain server used by CesiumTerrainProvider.  Note that it
does *not* provide a way of serving up those tilesets to the browser.

## Command Line Tools

The following tools are built on top of the C++ `libterrain` library:

### `terrain-build`

This creates gzipped terrain tiles from a GDAL raster representing a
[Digital Elevation Model](http://en.wikipedia.org/wiki/Digital_elevation_model)
(DEM), saving the resulting tiles to a directory.  It calculates the maximum
zoom level concomitant with the native raster resolution and creates terrain
tiles for all zoom levels between that maximum and zoom level `0` where the
tile extents overlap the raster extents, resampling and subsetting the data as
necessary.

The input raster should contain data representing elevations relative to sea
level. `NODATA` (null) values are not currently dealt with: these should be
filled using interpolation in a data preprocessing step.

Note that in the case of multiband rasters, only the first band is used as the
input DEM.

As well as creating terrain tiles, the tool can also be used for generating
tiles in GDAL supported formats using the `--output-format` option.  Tiles can
be created in either Web Mercator or Global Geodetic projections using the
`--profile` option.  Note that when generating terrain tiles for use in Cesium
the defaults should not be changed!

```
Usage: terrain-build [options] GDAL_DATASOURCE

Options:

  -V, --version                 output program version
  -h, --help                    output help information
  -o, --output-dir <dir>        specify the output directory for the tiles (defaults to working directory)
  -f, --output-format <format>  specify the output format for the tiles. This is either `Terrain` (the default) or any format listed by `gdalinfo --formats`
  -p, --profile <profile>       specify the TMS profile for the tiles. This is either `geodetic` (the default) or `mercator`
  -t, --tile-size <size>        specify the size of the tiles in pixels. This defaults to 65 for terrain tiles and 256 for other GDAL formats
  -s, --start-zoom <zoom>       specify the zoom level to start at. This should be greater than the end zoom level
  -e, --end-zoom <zoom>         specify the zoom level to end at. This should be less than the start zoom level and >= 0
```

#### Recommendations

* For performance reasons it is recommended that the input raster be in the
  [World Geodetic System](http://en.wikipedia.org/wiki/World_Geodetic_System)
  (WGS 84).  If it is in another spatial reference system, however, the tool
  will attempt to reproject the data to WGS 84 but with an associated
  performance penalty.

* For large rasters it is recommended that a format that supports overviews is
  used and overviews are implemented for resolutions corresponding to the
  [Global Geodetic Profile](http://wiki.osgeo.org/wiki/Tile_Map_Service_Specification#global-geodetic)
  in the Tile Mapping Service specification.  See the
  [`gdaladdo`](http://www.gdal.org/gdaladdo.html) tool for creating overviews.

* DEM datasets composed of multiple files can be composited into a single GDAL
  [Virtual Raster](http://www.gdal.org/gdal_vrttut.html) (VRT) dataset for use
  as input to CTB.  See the
  [`gdalbuildvrt`](http://www.gdal.org/gdalbuildvrt.html) tool.

### `terrain-info`

This provides various information on a terrain tile, mainly useful for
debugging purposes.

```
Usage: terrain-info [options] TERRAIN_FILE

Options:

  -V, --version                 output program version
  -h, --help                    output help information
  -e, --show-heights            show the height information as an ASCII raster
  -c, --no-child                hide information about child tiles
  -t, --no-type                 hide information about the tile type (i.e. water/land)
```

### `terrain-export`

This exports a terrain tile to [GeoTiff](http://en.wikipedia.org/wiki/GeoTIFF)
format for use in GIS software.  Terrain tiles do not contain information
defining their tile location, so this must be specified through the command
options.

Note that the tool does not normalise the terrain data to sea level but
displays it exactly as it is found in the terrain data.

```
Usage: terrain-export -i TERRAIN_FILE -z ZOOM_LEVEL -x TILE_X -y TILE_Y -o OUTPUT_FILE

Options:

  -V, --version                 output program version
  -h, --help                    output help information
  -i, --input-filename <filename> the terrain tile file to convert
  -z, --zoom-level <int>        the zoom level represented by the tile
  -x, --tile-x <int>            the tile x coordinate
  -y, --tile-y <int>            the tile y coordinate
  -o, --output-filename <filename> the output file to create
```

### `terrain-extents`

Sometimes it is useful to see the extent of coverage of terrain tilesets that
would be produced from a raster.  This tool does this by outputting each zoom
level as a [GeoJSON](http://geojson.org/) file containing the tile extents for
that particular zoom level.

```
Usage: terrain-extents GDAL_DATASET

Options:

  -V, --version                 output program version
  -h, --help                    output help information
  -o, --output-dir <dir>        specify the output directory for the geojson files (defaults to working directory)
  -p, --profile <profile>       specify the TMS profile for the tiles. This is either `geodetic` (the default) or `mercator`
  -t, --tile-size <size>        specify the size of the tiles in pixels. This defaults to 65 for terrain tiles and 256 for other GDAL formats
  -s, --start-zoom <zoom>       specify the zoom level to start at. This should be greater than the end zoom level
  -e, --end-zoom <zoom>         specify the zoom level to end at. This should be less than the start zoom level and >= 0
```

## LibTerrain

The C++ library is called `libterrain`.  It is capable of creating terrain
tiles according to the
[heightmap-1.0 terrain format](http://cesiumjs.org/data-and-assets/terrain/formats/heightmap-1.0.html). It
does not provide a way of serving up or storing the resulting tiles: this is
application specific. Instead its aim is simply to take a
[GDAL](http://www.gdal.org) supported raster format representing a Digital
Terrain Model (DTM) and convert this to terrain tiles.

See the source code for the tools provided with the library
(e.g. `terrain-build`) for examples on how the library is used to achieve
this.

### Documentation

[Doxygen](http://www.doxygen.org) based documentation is available for the C++
code: run the `doxygen` command in the `doc/` directory and point your browser
at `doc/html/index.html`.

### Implementation overview

The TMS Global Geodetic Profile is modelled by the `GlobalGeodetic` class.  The
`GDALTiler` class composes an instance of this class with a GDAL raster dataset
allowing it to find which zoom levels and tilesets are represented by the
raster.  For each valid tile it can then generate a GDAL
[Virtual Raster](http://www.gdal.org/gdal_vrttut.html) (VRT).  This is a
lightweight representation of the relevant underlying data necessary to create
a terrain tile.  The VRT can then be used to generate an actual `TerrainTile`
instance which can then be stored as required by the application.

The `TileIterator` class provides a simple interface for iterating over all
valid tiles represented by a `GDALTiler`.

## Status

Although the software has been used to create a substantial number of terrain
tile sets currently in production use, it should be considered alpha quality
software: it needs broader testing, a comprehensive test harness and the API is
liable to change.

To date the software has only been developed and deployed on a Linux OS,
although porting it to other systems should be relatively painless as the
library dependencies have been ported and the code itself is standard C++.

## Requirements

### Runtime requirements

Ensure [GDAL](http://www.gdal.org) >= 2.0.0 is installed.  At the time of
writing this is not a stable release so you may need a nightly build or build
the source directly from version control.

### Build requirements

In addition to ensuring the GDAL library is installed, you will need the GDAL
source development header files. You will also need
[CMake](http://www.cmake.org) to be available.

## Installation

### From Source

1. Ensure your system meets the requirements above.

2. [Download](https://github.com/geo-data/cesium-terrain-builder/releases) and
   unpack the source.

3. In the root package directory, assuming you are on a UNIX system, type
   `mkdir build && cd build && cmake .. && make install`.

4. On a UNIX system you may need to run `ldconfig` to update the shared library
   cache.

Alternatively in step 3 above you can create a debug build by running `cmake
-DCMAKE_BUILD_TYPE=Debug ..`.  You can also install to a different location by
specifying the `CMAKE_INSTALL_PREFIX` directive e.g. `cmake
-DCMAKE_INSTALL_PREFIX=/tmp/terrain ..`.

### Using Docker

A [Docker](http://www.docker.com/) image
[is available](https://registry.hub.docker.com/u/homme/cesium-terrain-builder/)
at the Docker Registry: follow the link for usage information.

The only requirement to getting up and running with Cesium Terrain Builder is
having docker available on your system: all software dependencies, build and
installation issues are encapsulated in the image.

## Limitations and TODO

* Create a comprehensive test harness, including code coverage and valgrind
  analysis.

* Add support for the new
  [quantized-mesh-1.0 terrain format](http://cesiumjs.org/data-and-assets/terrain/formats/quantized-mesh-1.0.html).

* The `terrain-build` command currently only outputs files to a directory and
  as such is subjected to filesystem limits (e.g. inode limits): it should be
  able to output tiles in a format that overcomes these limits and which is
  still portable and accessible.  [SQLite](http://www.sqlite.org/) would appear
  to be a strong contender.

* Provide hooks into the GDAL error handling mechanism to more gracefully
  intercept GDAL errors.

* Enable more options to be passed to the VRT warper, such as the resampling
  algorithm, deciding whether an approximate warp is acceptable, using the
  multi-threading warp functionality etc.

* Enable a zoom level range to be specified in the `terrain-build` tool (and
  the underlying `TileIterator` class): at the moment tiles are automatically
  generated from the maximum zoom level supported by the native raster
  resolution to zoom level `0`.

* Add support for creating water masks to tiles could be useful: at the moment
  all tiles are flagged as being of type 'land'.

## Issues and Contributing

Please report bugs or issues using the
[GitHub issue tracker](https://github.com/geo-data/cesium-terrain-builder).

Code and documentation contributions are very welcome, either as GitHub pull
requests or patches.  If you cannot do this but would still like to improve the
software, particularly overcoming the limitations listed above, then please
consider funding further development.

## License

The [Apache License, Version 2.0](http://www.apache.org/licenses/LICENSE-2.0).

## Acknowledgements

Software development funded by the
[Maritime Archaeology Trust](http://www.maritimearchaeologytrust.org/) and the
European Regional Development Fund through the
[Interreg IVA 2 Seas Programme](http://www.interreg4a-2mers.eu).

Software developed by [GeoData](http://www.geodata.soton.ac.uk) through the
[University of Southampton Open Source Geospatial Laboratory](http://www.osgl.soton.ac.uk/).

## Contact

Homme Zwaagstra <hrz@geodata.soton.ac.uk>
