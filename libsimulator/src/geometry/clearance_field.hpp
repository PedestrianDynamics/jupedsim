// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "line_segment.hpp"
#include "point.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

/// Per region, a raster holding for every cell a lower bound on the distance from any point of
/// that cell to the nearest line segment (wall or seam) of the region.
///
/// It lets geometric queries exit early: a query that only looks within some radius of an agent
/// has nothing to find while the region's segments are all farther away than that radius.
///
/// The bound is capped at 'max_clearance': further away nothing is recorded, a cell that far
/// from every segment simply holds 'max_clearance'. Queries with a larger radius never exit
/// early and take their full path.
class ClearanceField
{
public:
    static constexpr double default_cell_size = 0.5;
    static constexpr double default_max_clearance = 5.0;

    ClearanceField() = default;
    /// @param segments_per_region all walls and seams of each region, indexed by region id
    explicit ClearanceField(
        const std::vector<std::vector<LineSegment>>& segments_per_region,
        double cell_size = default_cell_size,
        double max_clearance = default_max_clearance);

    /// Lower bound on the distance from @p p to the nearest segment of region @p region.
    /// Zero wherever nothing is known, i.e. outside the region's bounding box.
    double clearance(std::size_t region, Point p) const;

private:
    struct Raster {
        Point origin{};
        std::int64_t width{};
        std::int64_t height{};
        /// Row-major, width * height entries.
        std::vector<float> values{};
    };

    double _cell_size{default_cell_size};
    std::vector<Raster> _rasters{};
};
