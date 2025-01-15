// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "HashCombine.hpp"
#include "IteratorPair.hpp"
#include "Point.hpp"

#include <algorithm>
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
    using Grid = std::unordered_map<Grid2DIndex, std::vector<Value>>;

    double _cellSize;
    Grid _grid{};

private:
    Grid2DIndex getIndex(const Point& pos) const
    {
        const int32_t idx = static_cast<int32_t>(pos.x / _cellSize);
        const int32_t idy = static_cast<int32_t>(pos.y / _cellSize);
        return Grid2DIndex{idx, idy};
    }

public:
    explicit NeighborhoodSearch(double cellSize) : _cellSize(cellSize){};

    void AddAgent(const Value& item)
    {
        auto index = getIndex(item.pos);
        auto& vec = _grid[index];
        vec.push_back(item);
    }

    void RemoveAgent(const Value& item)
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

    void Update(const std::vector<Value>& items)
    {
        _grid.clear();
        for(const auto& item : items) {
            auto index = getIndex(item.pos);
            auto& vec = _grid[index];
            vec.push_back(item);
        }
    }

    std::vector<Value> GetNeighboringAgents(Point pos, double radius) const
    {
        std::vector<Value> result{};
        result.reserve(128);

        const auto posIdx = getIndex(pos);
        const auto offset = static_cast<int32_t>(std::ceil(radius / _cellSize));
        const int32_t xMin = posIdx.idx - offset;
        const int32_t xMax = posIdx.idx + offset;
        const int32_t yMin = posIdx.idy - offset;
        const int32_t yMax = posIdx.idy + offset;

        const auto radiusSquared = radius * radius;

        for(int32_t x = xMin; x <= xMax; ++x) {
            for(int32_t y = yMin; y <= yMax; ++y) {
                auto it = _grid.find({x, y});
                if(it != _grid.cend()) {
                    for(const auto& item : it->second) {
                        if(DistanceSquared(item.pos, pos) <= radiusSquared) {
                            result.emplace_back(item);
                        }
                    }
                }
            }
        }
        return result;
    }
};
