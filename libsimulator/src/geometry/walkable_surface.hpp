// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "geometry/geometry.hpp"

#include <boost/graph/adjacency_list.hpp>
#include <fmt/ranges.h>

#include <array>
#include <optional>
#include <vector>

class Geometry;

class WalkableSurface
{
public:
    using Ring = std::vector<Point>;
    struct Polygon {
        Ring boundary;
        std::vector<Ring> holes;
    };

    size_t add_region(Polygon polygon, double height);
    size_t add_region(const PolyWithHoles& polygon, double height);
    size_t connect_regions(size_t from_region, LineSegment from, size_t to_region, LineSegment to);

    using RegionGraph2D = Geometry::RegionGraph2D;
    std::unique_ptr<RegionGraph2D> create_region_graph_2d() const;

    std::unique_ptr<Geometry> create_geometry();

private:
    std::vector<Point3D> _global_vertices;

    struct Region {
        // Store boundary (index 0) + holes as vector of indices into globalVertices
        std::vector<std::vector<size_t>> polygons;
        /// Empty for connectors, which are inclined instead of flat.
        std::optional<double> height;
        /// Polygons projected to x/y.
        PolyWithHoles poly_with_holes;

        /// Connectors may not be connected again.
        bool is_connectable() const { return height.has_value(); }
    };

    /// Global vertex IDs of the shared edge of connected regions.
    using Seam = std::array<size_t, 2>;

    using RegionGraph =
        boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS, Region, Seam>;
    RegionGraph _region_graph{};

    size_t insert_region(
        std::vector<std::vector<size_t>> polygons,
        const std::vector<Point3D>& vertices,
        double height);

    std::array<size_t, 2> find_edge(size_t region_id, const LineSegment& edge) const;

    /// Check whether floors at specified height overlap. Throws in case of error.
    void validate_floor_overlap(const PolyWithHoles& poly_with_holes, double height) const;
};

template <>
struct fmt::formatter<WalkableSurface::Polygon> {
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const WalkableSurface::Polygon& p, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), "({}, {})", p.boundary, p.holes);
    }
};
