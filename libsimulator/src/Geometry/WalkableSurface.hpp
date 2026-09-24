// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "Geometry/Geometry.hpp"

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

    size_t AddRegion(Polygon polygon, double height);
    size_t AddRegion(const PolyWithHoles& polygon, double height);
    size_t ConnectRegions(size_t fromRegion, LineSegment from, size_t toRegion, LineSegment to);

    using RegionGraph2D = Geometry::RegionGraph2D;
    std::unique_ptr<RegionGraph2D> CreateRegionGraph2D() const;

    std::unique_ptr<Geometry> CreateGeometry();

private:
    std::vector<Point3D> _globalVertices;

    struct Region {
        // Store boundary (index 0) + holes as vector of indices into globalVertices
        std::vector<std::vector<size_t>> polygons;
        /// Empty for connectors, which are inclined instead of flat.
        std::optional<double> height;
        /// Polygons projected to x/y.
        PolyWithHoles polyWithHoles;

        /// Connectors may not be connected again.
        bool is_connectable() const { return height.has_value(); }
    };

    /// Global vertex IDs of the shared edge of connected regions.
    using Seam = std::array<size_t, 2>;

    using RegionGraph =
        boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS, Region, Seam>;
    RegionGraph _regionGraph{};

    size_t insert_region(
        std::vector<std::vector<size_t>> polygons,
        const std::vector<Point3D>& vertices,
        double height);

    std::array<size_t, 2> FindEdge(size_t regionId, const LineSegment& edge) const;

    /// Check whether floors at specified height overlap. Throws in case of error.
    void ValidateFloorOverlap(const PolyWithHoles& polyWithHoles, double height) const;
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
