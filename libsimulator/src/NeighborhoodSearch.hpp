// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once
#include "GenericAgent.hpp"
#include "HashCombine.hpp"
#include "Point.hpp"

#include <algorithm>
#include <cmath>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iterator>
#include <unordered_map>
#include <vector>

template <typename T>
T* as_ptr(T* t)
{
    return t;
}

template <typename T>
const T* as_ptr(const T* t)
{
    return t;
}

template <typename T>
T* as_ptr(T& t)
{
    return &t;
}

template <typename T>
const T* as_ptr(const T& t)
{
    return &t;
}

struct Grid2DIndex {
    std::int32_t idx;
    std::int32_t idy;

    bool operator<(const Grid2DIndex& other) const
    {
        return idx < other.idx || (idx == other.idx && idy < other.idy);
    }

    bool operator==(const Grid2DIndex& other) const { return idx == other.idx && idy == other.idy; }
};

template <>
struct std::hash<Grid2DIndex> {
    std::size_t operator()(const Grid2DIndex& id) const noexcept
    {
        std::hash<std::int32_t> hasher{};
        return jps::hash_combine(hasher(id.idx), hasher(id.idy));
    }
};

template <typename Value>
class NeighborhoodSearch
{
    using Grid = std::unordered_map<Grid2DIndex, std::vector<const Value*>>;

    double _cell_size;
    Grid _grid{};

private:
    Grid2DIndex get_index(const Point& pos) const
    {
        const int32_t idx = static_cast<int32_t>(pos.x / _cell_size);
        const int32_t idy = static_cast<int32_t>(pos.y / _cell_size);
        return Grid2DIndex{idx, idy};
    }

public:
    explicit NeighborhoodSearch(double cell_size) : _cell_size(cell_size) {};

    void add_agent(const Value& item)
    {
        auto index = get_index(item.location.xy());
        auto& vec = _grid[index];
        vec.push_back(&item);
    }

    void remove_agent(const Value& item)
    {
        for(auto& [_, agents] : _grid) {
            const auto iter =
                std::find_if(std::begin(agents), std::end(agents), [item](auto& agent) {
                    return agent.id == item.id;
                });
            if(iter != std::end(agents)) {
                agents.erase(iter);
                return;
            }
        }
        throw SimulationError("Unknown agent id {}", item.id);
    }

    void update(const AgentContainer<Value>& items)
    {
        _grid.clear();
        for(const auto& item : items) {
            auto index = get_index(item.location.xy());
            auto& vec = _grid[index];
            vec.push_back(&item);
        }
    }

    /// Calls 'fn' for every item within 'radius' of 'pos'.
    template <std::invocable<const Value&> Fn>
    void for_each_in_range(Point pos, double radius, Fn&& fn) const
    {
        const auto pos_idx = get_index(pos);
        const auto offset = static_cast<int32_t>(std::ceil(radius / _cell_size));
        const int32_t x_min = pos_idx.idx - offset;
        const int32_t x_max = pos_idx.idx + offset;
        const int32_t y_min = pos_idx.idy - offset;
        const int32_t y_max = pos_idx.idy + offset;

        const auto radius_squared = radius * radius;

        for(int32_t x = x_min; x <= x_max; ++x) {
            for(int32_t y = y_min; y <= y_max; ++y) {
                auto it = _grid.find({x, y});
                if(it != _grid.cend()) {
                    for(const auto& item : it->second) {
                        if(distance_squared(item->location.xy(), pos) <= radius_squared) {
                            fn(*item);
                        }
                    }
                }
            }
        }
    }
};
