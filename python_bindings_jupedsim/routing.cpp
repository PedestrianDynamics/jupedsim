// SPDX-License-Identifier: LGPL-3.0-or-later
#include "AStarRoutingEngine.hpp"
#include "CollisionGeometry.hpp"
#include "RoutingEngine.hpp"
#include "conversion.hpp"
#include "python_routing_engine_proxy.hpp"
#include "routing_factory.hpp"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h> // IWYU pragma: keep

#include <cstddef>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <tuple>
#include <vector>

namespace py = pybind11;

void init_routing(py::module_& m)
{
    // Abstract base — cannot be instantiated from Python directly.
    // Exposed so that AStarRoutingEngine appears as a RoutingEngine subtype.
    py::class_<RoutingEngine>(m, "RoutingEngine")
        .def_property_readonly("id", [](const RoutingEngine& e) { return e.Id().getID(); })
        .def_property_readonly("name", [](const RoutingEngine& e) { return e.name(); });

    py::class_<AStarRoutingEngine, RoutingEngine>(m, "AStarRoutingEngine")
        .def(py::init([](const CollisionGeometry& geo) {
            return std::make_unique<AStarRoutingEngine>(geo.Polygon());
        }))
        .def(
            "compute_waypoints",
            [](AStarRoutingEngine& engine,
               std::tuple<double, double> from,
               std::tuple<double, double> to) {
                return intoTuples(engine.ComputeAllWaypoints(intoPoint(from), intoPoint(to)));
            })
        .def(
            "is_routable",
            [](AStarRoutingEngine& engine, std::tuple<double, double> point) {
                return engine.IsRoutable(intoPoint(point));
            })
        .def("mesh", [](const AStarRoutingEngine& routingEngine) {
            const auto mesh = routingEngine.MeshData();
            const auto polygonCount = mesh->CountPolygons();
            using Ind = decltype(mesh->Polygons(0).vertices);
            std::vector<Ind> polys;
            polys.reserve(polygonCount);
            for(size_t index = 0; index < polygonCount; ++index) {
                polys.emplace_back(mesh->Polygons(index).vertices);
            }
            return std::make_tuple(intoTuples(mesh->FVertices()), polys);
        });

    // Opaque handle to a C++ routing-engine factory.  Python code receives
    // these from factory-creation functions below and passes them straight to
    // Simulation.switch_routing_algorithm without ever touching the engine.
    py::class_<RoutingFactory>(m, "RoutingEngineFactory");

    m.def("astar_routing_factory", []() {
        return RoutingFactory{
            [](const PolyWithHoles& poly) -> std::unique_ptr<RoutingEngine> {
                return std::make_unique<AStarRoutingEngine>(poly);
            }};
    });

    // Generic adapter: wrap any Python callable factory(geometry) -> RoutingEngine.
    // The Python engine is kept alive by PythonRoutingEngineProxy, which holds
    // a strong py::object reference for as long as the simulation owns the engine.
    m.def("python_routing_factory", [](py::object pyfactory) {
        if(!PyCallable_Check(pyfactory.ptr())) {
            throw std::invalid_argument(
                "python_routing_factory expects a callable factory(geometry) -> RoutingEngine");
        }
        return RoutingFactory{
            [pyfactory](const PolyWithHoles& poly) -> std::unique_ptr<RoutingEngine> {
                py::gil_scoped_acquire lock{};
                return std::make_unique<PythonRoutingEngineProxy>(
                    pyfactory(CollisionGeometry{poly}));
            }};
    });
}
