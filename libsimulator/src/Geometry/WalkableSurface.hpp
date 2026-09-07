// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "LineSegment.hpp"
#include "Point.hpp"
#include "CfgCgal.hpp"

#include <fmt/ranges.h>

#include <vector>

class WalkableSurface
{
public:
    using Ring = std::vector<Point>;
    struct Polygon {
        Ring Boundary;
        std::vector<Ring> Holes;
    };

private:
    struct Region {
        Polygon Polygon;
        double Height;
    };

    struct Connector {
        size_t fromRegion;
        LineSegment fromSegemnt;
        size_t toRegion;
        LineSegment toSegement;
    };

    std::vector<Region> regions{};
    std::vector<Connector> connectors{};

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
        return fmt::format_to(ctx.out(), "({}, {})", p.Boundary, p.Holes);
    }
};
