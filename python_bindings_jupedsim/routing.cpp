// SPDX-License-Identifier: LGPL-3.0-or-later
#include "AStarRoutingEngine.hpp"
#include "CollisionGeometry.hpp"
#include "RoutingEngine.hpp"
#include "conversion.hpp"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h> // IWYU pragma: keep

#include <cstddef>
#include <memory>
#include <tuple>
#include <vector>

namespace py = pybind11;

void init_routing(py::module_& m)
{
    py::class_<RoutingEngine, py::smart_holder>(
        m,
        "RoutingEngine",
        R"doc(Abstract base class for routing engines.

Exposed so that concrete engines such as :class:`AStarRoutingEngine` appear
as a ``RoutingEngine`` subtype.
)doc")
        .def_property_readonly(
            "id",
            [](const RoutingEngine& e) { return e.Id().getID(); },
            "Unique identifier of this routing engine instance.")
        .def_property_readonly(
            "name",
            [](const RoutingEngine& e) { return e.name(); },
            "Name of the routing engine implementation.")
        .def(
            "set_geometry",
            [](RoutingEngine& engine, const CollisionGeometry& geometry) {
                engine.SetGeometry(geometry);
            },
            py::arg("geometry"),
            R"doc(Bind this routing engine to *geometry*.

Must be called before any routing query (:meth:`compute_waypoints` /
:meth:`is_routable`) is called. This is called automatically by
:class:`~jupedsim.Simulation` objects.
)doc")
        .def(
            "compute_waypoints",
            [](RoutingEngine& engine,
               std::tuple<double, double> from,
               std::tuple<double, double> to) {
                return intoTuples(engine.ComputeAllWaypoints(intoPoint(from), intoPoint(to)));
            },
            py::arg("frm"),
            py::arg("to"),
            R"doc(Computes shortest path between specified points.

Arguments:
    frm: point from which to find the shortest path
    to: point to which to find the shortest path

Returns:
    List of points (path) from 'frm' to 'to' including from and to.
)doc")
        .def(
            "is_routable",
            [](const RoutingEngine& engine, std::tuple<double, double> point) {
                return engine.IsRoutable(intoPoint(point));
            },
            py::arg("point"),
            R"doc(Tests if the supplied point is inside the underlying geometry.

Returns:
    If the point is inside the geometry.
)doc");

    py::class_<AStarRoutingEngine, RoutingEngine, py::smart_holder>(
        m,
        "AStarRoutingEngine",
        R"doc(A* shortest-path routing on a navigation mesh.

Default-construct object. Either pass to :class:`Simulation` (which calls
:meth:`set_geometry` automatically) or call :meth:`set_geometry` explicitly
before performing any routing query.
)doc")
        .def(py::init<>(), "Default-construct an engine without a geometry.")
        .def(
            "mesh",
            [](const AStarRoutingEngine& routingEngine) {
                const auto mesh = routingEngine.MeshData();
                if(!mesh) {
                    throw std::runtime_error(
                        "AStarRoutingEngine has no geometry; call set_geometry first");
                }
                const auto polygonCount = mesh->CountPolygons();
                using Ind = decltype(mesh->Polygons(0).vertices);
                std::vector<Ind> polys;
                polys.reserve(polygonCount);
                for(size_t index = 0; index < polygonCount; ++index) {
                    polys.emplace_back(mesh->Polygons(index).vertices);
                }
                return std::make_tuple(intoTuples(mesh->FVertices()), polys);
            },
            R"doc(Access the navigation mesh geometry.

The navigation mesh is store as a collection of convex polygons in CCW order.

The returned data is to be interpreted as:

.. code::

    tuple[
        list[tuple[float, float]], # All vertices in this mesh.
        list[ # List of polygons
            list[int] # List of indices into the vertices that compose this polygon in CCW order
        ]
    ]

Returns:
    A tuple of vertices and list of polygons which in turn are a list of indices
    tuple[list[tuple[float, float]],list[list[int]]]
)doc");
}
