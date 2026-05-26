// SPDX-License-Identifier: LGPL-3.0-or-later
#include "AStarRoutingEngine.hpp"
#include "CollisionGeometry.hpp"
#include "RoutingEngine.hpp"
#include "conversion.hpp"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h> // IWYU pragma: keep
#include <pybind11/trampoline_self_life_support.h>

#include <cstddef>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

namespace py = pybind11;

// Type caster: According to https://pybind11.readthedocs.io/en/stable/advanced/cast/custom.html
// which coincidentally also shows a conversion for Point2D. :)
namespace pybind11
{
namespace detail
{
template <>
struct type_caster<Point> {
    PYBIND11_TYPE_CASTER(Point, const_name("tuple[float, float]"));

    bool load(handle src, bool)
    {
        if(!isinstance<sequence>(src)) {
            return false;
        }
        auto seq = reinterpret_borrow<sequence>(src);
        if(seq.size() != 2) {
            return false;
        }
        value.x = seq[0].cast<double>();
        value.y = seq[1].cast<double>();
        return true;
    }

    static handle cast(const Point& src, return_value_policy, handle)
    {
        return make_tuple(src.x, src.y).release();
    }
};
} // namespace detail
} // namespace pybind11

namespace
{
// Trampoline that lets Python subclasses of RoutingEngine override the virtual methods. Link for
// details:
// https://pybind11.readthedocs.io/en/stable/advanced/classes.html#overriding-virtual-functions-in-python
class PyRoutingEngine : public RoutingEngine, public py::trampoline_self_life_support
{
public:
    using RoutingEngine::RoutingEngine;

    std::string name() const override { PYBIND11_OVERRIDE_PURE(std::string, RoutingEngine, name); }

    void set_geometry(const CollisionGeometry& geometry) override
    {
        PYBIND11_OVERRIDE_PURE(void, RoutingEngine, set_geometry, geometry);
    }

    std::vector<Point> compute_waypoints(Point from, Point destination) override
    {
        PYBIND11_OVERRIDE_PURE(
            std::vector<Point>, RoutingEngine, compute_waypoints, from, destination);
    }

    bool is_routable(Point p) const override
    {
        PYBIND11_OVERRIDE_PURE(bool, RoutingEngine, is_routable, p);
    }
};
} // namespace

void init_routing(py::module_& m)
{
    py::class_<RoutingEngine, PyRoutingEngine, py::smart_holder>(
        m,
        "RoutingEngine",
        R"doc(Abstract base class for routing engines.

This also works for Python classes due to pybind11 trampoline.)doc")
        .def(py::init<>())
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
                engine.set_geometry(geometry);
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
                return intoTuples(engine.compute_waypoints(intoPoint(from), intoPoint(to)));
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
                return engine.is_routable(intoPoint(point));
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
