// SPDX-License-Identifier: LGPL-3.0-or-later
#include "conversion.hpp"

#include <CollisionGeometry.hpp>
#include <RoutingEngine.hpp>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <algorithm>
#include <cstddef>
#include <memory>
#include <tuple>
#include <vector>

namespace py = pybind11;

void init_routing(py::module_& m)
{
    py::class_<RoutingEngine>(m, "RoutingEngine")
        .def(py::init([](const CollisionGeometry& geo) {
            return std::make_unique<RoutingEngine>(geo.Polygon());
        }))
        .def("compute_waypoints", &RoutingEngine::ComputeAllWaypoints)
        .def("is_routable", &RoutingEngine::IsRoutable)
        .def("mesh", [](const RoutingEngine& routingEngine) {
            using Pt = glm::vec2;
            using Vert = std::vector<Pt>;
            using Ind = std::vector<uint16_t>;
            using Polys = std::vector<Ind>;
            const auto mesh = routingEngine.MeshData();
            const auto polygonCount = mesh->CountPolygons();
            Polys polys{polygonCount};
            for(size_t index = 0; index < polygonCount; ++index) {
                const auto& poly = mesh->Polygons(index);
                const auto& vertices = poly.vertices;
                polys[index].reserve(vertices.size());
                for(size_t vertIndex = 0; vertIndex < vertices.size(); ++vertIndex) {
                    polys[index][vertIndex] = vertices[vertIndex];
                }
            }
            return std::make_tuple(mesh->FVertices(), polys);
        });
}
