// SPDX-License-Identifier: LGPL-3.0-or-later
#include "geometry/geometry_builder.hpp"

#include "conversion.hpp"
#include "geometry/geometry.hpp" // IWYU pragma: keep
#include "kernel.hpp"
#include "line_segment.hpp"
#include "type_casters.hpp" // IWYU pragma: keep

#include <pybind11/pybind11.h>
#include <pybind11/stl.h> // IWYU pragma: keep

#include <cstddef>
#include <utility>
#include <vector>
namespace py = pybind11;

void init_geometry_builder(py::module_& m)
{
    py::classh<GeometryBuilder> builder(m, "GeometryBuilder");
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
            [](GeometryBuilder& builder, GeometryBuilder::Polygon polygon, double height) {
                return builder.add_region(std::move(polygon), height);
            },
            py::kw_only(),
            py::arg("polygon"),
            py::arg("height"))
        // Sphinx autoapi renders only the last overload's docstring, so it covers both.
        .def(
            "add_region",
            [](GeometryBuilder& builder,
               std::vector<Point> boundary,
               std::vector<std::vector<Point>> holes,
               double height) {
                return builder.add_region({std::move(boundary), std::move(holes)}, height);
            },
            py::kw_only(),
            py::arg("boundary"),
            py::arg("holes") = std::vector<std::vector<Point>>{},
            py::arg("height"),
            clean_doc(R"(
                Add a flat region.

                The region is given either as ``polygon`` or as ``boundary`` with optional
                ``holes``.

                Args:
                    polygon: Walkable area of the region; its interiors become holes.
                    boundary: Points of the outer boundary.
                    holes: Point lists of the holes, each strictly inside the boundary.
                    height: Height (z coordinate) of the region [m].

                Returns:
                    Id of the new region.

                Raises:
                    SimulationError: If the boundary or a hole is not simple or has fewer than
                        three distinct points, a hole does not lie strictly inside the boundary
                        or overlaps or touches another hole, two vertices are closer than
                        2.5 cm, or a coordinate or the height is not finite. Overlapping or
                        touching regions at the same height are reported by :meth:`build`.
            )")
                .c_str())
        .def(
            "connect_regions",
            [](GeometryBuilder& builder,
               size_t from_region,
               LineSegment from_edge,
               size_t to_region,
               LineSegment to_edge) {
                return builder.connect_regions(from_region, from_edge, to_region, to_edge);
            },
            py::kw_only(),
            py::arg("from_region"),
            py::arg("from_edge"),
            py::arg("to_region"),
            py::arg("to_edge"),
            clean_doc(R"(
                Join two regions by a connector, e.g. a stair or a ramp.

                The connector is the planar quadrilateral spanned by one edge of each region.
                It is a region of its own and cannot be connected again.

                Args:
                    from_region: Id of the first region.
                    from_edge: Edge of the first region's boundary or of one of its holes,
                        given by its two end points.
                    to_region: Id of the second region.
                    to_edge: Edge of the second region's boundary or of one of its holes.

                Returns:
                    Id of the connector region.

                Raises:
                    SimulationError: If a region id is unknown or names a connector, both ids
                        are the same, an edge is not an edge of its region or already carries
                        a connector, or the connector is not simple in the (x, y) plane, does
                        not lie outside of both regions, is not planar (the edges of regions
                        at different heights must be parallel) or is too steep to walk.
            )")
                .c_str())
        .def(
            "build",
            &GeometryBuilder::build,
            clean_doc(R"(
                Create the geometry from the regions and connectors added so far.

                The builder is left unchanged and can be extended and built again.

                Returns:
                    The :class:`~jupedsim.Geometry` to pass to :class:`~jupedsim.Simulation`.

                Raises:
                    SimulationError: If no region was added, regions at the same height
                        overlap or touch, two vertices of different regions are closer than
                        2.5 cm, or the regions do not form one connected walkable surface
                        without parts passing through each other. Regions cannot be removed,
                        so after such an error the geometry has to be added to a new builder.
            )")
                .c_str());
}
