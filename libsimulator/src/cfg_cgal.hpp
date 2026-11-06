// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "into.hpp"
#include "kernel.hpp"

#include <CGAL/AABB_face_graph_triangle_primitive.h>
#include <CGAL/AABB_traits_3.h>
#include <CGAL/AABB_tree.h>
#include <CGAL/Boolean_set_operations_2.h> // IWYU pragma: keep
#include <CGAL/Constrained_Delaunay_triangulation_2.h>
#include <CGAL/Constrained_triangulation_2.h>
#include <CGAL/Constrained_triangulation_face_base_2.h>
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Gps_segment_traits_2.h>
#include <CGAL/Polygon_2.h>
#include <CGAL/Polygon_with_holes_2.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/Triangulation_data_structure_2.h>
#include <CGAL/Triangulation_vertex_base_2.h>
#include <fmt/format.h>

#include <concepts>
#include <cstddef>
#include <functional>
#include <ranges>
#include <sstream>
#include <string>

/// Convenience alias for vertex descriptor types
template <class G>
using VertexDescriptor = typename boost::graph_traits<G>::vertex_descriptor;

/// Convenience alias for face descriptor types
template <class G>
using FaceDescriptor = typename boost::graph_traits<G>::face_descriptor;

/// Convenience alias for half edge descriptor types
template <class G>
using HalfedgeDescriptor = typename boost::graph_traits<G>::halfedge_descriptor;

/// Convenience alias for face descriptor types
template <class G>
using EdgeDescriptor = typename boost::graph_traits<G>::edge_descriptor;

template <class Gt, class Fb = CGAL::Constrained_triangulation_face_base_2<Gt>>
class MyFace : public Fb
{
    bool _in{false};
    typedef Fb Base;
    typedef typename Fb::Triangulation_data_structure TDS;

public:
    using Fb::Fb;
    template <typename TDS2>
    struct Rebind_TDS {
        typedef typename Fb::template Rebind_TDS<TDS2>::Other Fb2;
        typedef MyFace<Gt, Fb2> Other;
    };
    void set_in_domain(bool v) { _in = v; }
    bool get_in_domain() const { return _in; }
};

#define DEFINE_KERNEL_TYPES(NS, KERNEL)                                                            \
    namespace NS                                                                                   \
    {                                                                                              \
    using Kernel = KERNEL;                                                                         \
    using FT = Kernel::FT;                                                                         \
    using Point2D = Kernel::Point_2;                                                               \
    using Point3D = Kernel::Point_3;                                                               \
    using Vector2D = Kernel::Vector_2;                                                             \
    using Vector3D = Kernel::Vector_3;                                                             \
    using Direction3D = Kernel::Direction_3;                                                       \
    using Line3D = Kernel::Line_3;                                                                 \
    using Line2D = Kernel::Line_2;                                                                 \
    using Plane3D = Kernel::Plane_3;                                                               \
    using Ray3D = Kernel::Ray_3;                                                                   \
    using Segment2D = Kernel::Segment_2;                                                           \
    using Segment3D = Kernel::Segment_3;                                                           \
    using Triangle2D = Kernel::Triangle_2;                                                         \
    using Triangle3D = Kernel::Triangle_3;                                                         \
    using Mesh = CGAL::Surface_mesh<Point3D>;                                                      \
    using Poly = CGAL::Polygon_2<Kernel>;                                                          \
    using PolyWithHoles = CGAL::Polygon_with_holes_2<Kernel>;                                      \
    using Traits = CGAL::Gps_segment_traits_2<Kernel>;                                             \
    using AABBPrimitive = CGAL::AABB_face_graph_triangle_primitive<Mesh>;                          \
    using AABBTraits = CGAL::AABB_traits_3<Kernel, AABBPrimitive>;                                 \
    using AABBTree = CGAL::AABB_tree<AABBTraits>;                                                  \
    using Vb = CGAL::Triangulation_vertex_base_2<Kernel>;                                          \
    using TDS = CGAL::Triangulation_data_structure_2<Vb, MyFace<Kernel>>;                          \
    using Itag = CGAL::Exact_predicates_tag;                                                       \
    using CDT = CGAL::Constrained_Delaunay_triangulation_2<Kernel, TDS, Itag>;                     \
    }

DEFINE_KERNEL_TYPES(epick, CGAL::Exact_predicates_inexact_constructions_kernel)
DEFINE_KERNEL_TYPES(epeck, CGAL::Exact_predicates_exact_constructions_kernel)
#undef DEFINE_KERNEL_TYPES

//=================================================================================================
// std::hash support
//=================================================================================================
namespace std
{
template <>
class hash<epick::PolyWithHoles>
{
public:
    size_t operator()(const epick::PolyWithHoles& p) const
    {
        std::stringstream ss{};
        ss << p;
        return std::hash<std::string>()(ss.str());
    }
};
} // namespace std

//=================================================================================================
// Free helper functions
//=================================================================================================
/// Projects @p point onto XY plane by dropping Z.
/// @tparam Kernel CGAL Kernel to use.
/// @param point to project.
/// @return projected @p point (x, y) of same Kernel.
template <typename Kernel>
CGAL::Point_2<Kernel> onto_xy(const CGAL::Point_3<Kernel>& point)
{
    return {point.x(), point.y()};
}

/// Fixes expected winding for a polygon with holes.
/// Expected winding is counter clockwise widing for the outer boundary and clockwise winding for
/// holes. Does not change the input polygon if winding is correct already.
/// @tparam Kernel the CGAL Kernel to use.
/// @param[in,out] polygon with holes to fixup.
template <typename Kernel>
void fixup_winding(CGAL::Polygon_2<Kernel>& polygon);

template <typename Kernel>
void fixup_winding(CGAL::Polygon_with_holes_2<Kernel>& polygon)
{
    fixup_winding(polygon.outer_boundary());
    for(auto& h : polygon.holes()) {
        if(h.is_counterclockwise_oriented())
            h.reverse_orientation();
    }
}

/// Fixes the winding of a polygon to counter clockwise.
/// Does not change the input polygon if winding is correct already.
/// @tparam Kernel the CGAL Kernel to use.
/// @param[in,out] polygon to fixup.
template <typename Kernel>
void fixup_winding(CGAL::Polygon_2<Kernel>& polygon)
{
    if(polygon.is_clockwise_oriented()) {
        polygon.reverse_orientation();
    }
}

/// Create range over all vertices in a polygon with holes.
/// @tparam Kernel the CGAL Kernel to use.
/// @param polygon with holes to create range from
/// @return range over all vertices
template <typename Kernel>
auto vertices(const CGAL::Polygon_with_holes_2<Kernel>& polygon)
{
    auto ring = [&polygon](std::size_t i) -> const auto& {
        return i == 0 ? polygon.outer_boundary().container() :
                        std::next(polygon.holes_begin(), i - 1)->container();
    };
    return std::views::iota(std::size_t{0}, polygon.number_of_holes() + 1) |
           std::views::transform(ring) | std::views::join;
}

/// Count all vertices in a polygon with holes.
/// @tparam Kernel the CGAL Kernel to use.
/// @param polygon with holes to count vertices of
/// @return number of vertices in the outer boundary and all holes
template <typename Kernel>
std::size_t vertex_count(const CGAL::Polygon_with_holes_2<Kernel>& polygon)
{
    std::size_t count = polygon.outer_boundary().size();
    for(const auto& hole : polygon.holes()) {
        count += hole.size();
    }
    return count;
}

//=================================================================================================
// Type Converters
//=================================================================================================
/// Converts an inexact CGAL point into a Point, exact.
template <>
struct Convert<epick::Point2D, Point> {
    static Point from(const epick::Point2D& p) { return Point{p}; }
};

/// Converts a Point into an inexact CGAL point, exact.
template <>
struct Convert<Point, epick::Point2D> {
    static epick::Point2D from(const Point& p) { return p.cgal(); }
};

/// Converts an exact CGAL point into a Point.
/// Rounds each coordinate to a double; exact if the coordinates are representable as doubles,
/// e.g. for points constructed from doubles.
template <>
struct Convert<epeck::Point2D, Point> {
    static Point from(const epeck::Point2D& p)
    {
        return {CGAL::to_double(p.x()), CGAL::to_double(p.y())};
    }
};

/// Converts a Point into an exact CGAL point, exact.
template <>
struct Convert<Point, epeck::Point2D> {
    static epeck::Point2D from(const Point& p) { return {p.x(), p.y()}; }
};

/// Converts a CGAL point between kernels.
/// Rounds each coordinate to a double; exact if the coordinates are representable as doubles,
/// e.g. for points constructed from doubles.
template <typename FromKernel, typename ToKernel>
    requires(!std::same_as<FromKernel, ToKernel>)
struct Convert<CGAL::Point_2<FromKernel>, CGAL::Point_2<ToKernel>> {
    static CGAL::Point_2<ToKernel> from(const CGAL::Point_2<FromKernel>& p)
    {
        return {CGAL::to_double(p.x()), CGAL::to_double(p.y())};
    }
};

/// Converts a CGAL polygon between kernels, keeps vertex order.
/// Coordinates are converted as for points.
template <typename FromKernel, typename ToKernel>
    requires(!std::same_as<FromKernel, ToKernel>)
struct Convert<CGAL::Polygon_2<FromKernel>, CGAL::Polygon_2<ToKernel>> {
    static CGAL::Polygon_2<ToKernel> from(const CGAL::Polygon_2<FromKernel>& polygon)
    {
        CGAL::Polygon_2<ToKernel> result{};
        result.container().reserve(polygon.size());
        for(const auto& p : polygon) {
            result.push_back(into<CGAL::Point_2<ToKernel>>(p));
        }
        return result;
    }
};

/// Converts a CGAL polygon with holes between kernels, keeps vertex and hole order.
/// Coordinates are converted as for points.
template <typename FromKernel, typename ToKernel>
    requires(!std::same_as<FromKernel, ToKernel>)
struct Convert<CGAL::Polygon_with_holes_2<FromKernel>, CGAL::Polygon_with_holes_2<ToKernel>> {
    static CGAL::Polygon_with_holes_2<ToKernel>
    from(const CGAL::Polygon_with_holes_2<FromKernel>& polygon)
    {
        using ToPolygon = CGAL::Polygon_2<ToKernel>;
        CGAL::Polygon_with_holes_2<ToKernel> result{into<ToPolygon>(polygon.outer_boundary())};
        for(const auto& hole : polygon.holes()) {
            result.add_hole(into<ToPolygon>(hole));
        }
        return result;
    }
};

//=================================================================================================
// Type Formatters
//=================================================================================================
template <typename K>
struct fmt::formatter<CGAL::Point_2<K>> {
    constexpr auto parse(fmt::format_parse_context& ctx) { return ctx.begin(); }

    auto format(const CGAL::Point_2<K>& p, fmt::format_context& ctx) const
    {
        const auto x = CGAL::to_double(p.x());
        const auto y = CGAL::to_double(p.y());
        return fmt::format_to(ctx.out(), "({}, {})", x, y);
    }
};

template <typename K>
struct fmt::formatter<CGAL::Point_3<K>> {
    constexpr auto parse(fmt::format_parse_context& ctx) { return ctx.begin(); }

    auto format(const CGAL::Point_3<K>& p, fmt::format_context& ctx) const
    {
        const auto x = CGAL::to_double(p.x());
        const auto y = CGAL::to_double(p.y());
        const auto z = CGAL::to_double(p.z());
        return fmt::format_to(ctx.out(), "({}, {}, {})", x, y, z);
    }
};
