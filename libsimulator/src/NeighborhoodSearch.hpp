/// Copyright © 2012-2022 Forschungszentrum Jülich GmbH
/// SPDX-License-Identifier: LGPL-3.0-or-later
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

template <typename T>
class Grid2D
{
public:
    struct Entry {
        Grid2DIndex id;
        Point pos;
        T value;

        bool operator<(const Entry& other) const { return id < other.id; }
        bool operator==(const Entry& other) const { return id == other.id && pos == other.pos; }
    };

public:
    using value_type = T;
    using size_type = std::size_t;
    using container_type = std::vector<Entry>;
    using it_type = typename container_type::const_iterator;
    using it_pair = IteratorPair<it_type>;

    Grid2D() = default;

    Grid2D(container_type data) : _data(data)
    {
        // sort data
        std::sort(_data.begin(), _data.end());

        // create mapping
        if(!_data.empty()) {
            auto last_it = _data.cbegin();
            _data_mapping.emplace(last_it->id, it_pair(last_it, _data.cend()));
            for(auto it = _data.cbegin(); it != _data.cend(); ++it) {
                if(last_it->id == it->id) {
                    continue;
                }
                _data_mapping.erase(last_it->id);
                _data_mapping.emplace(last_it->id, it_pair(last_it, it));
                _data_mapping.emplace(it->id, it_pair(it, _data.cend()));
                last_it = it;
            }
        }
    }

    size_type size() const { return _data.size(); }

    bool empty() const { return _data.empty(); }

    it_pair get(Grid2DIndex index) const
    {
        auto it = _data_mapping.find(index);

        if(it != _data_mapping.cend()) {
            return it->second;
        }
        return {_data.cend(), _data.cend()};
    }

private:
    container_type _data;
    std::unordered_map<Grid2DIndex, it_pair> _data_mapping;
};

template <typename Value>
class NeighborhoodSearch
{
    using Grid = Grid2D<const Value*>;
    double _cellSize;
    Grid _grid{};

public:
    explicit NeighborhoodSearch(double cellSize) : _cellSize(cellSize){};

    void Update(const std::vector<Value>& items)
    {
        std::vector<typename Grid::Entry> values;
        values.reserve(items.size());
        std::transform(
            std::begin(items), std::end(items), std::back_inserter(values), [this](const auto& v) {
                auto idx = Grid2DIndex{
                    static_cast<std::int32_t>(v.pos.x / _cellSize),
                    static_cast<std::int32_t>(v.pos.y / _cellSize)};
                return typename Grid::Entry{idx, v.pos, &v};
            });
        _grid = Grid(values);
    }

    std::vector<const Value*> GetNeighboringAgents(Point pos, double radius) const
    {
        const int32_t pos_idx = static_cast<int32_t>(pos.x / _cellSize);
        const int32_t pos_idy = static_cast<int32_t>(pos.y / _cellSize);
        const int32_t nh_level = static_cast<int32_t>(std::ceil(radius / _cellSize));
        const int32_t x_min = pos_idx - nh_level;
        const int32_t x_max = pos_idx + nh_level;
        const int32_t y_min = pos_idy - nh_level;
        const int32_t y_max = pos_idy + nh_level;

        std::vector<const Value*> result{};
        for(int32_t x = x_min; x <= x_max; ++x) {
            for(int32_t y = y_min; y <= y_max; ++y) {
                for(const auto& item : _grid.get({x, y})) {
                    if(Distance(item.pos, pos) <= radius) {
                        result.emplace_back(item.value);
                    }
                }
            }
        }
        return result;
    }
};
