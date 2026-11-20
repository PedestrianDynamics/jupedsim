// SPDX-License-Identifier: LGPL-3.0-or-later
#include "geometry/geometry_builder.hpp"

#include "conversion.hpp"
#include "geometry/geometry.hpp" // IWYU pragma: keep
#include "id_and_name.hpp"
#include "kernel.hpp"
#include "line_segment.hpp"
#include "region.hpp"
#include "simulation_error.hpp"
#include "type_casters.hpp" // IWYU pragma: keep

#include <pybind11/pybind11.h>
#include <pybind11/stl.h> // IWYU pragma: keep

#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>
namespace py = pybind11;

namespace
{
/// A GeometryBuilder with the scope its region handles share.
struct ScopedGeometryBuilder {
    GeometryBuilder builder;
    std::shared_ptr<RegionScope> scope{std::make_shared<RegionScope>()};

    RegionHandle handle(std::size_t id, std::optional<std::string> name) const
    {
        return {scope, id, std::move(name)};
    }

    std::size_t own(const RegionHandle& region) const
    {
        if(region.scope != scope) {
            throw SimulationError(
                "Region {} belongs to another GeometryBuilder.",
                id_and_name(region.id, region.name));
        }
        return region.id;
    }
};
} // namespace

void init_geometry_builder(py::module_& m)
{
    py::classh<ScopedGeometryBuilder> builder(m, "GeometryBuilder");
    builder.doc() = clean_doc(R"(
        Assembles the walkable area of a simulation from regions.

        Flat regions (floors) are added with :meth:`add_region` and joined by inclined
        connectors (stairs, ramps) with :meth:`connect_regions`. :meth:`build` creates the
        :class:`~jupedsim.Geometry` that is passed to :class:`~jupedsim.Simulation`:

        .. code:: python

            geo_builder = jps.GeometryBuilder()
            room = geo_builder.add_region(
                boundary=[(0, 0), (10, 0), (10, 10), (0, 10)], height=0.0
            )
            simulation = jps.Simulation(
                model=jps.CollisionFreeSpeedModel(), geometry=geo_builder.build()
            )
    )");
    builder.def(py::init())
        .def(
            "add_region",
            [](ScopedGeometryBuilder& b,
               GeometryBuilder::Polygon polygon,
               double height,
               std::optional<std::string> name) {
                return b.handle(b.builder.add_region(std::move(polygon), height, name), name);
            },
            py::kw_only(),
            py::arg("polygon"),
            py::arg("height"),
            py::arg("name") = std::nullopt)
        // Sphinx autoapi renders only the last overload's docstring, so it covers both.
        .def(
            "add_region",
            [](ScopedGeometryBuilder& b,
               std::vector<Point> boundary,
               std::vector<std::vector<Point>> holes,
               double height,
               std::optional<std::string> name) {
                // Accept closed point lists
                py::detail::drop_closing_vertex(boundary);
                for(auto& hole : holes) {
                    py::detail::drop_closing_vertex(hole);
                }
                return b.handle(
                    b.builder.add_region({std::move(boundary), std::move(holes)}, height, name),
                    name);
            },
            py::kw_only(),
            py::arg("boundary"),
            py::arg("holes") = std::vector<std::vector<Point>>{},
            py::arg("height"),
            py::arg("name") = std::nullopt,
            clean_doc(R"(
                Add a flat region.

                The region is given either as ``polygon`` or as ``boundary`` with optional
                ``holes``. Rings are closed automatically; repeating the first point at the end
                is optional.

                Args:
                    polygon: Walkable area of the region; its interiors become holes.
                    boundary: Points of the outer boundary.
                    holes: Point lists of the holes, each strictly inside the boundary.
                    height: Height (z coordinate) of the region [m].
                    name: Unique among the regions of the geometry, if given. Used in
                        :meth:`~jupedsim.Geometry.region` and in messages.

                Returns:
                    Handle to the new region.

                Raises:
                    SimulationError: If the boundary or a hole is not simple or has fewer than
                        three distinct points, a hole does not lie strictly inside the boundary
                        or overlaps or touches another hole, walls of the region come closer
                        than 2.5 cm to each other, a coordinate or the height is not finite
                        or larger than 100 km in magnitude, or the region overlaps or touches a
                        region at the same height, or overlaps one less than 1.5 m above or
                        below it, or the name is already used.
            )")
                .c_str())
        .def(
            "connect_regions",
            [](ScopedGeometryBuilder& b,
               const RegionHandle& from_region,
               LineSegment from_edge,
               const RegionHandle& to_region,
               LineSegment to_edge,
               std::optional<std::string> name) {
                return b.handle(
                    b.builder.connect_regions(
                        b.own(from_region), from_edge, b.own(to_region), to_edge, name),
                    name);
            },
            py::kw_only(),
            py::arg("from_region"),
            py::arg("from_edge"),
            py::arg("to_region"),
            py::arg("to_edge"),
            py::arg("name") = std::nullopt,
            clean_doc(R"(
                Join two regions by a connector, e.g. a stair or a ramp.

                The connector is the planar quadrilateral spanned by one edge of each region.
                It is a region of its own and cannot be connected again. Unlike
                :meth:`add_region`, it does not check the headroom to other regions; only
                :meth:`build` rejects a connector that passes through another region.

                Args:
                    from_region: The first region.
                    from_edge: Edge of the first region's boundary or of one of its holes,
                        given by its two end points.
                    to_region: The second region.
                    to_edge: Edge of the second region's boundary or of one of its holes.
                    name: Unique among the regions of the geometry, if given.

                Returns:
                    Handle to the connector region.

                Raises:
                    SimulationError: If a region belongs to another builder or is a connector,
                        both regions are the same, an edge is not an edge of its region or
                        already carries a connector, the connector is not simple in the (x, y)
                        plane, does not lie outside of both regions, is not planar (the edges of
                        regions at different heights must be parallel), is too steep to walk, or
                        its walls come closer than 2.5 cm to each other, or the name is already
                        used.
            )")
                .c_str())
        .def(
            "build",
            [](ScopedGeometryBuilder& b) {
                std::shared_ptr<Geometry> geometry = b.builder.build();
                b.scope->geometry = geometry;
                return geometry;
            },
            clean_doc(R"(
                Create the geometry from the regions and connectors added so far.

                After success, the builder cannot be used any more, and its region handles
                refer to the geometry; after an error, it is left unchanged.

                Returns:
                    The :class:`~jupedsim.Geometry` to pass to :class:`~jupedsim.Simulation`.

                Raises:
                    SimulationError: If no region was added, two vertices of different
                        regions are closer than 2.5 cm, or the regions do not form one
                        connected walkable surface without parts passing through each other.
                        Regions cannot be removed; where fixing the error needs that, add the
                        geometry to a new builder.
            )")
                .c_str());
}
