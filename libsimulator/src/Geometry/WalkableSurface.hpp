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

    size_t AddRegion(Polygon polygon, double height);
    size_t ConnectRegions(size_t fromRegion, LineSegment from, size_t toRegion, LineSegment to);

    /// For debugging purposes
    std::unique_ptr<SurfaceMesh> CreateMesh();

private:
    std::vector<Point3D> _globalVertices;

    struct Region {
        // Store boundary (index 0) + holes as vector of indices into globalVertices
        std::vector<std::vector<size_t>> polygons;
        bool connectable = false; // E.g. right now do not allow to connect to Connectors
    };

    using RegionGraph = boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS, Region>;
    RegionGraph _regionGraph{};

    size_t FindVertex(size_t regionId, const Point& p) const;
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
