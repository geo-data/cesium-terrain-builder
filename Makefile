all: gdal2terrain terrain-info terrain-tile-bounds terrain2tiff

##
# Programs
#

gdal2terrain: gdal2terrain.cpp src/GDALTiler.o src/TileIterator.o src/TerrainTile.o deps/commander.o deps/commander.hpp
	g++ -Wall -Wextra --pedantic -lgdal -lz -I./deps -o gdal2terrain deps/commander.o src/TerrainTile.o src/TileIterator.o src/GDALTiler.o gdal2terrain.cpp

terrain-info: terrain-info.cpp src/TerrainTile.o deps/commander.o deps/commander.hpp
	g++ -Wall -Wextra --pedantic -lgdal -lz -I./deps -o terrain-info deps/commander.o src/TerrainTile.o terrain-info.cpp

terrain-tile-bounds: terrain-tile-bounds.cpp src/GDALTiler.o src/TerrainTile.o deps/commander.o deps/commander.hpp
	g++ -Wall -Wextra --pedantic -lgdal -lz -I./deps -o terrain-tile-bounds src/GDALTiler.o src/TerrainTile.o deps/commander.o terrain-tile-bounds.cpp

terrain2tiff: terrain2tiff.cpp src/TerrainTile.o src/GlobalGeodetic.hpp deps/commander.o deps/commander.hpp
	g++ -Wall -Wextra --pedantic -lgdal -lz -I./deps -o terrain2tiff src/TerrainTile.o deps/commander.o terrain2tiff.cpp


##
# Objects
#

src/GDALTiler.o: src/GlobalGeodetic.hpp src/Bounds.hpp src/TerrainTile.hpp src/GDALTiler.hpp src/GDALTiler.cpp
	g++ -Wall -Wextra --pedantic -c -o src/GDALTiler.o src/GDALTiler.cpp

src/TileIterator.o: src/TerrainTile.hpp src/TileIterator.hpp src/GDALTiler.hpp src/TileIterator.cpp
	g++ -Wall -Wextra --pedantic -c -o src/TileIterator.o src/TileIterator.cpp

src/TerrainTile.o: src/TerrainTile.hpp src/TerrainTile.cpp
	g++ -Wall -Wextra --pedantic -c -o src/TerrainTile.o src/TerrainTile.cpp

deps/commander.o: deps/commander.h deps/commander.c
	gcc -Wall -Wextra --pedantic -std=c99 -c -o deps/commander.o deps/commander.c


clean:
	rm -f src/*.o deps/*.o gdal2terrain terrain-info terrain-tile-bounds terrain2tiff

.PHONY: clean
