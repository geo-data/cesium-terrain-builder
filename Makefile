all: gdal2terrain terrain-info terrain-tile-bounds
gdal2terrain: gdal2terrain.cpp src/GlobalGeodetic.hpp src/TerrainTile.hpp src/GDALTiler.hpp
	g++ -Wall --pedantic -lgdal gdal2terrain.cpp -o gdal2terrain
terrain-info: terrain-info.cpp src/TerrainTile.hpp
	g++ -Wall --pedantic -lgdal terrain-info.cpp -o terrain-info
terrain-tile-bounds: terrain-tile-bounds.cpp src/GDALTiler.hpp
	g++ -Wall --pedantic -lgdal terrain-tile-bounds.cpp -o terrain-tile-bounds
