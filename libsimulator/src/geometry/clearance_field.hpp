// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "line_segment.hpp"
#include "point.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

/// The clearance field is a precomputed field discretized into a grid of cells, which remains
/// immutable during the iteration. For each cell, it stores a lower bound on the distance to
/// the nearest wall or seam of the region.
///
/// When a query asks for walls in a certain radius, the clearance field can be used to
/// quickly determine if there are any walls or seams within that radius. If the clearance value for
/// a cell is greater than the query radius, the query can exit early without further
/// computation. Currently, the clearance field is used for the call walls_nearby or
/// walls_nearby_within_radius in the BoundaryIndex class, as well as the no_geometry_between filter
/// in the RoutingEngine class. By treating seams as walls, the clearance field still holds as a
/// lower bound on the distance to nearest wall in the neighboring region.
///
/// All fields are initialized to a maximum clearance value and by iterating over all segments of
/// the region, the clearance values are updated to reflect the actual distance bound to the nearest
/// segment.
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
    /// Throws SimulationError for an unknown @p region, and for a @p p outside the region's
    /// bounding box (NaN included).
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
