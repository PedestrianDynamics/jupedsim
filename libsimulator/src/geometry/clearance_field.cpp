// SPDX-License-Identifier: LGPL-3.0-or-later
#include "geometry/clearance_field.hpp"

#include "simulation_error.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <vector>

namespace
{
/// Taken off every stored bound. Covers float storage (relative error ~6e-8 on values of a few
/// metres) and points that round into a neighbouring cell on lookup.
constexpr double safety_margin = 1e-5;
} // namespace

ClearanceField::ClearanceField(
    const std::vector<std::vector<LineSegment>>& segments_per_region,
    double cell_size,
    double max_clearance)
    : _cell_size(cell_size)
{
    // Any point of a cell is at most half the cell diagonal away from the cell centre, so the
    // distance from the centre minus that is a bound for the whole cell.
    const double half_diagonal = cell_size * std::sqrt(0.5);
    const double reach = max_clearance + half_diagonal;

    _rasters.reserve(segments_per_region.size());
    for(const auto& segments : segments_per_region) {
        auto& raster = _rasters.emplace_back();
        if(segments.empty()) {
            continue;
        }
        double xmin = std::numeric_limits<double>::max();
        double ymin = std::numeric_limits<double>::max();
        double xmax = std::numeric_limits<double>::lowest();
        double ymax = std::numeric_limits<double>::lowest();
        for(const auto& s : segments) {
            xmin = std::min({xmin, s.p1.x, s.p2.x});
            ymin = std::min({ymin, s.p1.y, s.p2.y});
            xmax = std::max({xmax, s.p1.x, s.p2.x});
            ymax = std::max({ymax, s.p1.y, s.p2.y});
        }
        // The walls and seams enclose the region, so their bounding box covers all of it.
        raster.origin = {xmin, ymin};
        raster.width = static_cast<std::int64_t>(std::floor((xmax - xmin) / cell_size)) + 1;
        raster.height = static_cast<std::int64_t>(std::floor((ymax - ymin) / cell_size)) + 1;
        raster.values.assign(
            static_cast<std::size_t>(raster.width * raster.height),
            static_cast<float>(max_clearance));

        const auto to_index = [&](double v, double origin, std::int64_t size) {
            const auto i = static_cast<std::int64_t>(std::floor((v - origin) / cell_size));
            return std::clamp<std::int64_t>(i, 0, size - 1);
        };
        // Only cells whose centre lies within 'reach' of a segment can be closer than
        // 'max_clearance' to it, and all of those lie in the segment's box grown by 'reach'.
        for(const auto& s : segments) {
            const auto x0 = to_index(std::min(s.p1.x, s.p2.x) - reach, xmin, raster.width);
            const auto x1 = to_index(std::max(s.p1.x, s.p2.x) + reach, xmin, raster.width);
            const auto y0 = to_index(std::min(s.p1.y, s.p2.y) - reach, ymin, raster.height);
            const auto y1 = to_index(std::max(s.p1.y, s.p2.y) + reach, ymin, raster.height);
            for(auto y = y0; y <= y1; ++y) {
                for(auto x = x0; x <= x1; ++x) {
                    const Point centre{
                        xmin + (static_cast<double>(x) + 0.5) * cell_size,
                        ymin + (static_cast<double>(y) + 0.5) * cell_size};
                    const double bound = s.dist_to(centre) - half_diagonal - safety_margin;
                    auto& value = raster.values[static_cast<std::size_t>(y * raster.width + x)];
                    value = std::min(value, static_cast<float>(std::max(bound, 0.0)));
                }
            }
        }
    }
}

double ClearanceField::clearance(std::size_t region, Point p) const
{
    if(region >= _rasters.size()) {
        throw SimulationError("ClearanceField: region id {} out of range", region);
    }
    const auto& raster = _rasters[region];
    const double fx = std::floor((p.x - raster.origin.x) / _cell_size);
    const double fy = std::floor((p.y - raster.origin.y) / _cell_size);
    // Also rejects NaN: every comparison with it is false.
    if(!(fx >= 0 && fy >= 0 && fx < static_cast<double>(raster.width) &&
         fy < static_cast<double>(raster.height))) {
        throw SimulationError(
            "ClearanceField: point {} is outside the bounding box of region {}", p, region);
    }
    const auto x = static_cast<std::int64_t>(fx);
    const auto y = static_cast<std::int64_t>(fy);
    return raster.values[static_cast<std::size_t>(y * raster.width + x)];
}
