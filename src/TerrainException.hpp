#ifndef TERRAINEXCEPTION_HPP
#define TERRAINEXCEPTION_HPP

/**
 * @file TerrainException.hpp
 * @brief This declares and defines the `TerrainException` class
 */

#include <stdexcept>

namespace terrain {
  class TerrainException;
}

/// This represents a terrain runtime error
class terrain::TerrainException:
  public std::runtime_error
{ 
public:
  TerrainException(const char *message):
    std::runtime_error(message)
  {}
};

#endif /* TERRAINEXCEPTION_HPP */
