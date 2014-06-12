#ifndef TILEITERATOR_HPP
#define TILEITERATOR_HPP

#include <iterator>

#include "TerrainTile.hpp"

class GDALTiler;

class TileIterator : 
public std::iterator<std::input_iterator_tag, TerrainTile *>
{
  int tminx, tminy, tmaxx, tmaxy;
  unsigned short int zoom;
  int tx, ty;
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

  const TerrainTile *
  operator*() const;

  bool
  exhausted() const;
};

#endif /* TILEITERATOR_HPP */
