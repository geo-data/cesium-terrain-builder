#ifndef TERRAINEXCEPTION_HPP
#define TERRAINEXCEPTION_HPP

#include <stdexcept>

class TerrainException:
  public std::runtime_error
{ 
public:
  TerrainException(const char *message):
    std::runtime_error(message)
  {}
};

#endif /* TERRAINEXCEPTION_HPP */
