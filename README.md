# Cesium Terrain Builder

This is a C++ library and associated command line tools designed to create
terrain tiles for use with the [Cesium JavaScript](http://cesiumjs.org) library.

Cesium can create interactive 3D globes (à la Google Earth) in your web browser
whereby imagery is draped over a model of the underlying terrain.  Cesium
provides a number of
[different sources](http://cesiumjs.org/2013/02/15/Cesium-Terrain-Tutorial) for
the terrain data, one of which is height map data for use with the
[`CesiumTerrainProvider`](http://cesiumjs.org/Cesium/Build/Documentation/CesiumTerrainProvider.html)
JavaScript class.

Cesium Terrain Builder can be used to create the tilesets that sit behind a
terrain server used by `CesiumTerrainProvider`.  Note that these tools do *not*
provide a way of serving up those tilesets to the browser: instead
[Cesium Terrain Server](https://github.com/geo-data/cesium-terrain-server) is
designed to serve terrain tilesets.  In particular the Docker
[geodata/cesium-terrain-server](https://registry.hub.docker.com/u/geodata/cesium-terrain-server/)
image is designed to simplify the visualisation of terrain tilesets.

## Command Line Tools

The following tools are built on top of the C++ `libctb` library:

### `ctb-tile`

This creates gzipped terrain tiles from a GDAL raster representing a
[Digital Elevation Model](http://en.wikipedia.org/wiki/Digital_elevation_model)
(DEM), saving the resulting tiles to a directory.  It calculates the maximum
zoom level concomitant with the native raster resolution and creates terrain
tiles for all zoom levels between that maximum and zoom level `0` where the
tile extents overlap the raster extents, resampling and subsetting the data as
necessary. E.g.

    ctb-tile --output-dir ./terrain-tiles dem.tif

The input raster should contain data representing elevations relative to sea
level. `NODATA` (null) values are not currently dealt with: these should be
filled using interpolation in a data preprocessing step.

Note that in the case of multiband rasters, only the first band is used as the
input DEM.

As well as creating terrain tiles, the tool can also be used for generating
tiles in GDAL supported formats using the `--output-format` option.  This
provides similar functionality to the
[`gdal2tiles.py`](http://www.gdal.org/gdal2tiles.html) script. Tiles can be
created in either Web Mercator or Global Geodetic projections using the
`--profile` option.  e.g.

    ctb-tile --output-format JPEG --profile mercator \
      --output-dir ./jpeg-tiles RGB-image.tif

An interesting variation on this is to specify `--output-format VRT` in order to
generate GDAL Virtual Rasters: these can be useful for debugging and are easily
modified programatically.

```
Usage: ctb-tile [options] GDAL_DATASOURCE

Options:

  -V, --version                       output program version
  -h, --help                          output help information
  -o --output-dir <dir>               specify the output directory for the tiles (defaults to working directory)
  -f --output-format <format>         specify the output format for the tiles. This is either `Terrain` (the default), `Mesh` (Chunked LOD mesh), or any format listed by `gdalinfo --formats`
  -p --profile <profile>              specify the TMS profile for the tiles. This is either `geodetic` (the default) or `mercator`
  -c --thread-count <count>           specify the number of threads to use for tile generation. On multicore machines this defaults to the number of CPUs
  -t --tile-size <size>               specify the size of the tiles in pixels. This defaults to 65 for terrain tiles and 256 for other GDAL formats
  -s --start-zoom <zoom>              specify the zoom level to start at. This should be greater than the end zoom level
  -e --end-zoom <zoom>                specify the zoom level to end at. This should be less than the start zoom level and >= 0
  -r --resampling-method <algorithm>  specify the raster resampling algorithm.  One of: nearest; bilinear; cubic; cubicspline; lanczos; average; mode; max; min; med; q1; q3. Defaults to average.
  -n --creation-option <option>       specify a GDAL creation option for the output dataset in the form NAME=VALUE. Can be specified multiple times. Not valid for Terrain tiles.
  -z --error-threshold <threshold>    specify the error threshold in pixel units for transformation approximation. Larger values should mean faster transforms. Defaults to 0.125
  -m --warp-memory <bytes>            specify the memory limit in bytes used for warp operations. Higher settings should be faster. Defaults to a conservative GDAL internal setting.
  -R --resume                         flag do not overwrite existing files
  -g --mesh-qfactor <factor>          specify the factor to multiply the estimated geometric error to convert heightmaps to irregular meshes. Larger values should mean minor quality. Defaults to 1.0
  -l --layer                          flag only outputs the layer.json metadata file
  -C --cesium-friendly                flag forces the creation of missing root tiles to be CesiumJS-friendly
  -N --vertex-normals                 flag writes 'Oct-Encoded Per-Vertex Normals' for Terrain Lighting, only for `Mesh` format
  -q --quiet                          flag outputs only errors
  -v --verbose                        flag outputs more noisy
```

#### Recommendations

* For performance reasons it is recommended that the input raster be in the same
  spatial reference system as the output tile grid in order to bypass the need
  to reproject the data.  For terrain data this is
  [World Geodetic System](http://en.wikipedia.org/wiki/World_Geodetic_System)
  (WGS 84).  If the source data is in another spatial reference system, however,
  the tool will attempt to reproject the data but with an associated performance
  penalty.

* For large rasters a tile based format (as opposed to scanline based) will
  drastically speed up processing.  A block size that is similar to the tile
  output size (i.e. 65x65 for terrain tiles) should be chosen.

* Adding overviews to the source dataset will speed up tile generation.  The
  overview will be chosen whose resolution most closely matches that of the zoom
  level being rendered.  Overviews will only be downsampled, never upsampled.
  As such, it is recommended to use rolutions corresponding to the
  [Global Geodetic Profile](http://wiki.osgeo.org/wiki/Tile_Map_Service_Specification#global-geodetic)
  in the Tile Mapping Service specification.  See the
  [`gdaladdo`](http://www.gdal.org/gdaladdo.html) tool for creating overviews.

* DEM datasets composed of multiple files can be composited into a single GDAL
  [Virtual Raster](http://www.gdal.org/gdal_vrttut.html) (VRT) dataset for use
  as input to `ctb-tile` and `ctb-extents`.  See the
  [`gdalbuildvrt`](http://www.gdal.org/gdalbuildvrt.html) tool.

* Setting
  [GDAL runtime configuration](http://trac.osgeo.org/gdal/wiki/ConfigOptions)
  options will also affect Cesium Terrain Builder.  Specifically the
  [`GDAL_CACHEMAX`](http://trac.osgeo.org/gdal/wiki/ConfigOptions#GDAL_CACHEMAX)
  environment variable should be set to a relatively high value, in conjunction
  with the warp memory, if required (see next recommendation).
  
* If warping the source dataset then set the warp memory to a relatively high
  value.  The correct value is system dependent but try starting your benchmarks
  from a value where the combined value of `GDAL_CACHEMAX` and the warp memory
  represents about 2/3 of your available RAM.

* `ctb-tile` will resample data from the source dataset when generating
  tilesets for the various zoom levels.  This can lead to performance issues and
  datatype overflows at lower zoom levels (e.g. level 0) when the source dataset
  is very large.  To overcome this the tool can be used on the original dataset
  to only create the tile set at the highest zoom level (e.g. level 18) using
  the `--start-zoom` and `--end-zoom` options.  Once this tileset is generated
  it can be turned into a GDAL Virtual Raster dataset for creating the next zoom
  level down (e.g. level 17).  Repeating this process until the lowest zoom
  level is created means that the resampling is much more efficient (e.g. level
  0 would be created from a VRT representation of level 1).  Because terrain
  tiles are not a format supported by VRT datasets you will need to perform this
  process in order to create tiles in a GDAL DEM format as an intermediate step.
  VRT representations of these intermediate tilesets can then be used to create
  the final terrain tile output.

### `ctb-info`

This provides various information on a terrain tile, mainly useful for
debugging purposes.

```
Usage: ctb-info [options] TERRAIN_FILE

Options:

  -V, --version                 output program version
  -h, --help                    output help information
  -e, --show-heights            show the height information as an ASCII raster
  -c, --no-child                hide information about child tiles
  -t, --no-type                 hide information about the tile type (i.e. water/land)
```

### `ctb-export`

This exports a terrain tile to [GeoTiff](http://en.wikipedia.org/wiki/GeoTIFF)
format for use in GIS software.  Terrain tiles do not contain information
defining their tile location, so this must be specified through the command
options.

Note that the tool does not normalise the terrain data to sea level but
displays it exactly as it is found in the terrain data.

```
Usage: ctb-export -i TERRAIN_FILE -z ZOOM_LEVEL -x TILE_X -y TILE_Y -o OUTPUT_FILE

Options:

  -V, --version                 output program version
  -h, --help                    output help information
  -i, --input-filename <filename> the terrain tile file to convert
  -z, --zoom-level <int>        the zoom level represented by the tile
  -x, --tile-x <int>            the tile x coordinate
  -y, --tile-y <int>            the tile y coordinate
  -o, --output-filename <filename> the output file to create
```

### `ctb-extents`

Sometimes it is useful to see the extent of coverage of terrain tilesets that
would be produced from a raster.  This tool does this by outputting each zoom
level as a [GeoJSON](http://geojson.org/) file containing the tile extents for
that particular zoom level.

```
Usage: ctb-extents GDAL_DATASET

Options:

  -V, --version                 output program version
  -h, --help                    output help information
  -o, --output-dir <dir>        specify the output directory for the geojson files (defaults to working directory)
  -p, --profile <profile>       specify the TMS profile for the tiles. This is either `geodetic` (the default) or `mercator`
  -t, --tile-size <size>        specify the size of the tiles in pixels. This defaults to 65 for terrain tiles and 256 for other GDAL formats
  -s, --start-zoom <zoom>       specify the zoom level to start at. This should be greater than the end zoom level
  -e, --end-zoom <zoom>         specify the zoom level to end at. This should be less than the start zoom level and >= 0
```

## LibCTB

`libctb` is a library implemented in standard C++11.  It is capable of creating
terrain tiles according to the
[heightmap-1.0 terrain format](http://cesiumjs.org/data-and-assets/terrain/formats/heightmap-1.0.html). It
does not provide a way of serving up or storing the resulting tiles: this is
application specific. Instead its aim is simply to take a
[GDAL](http://www.gdal.org) supported raster format representing a Digital
Terrain Model (DTM) and convert this to terrain tiles.

See the source code for the tools provided with the library
(e.g. `ctb-tile`) for examples on how the library is used to achieve
this.

### Documentation

[Doxygen](http://www.doxygen.org) based documentation is available for the C++
code: run the `doxygen` command in the `doc/` directory and point your browser
at `doc/html/index.html`.

## Status

Although the software has been used to create a substantial number of terrain
tile sets currently in production use, it should be considered beta quality
software: it needs broader testing, a comprehensive test harness and the API is
liable to change.

The software has primarily been developed and deployed on a Linux OS, and this
is the only officially supported platform.  However, it has been reported as
compiling and running on:

* Windows using Visual Studio 2010 and 2013 (see
  [this issue](https://github.com/geo-data/cesium-terrain-builder/issues/6)).

* Mac OS X Mavericks using clang (see
  [this issue](https://github.com/geo-data/cesium-terrain-builder/issues/3)).

Porting it to other systems should be relatively painless as the library
dependencies have been ported to numerous systems and the code itself is
standard C++11.

## Requirements

### Runtime requirements

Ensure [GDAL](http://www.gdal.org) >= 2.0.0 is installed.  At the time of
writing this is not a stable release so you may need to use a nightly build or
to build the source directly from version control.  Specifically, you will need
a version of trunk that has added the min,max,med,q1 and q3 resampling
algorithms.  In the subversion repository this is commit 28717 and on the GitHub
mirror this is 0a90a34.

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

Note that if you have GDAL installed in a custom location (e.g under
`/home/user/install`) it will likely not be found by running `cmake ..`. In this
case you will need to provide the `GDAL_LIBRARY_DIR`, `GDAL_LIBRARY` and
`GDAL_INCLUDE_DIR` directives e.g.

```sh
cmake -DGDAL_LIBRARY_DIR=/home/user/install/lib \
      -DGDAL_LIBRARY=/home/user/install/lib/libgdal.so \
      -DGDAL_INCLUDE_DIR=/home/user/install/include \
      ..
```

### Using Docker

[homme/cesium-terrain-builder](https://registry.hub.docker.com/u/homme/cesium-terrain-builder/)
is a [Docker](http://www.docker.com/) image that bundles the CTB tools and
simplifies their deployment.  Follow the link for usage information.

The only requirement to getting up and running with Cesium Terrain Builder is
having docker available on your system: all software dependencies, build and
installation issues are encapsulated in the image.

In addition, the
[geodata/cesium-terrain-server](https://registry.hub.docker.com/u/geodata/cesium-terrain-server/)
Docker image provides a way of visualising the tilesets created by
`homme/cesium-terrain-builder`.

## Limitations and TODO

* Create a comprehensive test harness (possibly using
  [Bandit](http://banditcpp.org/), including code coverage and valgrind
  analysis.

* Better coordination between threads in `ctb-tile` to enable graceful exits if
  there is a fatal error or other interrupt.

* Add support for the new
  [quantized-mesh-1.0 terrain format](http://cesiumjs.org/data-and-assets/terrain/formats/quantized-mesh-1.0.html).

* The `ctb-tile` command currently only outputs files to a directory and
  as such is subjected to filesystem limits (e.g. inode limits): it should be
  able to output tiles in a format that overcomes these limits and which is
  still portable and accessible.  [SQLite](http://www.sqlite.org/) would appear
  to be a strong contender.

* Provide hooks into the GDAL error handling mechanism to more gracefully
  intercept GDAL errors.

* Expose tilers using a standard container api (map and/or vector).

* Enable more options to be passed to the VRT warper by parameterising
  `GDALWarpOptions::papszWarpOptions` and `GDALCreateGenImgProjTransformer2` in
  `GDALTiler::createRasterTile`.

* Encapsulate the multithreading tile generation functionality currently
  implemented in `ctb-tile` within the library to make it more widely
  available.

* One of the `ctb-tile` recommendations above illustrates a process for
  efficiently creating tilesets at lower zoom levels by resampling an already
  generated tileset at the next highest zoom level.  This could be built
  directly into the `ctb-tile` tool.  An implementation could create a
  read-only GDAL `TiledDataset` driver (or use a VRT, if it efficiently supports
  the large number of tile files) which accesses the already generated tileset;
  this dataset could then be used as an input to the tiler.

* Add support for interpolating out `NODATA` values.  This could be done using
  either `GDALFillNodata()` or `GDALGridCreate()`.

* Adding support for creating water masks to tiles could be useful: at the
  moment all tiles are flagged as being of type 'land' (see
  [this issue](https://github.com/geo-data/cesium-terrain-builder/issues/13)).

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

Thanks to everyone in the community who has contributed to the code base.

## Contact

Homme Zwaagstra <hrz@geodata.soton.ac.uk>
