#pragma once

#include "Grid2D.hpp"
#include "Point.hpp"

#include <cstdint>
#include <functional>

class Agent;

class NeighborhoodEndIterator
{
};

class NeighborhoodIterator
{
public:
    NeighborhoodIterator(
        const Grid2D<Agent*>& grid,
        std::int32_t idx,
        std::int32_t idy,
        std::int32_t max_idx,
        std::int32_t max_idy,
        std::function<bool(Point)> filter = [](Point) { return true; });
    NeighborhoodIterator& operator++();
    NeighborhoodIterator operator++(int);
    bool operator==(NeighborhoodEndIterator other) const;
    bool operator!=(NeighborhoodEndIterator other) const;
    Agent* operator*() const;

private:
    const Grid2D<Agent*>& _grid;
    const std::int32_t _start_idx, _start_idy;
    const std::int32_t _max_idx, _max_idy;
    std::int32_t _cur_idx, _cur_idy;
    std::function<bool(Point)> _filter;
    typename Grid2D<Agent*>::it_type _cur_grid_it{}, _cur_grid_end{};

    void increment_grid_indices();
    void increment();
    bool is_it_valid() const;
    bool is_ended() const;
};
