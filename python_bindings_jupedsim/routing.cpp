// SPDX-License-Identifier: LGPL-3.0-or-later
#include "conversion.hpp"
#include "wrapper.hpp"

#include <cstddef>
#include <jupedsim/jupedsim.h>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <algorithm>
#include <memory>
#include <tuple>
#include <vector>

namespace py = pybind11;

void init_routing(py::module_& m)
{
    py::class_<JPS_RoutingEngine_Wrapper>(m, "RoutingEngine")
        .def(py::init([](const JPS_Geometry_Wrapper& geo) {
            return std::make_unique<JPS_RoutingEngine_Wrapper>(
                JPS_RoutingEngine_Create(geo.handle));
        }))
        .def(
            "compute_waypoints",
            [](const JPS_RoutingEngine_Wrapper& w,
               std::tuple<double, double> from,
               std::tuple<double, double> to) {
                auto intoJPS_Point = [](const auto p) {
                    return JPS_Point{std::get<0>(p), std::get<1>(p)};
                };
                auto waypoints = JPS_RoutingEngine_ComputeWaypoint(
                    w.handle, intoJPS_Point(from), intoJPS_Point(to));
                std::vector<std::tuple<double, double>> result;
                result.reserve(waypoints.len);
                std::transform(
                    waypoints.points,
                    waypoints.points + waypoints.len,
                    std::back_inserter(result),
                    [](const auto& p) { return std::make_tuple(p.x, p.y); });
                JPS_Path_Free(waypoints);
                return result;
            })
        .def(
            "is_routable",
            [](const JPS_RoutingEngine_Wrapper& w, std::tuple<double, double> p) {
                return JPS_RoutingEngine_IsRoutable(w.handle, intoJPS_Point(p));
            })
        .def("mesh", [](const JPS_RoutingEngine_Wrapper& w) {
            auto mesh = JPS_RoutingEngine_Mesh(w.handle);
            using Pt = std::tuple<double, double>;
            using Vert = std::vector<Pt>;
            using Ind = std::vector<uint16_t>;
            using Polys = std::vector<Ind>;
            auto result = std::tuple<Vert, Polys>{};
            auto& [vertices, polygons] = result;
            vertices.reserve(mesh.vertices_len);
            std::transform(
                mesh.vertices,
                mesh.vertices + mesh.vertices_len,
                std::back_inserter(vertices),
                [](const auto& v) { return std::make_tuple(v.x, v.y); });
            polygons.reserve(mesh.polygons_len);
            for(size_t polygon_index = 0; polygon_index < mesh.polygons_len; ++polygon_index) {
                polygons.emplace_back();
                const auto desc = mesh.polygons[polygon_index];
                polygons.back().reserve(desc.len);
                polygons.back().insert(
                    std::end(polygons.back()),
                    mesh.indices + desc.offset,
                    mesh.indices + desc.offset + desc.len);
            }

            JPS_Mesh_Free(mesh);
            return result;
        });
}
