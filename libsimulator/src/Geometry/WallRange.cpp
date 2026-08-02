// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Geometry/WallRange.hpp"

#include <algorithm>

WallRange::Iterator::Iterator(const WallRange* range, bool at_end) : _range(range)
{
    if(range->_isFlat) {
        _flat = at_end ? range->_flat.end() : range->_flat.begin();
        return;
    }
    if(at_end || range->_spans.empty()) {
        _span = range->_spans.size();
        return;
    }
    _wall = range->_spans.front().begin();
    settle();
}

bool WallRange::Iterator::worth_handing_out()
{
    // A wall reached through a second region has already been handed out by the first. With
    // one span there is no second source, so the bookkeeping is skipped entirely.
    if(_range->_spans.size() <= 1) {
        return true;
    }
    if(std::find(_seen.begin(), _seen.end(), (*_wall).index) != _seen.end()) {
        return false;
    }
    _seen.push_back((*_wall).index);
    return true;
}

void WallRange::Iterator::settle()
{
    while(_span < _range->_spans.size()) {
        if(_wall == _range->_spans[_span].end()) {
            ++_span;
            if(_span < _range->_spans.size()) {
                _wall = _range->_spans[_span].begin();
            }
            continue;
        }
        if(!worth_handing_out()) {
            ++_wall;
            continue;
        }
        return;
    }
    _wall = DistanceQueryIterator<MergedWall>{};
}

const LineSegment& WallRange::Iterator::operator*() const
{
    return _range->_isFlat ? *_flat : (*_wall).segment;
}

WallRange::Iterator& WallRange::Iterator::operator++()
{
    if(_range->_isFlat) {
        ++_flat;
        return *this;
    }
    ++_wall;
    settle();
    return *this;
}

bool WallRange::Iterator::operator==(const Iterator& other) const
{
    if(_range != nullptr && _range->_isFlat) {
        return _flat == other._flat;
    }
    return _span == other._span && _wall == other._wall;
}
