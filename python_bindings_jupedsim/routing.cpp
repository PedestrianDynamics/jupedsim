// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Geometry/Geometry2D.hpp"
#include "RoutingEngine.hpp"
#include "conversion.hpp"

#include <glm/ext/vector_float2.hpp>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h> // IWYU pragma: keep

#include <cstddef>
#include <cstdint>
#include <memory>
#include <tuple>
#include <vector>

namespace py = pybind11;

void init_routing(py::module_& m)
{
    py::class_<RoutingEngine>(m, "RoutingEngine")
        .def(py::init([](const Geometry2D& geo) {
            return std::make_unique<RoutingEngine>(geo.Polygon());
        }))
        .def(
            "compute_waypoints",
            [](RoutingEngine& engine,
               std::tuple<double, double> from,
               std::tuple<double, double> to) {
                return intoTuples(engine.ComputeAllWaypoints(intoPoint(from), intoPoint(to)));
            })
        .def(
            "is_routable",
            [](RoutingEngine& engine, std::tuple<double, double> point) {
                return engine.IsRoutable(intoPoint(point));
            })
        .def("mesh", [](const RoutingEngine& routingEngine) {
            const auto mesh = routingEngine.MeshData();
            const auto polygonCount = mesh->CountPolygons();
            using Ind = decltype(mesh->Polygons(0).vertices);
            std::vector<Ind> polys;
            polys.reserve(polygonCount);
            for(size_t index = 0; index < polygonCount; ++index) {
                const auto& poly = mesh->Polygons(index);
                const auto& vertices = poly.vertices;
                polys.emplace_back(mesh->Polygons(index).vertices);
            }
            return std::make_tuple(intoTuples(mesh->FVertices()), polys);
        });
}
