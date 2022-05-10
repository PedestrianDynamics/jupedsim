/**
 * \copyright   <2009-2020> Forschungszentrum Jülich GmbH. All rights reserved.
 *
 * \section License
 * This file is part of JuPedSim.
 *
 * JuPedSim is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * JuPedSim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with JuPedSim. If not, see <http://www.gnu.org/licenses/>.
 *
 **/

#pragma once

#include "HashCombine.hpp"
#include "IteratorPair.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <unordered_map>
#include <utility>
#include <vector>

struct Grid2DIndex {
    std::int32_t idx, idy;

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
    struct IndexValuePair {
        Grid2DIndex id;
        T value;

        bool operator<(const IndexValuePair& other) const { return id < other.id; }
        bool operator==(const IndexValuePair& other) const
        {
            return id == other.id && value == other.value;
        }
    };

public:
    using value_type = T;
    using size_type = std::size_t;
    using container_type = std::vector<IndexValuePair>;
    using it_type = typename container_type::const_iterator;
    using it_pair = IteratorPair<it_type>;

    Grid2D() = default;

    Grid2D(const Grid2D&) = default;
    Grid2D(Grid2D&&) = default;

    Grid2D& operator=(const Grid2D&) = default;
    Grid2D& operator=(Grid2D&&) = default;

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
