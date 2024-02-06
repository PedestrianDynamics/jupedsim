// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

/// Pair of Iterators for range based for loops
///
/// Provides begin and end method required for range based for loops.
/// _it_second must be reachable by _it_first.
template <typename IteratorFirst, typename IteratorSecond = IteratorFirst>
class IteratorPair
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
};
