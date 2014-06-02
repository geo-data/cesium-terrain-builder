test: test.cpp src/GlobalGeodetic.hpp src/TerrainTile.hpp src/GDALTiler.hpp
	g++ -Wall --pedantic -lgdal test.cpp -o test
terrain-info: terrain-info.cpp src/TerrainTile.hpp
	g++ -Wall --pedantic -lgdal terrain-info.cpp -o terrain-info
