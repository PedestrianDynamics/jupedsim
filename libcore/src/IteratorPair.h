#pragma once


/// Pair of Iterators for range based for loops
///
/// Provides begin and end method required for range based for loops.
/// _it_second must be reachable by _it_first.
template <typename Iterator>
class IteratorPair
{
    Iterator _it_first, _it_second;

public:
    IteratorPair(Iterator it1, Iterator it2) : _it_first(it1), _it_second(it2) {}

    Iterator first() const { return _it_first; }
    Iterator second() const { return _it_second; }

    Iterator begin() const { return first(); }
    Iterator end() const { return second(); }
};
