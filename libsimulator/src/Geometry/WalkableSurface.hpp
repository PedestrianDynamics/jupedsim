// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "CfgCgal.hpp"
#include "LineSegment.hpp"
#include "Point.hpp"

#include <boost/graph/adjacency_list.hpp>
#include <fmt/ranges.h>

#include <vector>

class WalkableSurface
{
public:
    using Ring = std::vector<Point>;
    struct Polygon {
        Ring boundary;
        std::vector<Ring> holes;
    };

private:
    struct Region {
        Polygon polygon;
        double height;
    };

    /// Seam for RegionGraph below
    struct Connector {
        LineSegment fromSegment;
        LineSegment toSegment;
    };

    using RegionGraph =
        boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS, Region, Connector>;

    RegionGraph _regionGraph{};

public:
    size_t AddRegion(Polygon polygon, double height);
    size_t ConnectRegions(size_t fromRegion, LineSegment from, size_t toRegion, LineSegment to);

    /// For debugging purposes
    std::unique_ptr<SurfaceMesh> CreateMesh();
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
