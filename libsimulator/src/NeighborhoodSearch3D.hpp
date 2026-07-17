// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "CfgCgal.hpp"
#include "HashCombine.hpp"

#include <cmath>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <ranges>
#include <unordered_map>
#include <vector>

struct Grid3DIndex {
    std::int32_t idx;
    std::int32_t idy;
    std::int32_t idz;

    bool operator==(const Grid3DIndex& other) const = default;
};

template <>
struct std::hash<Grid3DIndex> {
    std::size_t operator()(const Grid3DIndex& id) const noexcept
    {
        std::hash<std::int32_t> hasher{};
        return jps::hash_combine(jps::hash_combine(hasher(id.idx), hasher(id.idy)), hasher(id.idz));
    }
};

/// Filter horizontally within provided radius.
inline auto within_horizontal_distance(const Point3D& p, double radius)
{
    return [x = p.x(), y = p.y(), rSquared = radius * radius](const Point3D& other) {
        const auto dx = other.x() - x;
        const auto dy = other.y() - y;
        return dx * dx + dy * dy <= rSquared;
    };
}

/// Filter z against provided height.
inline auto within_vertical_band(const Point3D& p, double height)
{
    return [z = p.z(), height](const Point3D& other) { return std::abs(other.z() - z) < height; };
}

class NeighborhoodSearch3D
{
    double _cellSize;
    std::unordered_map<Grid3DIndex, std::vector<std::size_t>> _grid{};

    Grid3DIndex get_index(const Point3D& p) const
    {
        return {
            static_cast<std::int32_t>(p.x() / _cellSize),
            static_cast<std::int32_t>(p.y() / _cellSize),
            static_cast<std::int32_t>(p.z() / _cellSize)};
    }

public:
    explicit NeighborhoodSearch3D(double cellSize) : _cellSize(cellSize) {}

    /// Rebuild index cache with provided positions. Indices returned by
    /// candidates() refer to the enumeration order of `positions`.
    template <std::ranges::input_range R>
        requires std::same_as<std::ranges::range_value_t<R>, Point3D>
    void rebuild_index(R&& positions)
    {
        _grid.clear();
        std::size_t i = 0;
        for(const Point3D& p : positions) {
            _grid[get_index(p)].push_back(i++);
        }
    }

    /// Quick, broad check within "cylinder" with radius r horizontally (x/y positions) and
    /// height (against z position). This will set the list of candidates which will be filtered
    /// more afterwards. The list is not exact as we work on grid cells and do not form an exact
    /// cylinder. Additionally we do not do any visibility checks.
    std::vector<std::size_t> candidates(const Point3D& p, double radius, double height) const
    {
        std::vector<std::size_t> result{};
        result.reserve(128);

        const auto center = get_index(p);
        const auto rOffset = static_cast<std::int32_t>(std::ceil(radius / _cellSize));
        const auto hOffset = static_cast<std::int32_t>(std::ceil(height / _cellSize));

        for(std::int32_t x = center.idx - rOffset; x <= center.idx + rOffset; ++x) {
            for(std::int32_t y = center.idy - rOffset; y <= center.idy + rOffset; ++y) {
                for(std::int32_t z = center.idz - hOffset; z <= center.idz + hOffset; ++z) {
                    if(const auto it = _grid.find({x, y, z}); it != _grid.cend()) {
                        result.insert(result.end(), it->second.cbegin(), it->second.cend());
                    }
                }
            }
        }
        return result;
    }
};
