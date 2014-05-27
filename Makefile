test: test.cpp src/GlobalGeodetic.hpp
	g++ -Wall --pedantic -lgdal test.cpp -o test
