#ifndef TERRAINEXCEPTION_HPP
#define TERRAINEXCEPTION_HPP

#include <stdexcept>

namespace terrain {
  class TerrainException;
}

class terrain::TerrainException:
  public std::runtime_error
{ 
public:
  TerrainException(const char *message):
    std::runtime_error(message)
  {}
};

#endif /* TERRAINEXCEPTION_HPP */
