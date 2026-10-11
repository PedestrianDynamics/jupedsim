// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

/// Pair of Iterators for range based for loops
///
/// Provides begin and end method required for range based for loops.
/// _it_second must be reachable by _it_first.
#include <ranges>

/// Deriving from view_interface marks this as a std::ranges view, so that a pair returned by
/// value can be piped into range adaptors without being copied into a container first.
template <typename IteratorFirst, typename IteratorSecond = IteratorFirst>
class IteratorPair : public std::ranges::view_interface<IteratorPair<IteratorFirst, IteratorSecond>>
{
    IteratorFirst _it_first;
    IteratorSecond _it_second;

public:
    IteratorPair(IteratorFirst it1, IteratorSecond it2) : _it_first(it1), _it_second(it2) {}

    IteratorFirst first() const { return _it_first; }
    IteratorSecond second() const { return _it_second; }

    IteratorFirst begin() const { return first(); }
    IteratorSecond end() const { return second(); }

    bool empty() const { return _it_first == _it_second; }

    /// Does not define size() on purpose: counting would have to walk the range, which is O(n).
    /// std::ranges::sized_range promises O(1). view_interface adds size() by itself in case the
    /// iterators can subtract.
};
