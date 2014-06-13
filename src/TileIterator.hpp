#ifndef TILEITERATOR_HPP
#define TILEITERATOR_HPP

#include <iterator>

#include "TileCoordinate.hpp"
#include "TerrainTile.hpp"

class GDALTiler;

class TileIterator : 
public std::iterator<std::input_iterator_tag, TerrainTile>
{
  unsigned int tminx, tminy, tmaxx, tmaxy;
  TileCoordinate coord;
  const GDALTiler &tiler;

public:
  TileIterator(const GDALTiler &tiler);

  TileIterator &
  operator++();

  TileIterator
  operator++(int);

  bool
  operator==(const TileIterator &b) const;

  bool
  operator!=(const TileIterator &b) const {
    return !operator==(b);
  }

  TerrainTile
  operator*() const;

  bool
  exhausted() const;
};

#endif /* TILEITERATOR_HPP */
