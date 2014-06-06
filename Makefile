all: gdal2terrain terrain-info terrain-tile-bounds terrain2tiff
gdal2terrain: gdal2terrain.cpp src/GlobalGeodetic.hpp src/TerrainTile.hpp src/GDALTiler.hpp src/Bounds.hpp
	g++ -Wall --pedantic -lgdal -lz gdal2terrain.cpp -o gdal2terrain
terrain-info: terrain-info.cpp src/TerrainTile.hpp
	g++ -Wall --pedantic -lgdal -lz terrain-info.cpp -o terrain-info
terrain-tile-bounds: terrain-tile-bounds.cpp src/GDALTiler.hpp src/Bounds.hpp
	g++ -Wall --pedantic -lgdal -lz terrain-tile-bounds.cpp -o terrain-tile-bounds
terrain2tiff: terrain-tile-bounds.cpp src/TerrainTile.hpp src/Bounds.hpp
	g++ -Wall --pedantic -lgdal -lz terrain2tiff.cpp -o terrain2tiff
