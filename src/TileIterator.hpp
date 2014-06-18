#ifndef TILEITERATOR_HPP
#define TILEITERATOR_HPP

/**
 * @file TileIterator.hpp
 * @brief This declares the `TileIterator` class
 */

#include <iterator>

#include "TileCoordinate.hpp"
#include "TerrainTile.hpp"

namespace terrain {
  class TileIterator;
  class GDALTiler;
}

/**
 * @brief A `TileIterator` forward iterates over all tiles in a `GDALTiler`
 *
 * Instances of this class take a `GDALTiler` in the constructor and are used
 * to forward iterate over all tiles in the tiler, starting from the maximum
 * zoom level and moving up to level `0` e.g.
 *
 * \code
 *    for(TileIterator iter(tiler); !iter.exhausted(); ++iter) {
 *      TerrainTile tile = *iter;
 *      // do stuff with tile
 *    }
 * \endcode
 */
class terrain::TileIterator :
public std::iterator<std::input_iterator_tag, TerrainTile>
{
public:

  /// Instantiate an iterator with a tiler
  TileIterator(const terrain::GDALTiler &tiler);

  /// Override the ++prefix operator
  terrain::TileIterator &
  operator++();

  /// Override the postfix++ operator
  terrain::TileIterator
  operator++(int);

  /// Override the equality operator
  bool
  operator==(const terrain::TileIterator &b) const;

  /// Override the inequality operator
  bool
  operator!=(const terrain::TileIterator &b) const {
    return !operator==(b);
  }

  /// Override the dereference operator to return a `TerrainTile`
  terrain::TerrainTile
  operator*() const;

  /// Return `true` if the iterator is at the end
  bool
  exhausted() const;

private:

  terrain::TileBounds bounds; ///< The extent of the currently iterated zoom level
  terrain::TileCoordinate coord; ///< The identity of the current tile being pointed to
  const terrain::GDALTiler &tiler; ///< The tiler we are iterating over
};

#endif /* TILEITERATOR_HPP */
