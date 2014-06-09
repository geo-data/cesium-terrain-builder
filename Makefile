all: gdal2terrain terrain-info terrain-tile-bounds terrain2tiff
gdal2terrain: gdal2terrain.cpp src/GlobalGeodetic.hpp src/TerrainTile.hpp src/GDALTiler.hpp src/Bounds.hpp
	g++ -Wall -Wextra --pedantic -lgdal -lz gdal2terrain.cpp -o gdal2terrain
terrain-info: terrain-info.cpp src/TerrainTile.hpp
	g++ -Wall -Wextra --pedantic -lgdal -lz terrain-info.cpp -o terrain-info
terrain-tile-bounds: terrain-tile-bounds.cpp src/GDALTiler.hpp src/Bounds.hpp deps/commander.o deps/commander.hpp
	g++ -Wall -Wextra --pedantic -lgdal -lz -I./deps -o terrain-tile-bounds deps/commander.o terrain-tile-bounds.cpp
terrain2tiff: terrain2tiff.cpp src/TerrainTile.hpp src/Bounds.hpp src/GlobalGeodetic.hpp deps/commander.o deps/commander.hpp
	g++ -Wall -Wextra --pedantic -lgdal -lz -I./deps -o terrain2tiff deps/commander.o terrain2tiff.cpp
deps/commander.o: deps/commander.h deps/commander.c
	gcc -Wall -Wextra --pedantic -std=c99 -c -o commander.o deps/commander.c
