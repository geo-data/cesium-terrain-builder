test: test.cpp src/GlobalGeodetic.hpp
	g++ -Wall --pedantic -lgdal test.cpp -o test
terrain: terrain.cpp src/TerrainTile.hpp
	g++ -Wall --pedantic terrain.cpp -o terrain
