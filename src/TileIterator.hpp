#ifndef TILEITERATOR_HPP
#define TILEITERATOR_HPP

#include <iterator>

#include "TileCoordinate.hpp"
#include "TerrainTile.hpp"

namespace terrain {
  class TileIterator;
  class GDALTiler;
}

class terrain::TileIterator :
public std::iterator<std::input_iterator_tag, TerrainTile>
{
  terrain::TileBounds bounds;
  terrain::TileCoordinate coord;
  const terrain::GDALTiler &tiler;

public:
  TileIterator(const terrain::GDALTiler &tiler);

  terrain::TileIterator &
  operator++();

  terrain::TileIterator
  operator++(int);

  bool
  operator==(const terrain::TileIterator &b) const;

  bool
  operator!=(const terrain::TileIterator &b) const {
    return !operator==(b);
  }

  terrain::TerrainTile
  operator*() const;

  bool
  exhausted() const;
};

#endif /* TILEITERATOR_HPP */
