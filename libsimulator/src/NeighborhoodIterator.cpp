/// Copyright © 2012-2022 Forschungszentrum Jülich GmbH
/// SPDX-License-Identifier: LGPL-3.0-or-later
#include "NeighborhoodIterator.hpp"

#include "Agent.hpp"

NeighborhoodIterator::NeighborhoodIterator(
    const Grid2D<Agent*>& grid,
    std::int32_t idx,
    std::int32_t idy,
    std::int32_t max_idx,
    std::int32_t max_idy,
    std::function<bool(Point)> filter)
    : _grid(grid)
    , _start_idx(idx)
    , _start_idy(idy)
    , _max_idx(max_idx)
    , _max_idy(max_idy)
    , _cur_idx(idx)
    , _cur_idy(idy)
    , _filter(std::move(filter))
    , _cur_grid_it(_grid.get({idx, idy}).begin())
    , _cur_grid_end(_grid.get({idx, idy}).end())
{
    while(!is_it_valid() && !is_ended()) {
        increment();
    }
}
NeighborhoodIterator& NeighborhoodIterator::operator++()
{
    if(is_ended()) {
        return *this;
    }

    increment();
    while(!is_it_valid() && !is_ended()) {
        increment();
    }

    return *this;
}
NeighborhoodIterator NeighborhoodIterator::operator++(int)
{
    NeighborhoodIterator copy = *this;
    ++(*this);
    return copy;
}
bool NeighborhoodIterator::operator==(NeighborhoodEndIterator /*other*/) const
{
    return is_ended();
}
bool NeighborhoodIterator::operator!=(NeighborhoodEndIterator /*other*/) const
{
    return !is_ended();
}
Agent* NeighborhoodIterator::operator*() const
{
    return _cur_grid_it->value;
}

void NeighborhoodIterator::increment_grid_indices()
{
    if(_cur_idy < _max_idy) {
        ++_cur_idy;
    } else {
        _cur_idy = _start_idy;
        ++_cur_idx;
    }

    auto it_pair = _grid.get({_cur_idx, _cur_idy});
    _cur_grid_it = it_pair.begin();
    _cur_grid_end = it_pair.end();
}

void NeighborhoodIterator::increment()
{
    if(_grid.get({_cur_idx, _cur_idy}).empty()) {
        increment_grid_indices();
    } else {
        ++_cur_grid_it;
        if(_cur_grid_it == _cur_grid_end) {
            increment_grid_indices();
        }
    }
}

bool NeighborhoodIterator::is_it_valid() const
{
    return _cur_grid_it != _cur_grid_end && !is_ended() && _filter(_cur_grid_it->value->GetPos());
}

bool NeighborhoodIterator::is_ended() const
{
    return _cur_idx > _max_idx;
}
