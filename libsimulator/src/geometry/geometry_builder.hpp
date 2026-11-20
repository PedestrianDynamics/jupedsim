// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "cfg_cgal.hpp"
#include "geometry/geometry.hpp"
#include "into.hpp"
#include "kernel.hpp"
#include "line_segment.hpp"

#include <boost/graph/adjacency_list.hpp>
#include <fmt/ranges.h>

#include <array>
#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <vector>

/// Assembles the walkable area of a simulation and creates the Geometry from it.
///
/// Flat regions (floors) are added with add_region() and joined by inclined connectors (stairs,
/// ramps) with connect_regions(). Each call validates its own input right away; build()
/// validates what needs all regions at once and creates the Geometry. After a successful
/// build(), the builder cannot be used any more. Regions cannot be removed.
///
/// Kernels: every stored vertex is an input double; the builder never constructs a point, and
/// connectors reuse the vertices of the regions they join. Vertices are therefore stored as
/// EPICK, whose predicates are exact on them. Algorithms that construct new points and test
/// them again (Boolean set operations, sweeps, arrangements) run in EPECK, as EPICK may crash
/// or give wrong answers there; converting with into<> is lossless both ways, since no stored
/// point is ever constructed. Should the builder start constructing points, e.g. to split edges
/// or merge regions, store EPECK instead and convert to EPICK only in build().
class GeometryBuilder
{
public:
    using Ring = std::vector<Point>;

    struct Polygon {
        Ring boundary;
        std::vector<Ring> holes;
    };

    GeometryBuilder();

    /// @p name, if given, is unique among the regions and connectors of the geometry.
    size_t add_region(Polygon polygon, double height, std::optional<std::string> name = {});

    size_t connect_regions(
        size_t from_region,
        LineSegment from,
        size_t to_region,
        LineSegment to,
        std::optional<std::string> name = {});

    /// Creates the geometry from the regions and connections added so far. After success, the
    /// builder cannot be used any more; after an error, it is left unchanged.
    /// Validates what needs the whole input: all vertices need a clearance of min_clearance, and
    /// the resulting surface must be walkable, free of faces passing through each other and
    /// connected.
    /// @throws SimulationError if the input does not form a valid geometry.
    std::unique_ptr<Geometry> build();

private:
    struct Region {
        // Store boundary (index 0) + holes as vector of indices into globalVertices
        std::vector<std::vector<size_t>> polygons;
        /// Empty for connectors, which are inclined instead of flat.
        std::optional<double> height;
        /// Polygons projected to x/y.
        epick::PolyWithHoles poly_with_holes;
        std::optional<std::string> name;

        /// Connectors may not be connected again.
        bool is_connectable() const { return height.has_value(); }
    };

    /// Global vertex IDs of the shared edge of connected regions.
    using Seam = std::array<size_t, 2>;

    using RegionGraph =
        boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS, Region, Seam>;

    /// Minimum distance in meters between the walls of a region or connector, and between any
    /// two vertices of the geometry in 3D.
    static constexpr double min_clearance{0.025};
    /// Largest absolute coordinate or height in meters, to avoid a loss in precision.
    static constexpr double max_coordinate{1e5};
    /// Sine of the largest angle between the seams of a connector between two heights
    /// (about 0.01 degrees, i.e. 1.7 mm over 10 m).
    static constexpr double max_seam_angle_sine{1.75e-4};
    /// Heights closer than this in meters count as the same height, so that float noise like
    /// 0.1 + 0.2 != 0.3 neither lets floors overlap nor tilts a flat connector.
    static constexpr double height_tolerance{1e-6};
    /// Smallest height difference in meters between floors above each other.
    static constexpr double min_headroom{1.5};

    std::array<size_t, 2> find_edge(size_t region_id, const LineSegment& edge) const;
    std::unique_ptr<Geometry::RegionGraph2D> create_region_graph_2d() const;
    /// Throws if a new floor overlaps or touches a floor at the same height (up to
    /// height_tolerance), or overlaps one less than min_headroom above or below it.
    void check_against_floors(const epick::PolyWithHoles& poly_with_holes, double height) const;
    void throw_if_built() const;
    void throw_if_name_used(const std::optional<std::string>& name) const;
    /// id_and_name() of region @p region_id.
    std::string label(size_t region_id) const;

    std::vector<epick::Point3D> _global_vertices;
    RegionGraph _region_graph{};
    bool _built{false};
};

/// Converts a Polygon into a CGAL polygon with holes.
/// Copies the rings verbatim: they are neither checked nor reoriented, so the result only has the
/// winding CGAL expects if @p polygon has it.
template <typename Kernel>
struct Convert<GeometryBuilder::Polygon, CGAL::Polygon_with_holes_2<Kernel>> {
    static CGAL::Polygon_with_holes_2<Kernel> from(const GeometryBuilder::Polygon& polygon)
    {
        const auto convert_ring = [](const GeometryBuilder::Ring& ring) {
            CGAL::Polygon_2<Kernel> converted{};
            for(const Point& point : ring) {
                converted.push_back(into<CGAL::Point_2<Kernel>>(point));
            }
            return converted;
        };

        std::vector<CGAL::Polygon_2<Kernel>> holes{};
        holes.reserve(polygon.holes.size());
        for(const auto& hole : polygon.holes) {
            holes.emplace_back(convert_ring(hole));
        }
        return CGAL::Polygon_with_holes_2<Kernel>(
            convert_ring(polygon.boundary), std::begin(holes), std::end(holes));
    }
};

template <>
struct fmt::formatter<GeometryBuilder::Polygon> {
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const GeometryBuilder::Polygon& p, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), "({}, {})", p.boundary, p.holes);
    }
};
