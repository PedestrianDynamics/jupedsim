#pragma once

#include "Grid2D.hpp"
#include "Pedestrian.hpp"

#include <cstdint>
#include <functional>

class NeighborhoodEndIterator
{
};

class NeighborhoodIterator
{
public:
    NeighborhoodIterator(
        const Grid2D<Pedestrian*>& grid,
        std::int32_t idx,
        std::int32_t idy,
        std::int32_t max_idx,
        std::int32_t max_idy,
        std::function<bool(Point)> filter = [](Point) { return true; })
        : _grid(grid)
        , _start_idx(idx)
        , _start_idy(idy)
        , _max_idx(max_idx)
        , _max_idy(max_idy)
        , _cur_idx(idx)
        , _cur_idy(idy)
        , _filter(filter)
        , _cur_grid_it(_grid.get({idx, idy}).begin())
        , _cur_grid_end(_grid.get({idx, idy}).end())
    {
        while(!is_it_valid() && !is_ended()) {
            increment();
        }
    }
    NeighborhoodIterator& operator++()
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
    NeighborhoodIterator operator++(int)
    {
        NeighborhoodIterator copy = *this;
        ++(*this);
        return copy;
    }
    bool operator==(NeighborhoodEndIterator /*other*/) const { return is_ended(); }
    bool operator!=(NeighborhoodEndIterator /*other*/) const { return !is_ended(); }
    Pedestrian* operator*() const { return _cur_grid_it->value; }

private:
    const Grid2D<Pedestrian*>& _grid;

    const std::int32_t _start_idx, _start_idy;
    const std::int32_t _max_idx, _max_idy;

    std::int32_t _cur_idx, _cur_idy;

    std::function<bool(Point)> _filter;

    typename Grid2D<Pedestrian*>::it_type _cur_grid_it{}, _cur_grid_end{};

    void increment_grid_indices()
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

    void increment()
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

    bool is_it_valid() const
    {
        return _cur_grid_it != _cur_grid_end && !is_ended() &&
               _filter(_cur_grid_it->value->GetPos());
    }

    bool is_ended() const { return _cur_idx > _max_idx; }
};
