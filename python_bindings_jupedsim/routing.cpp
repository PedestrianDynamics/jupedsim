// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "conversion.hpp"
#include "wrapper.hpp"

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
                JPS_Path_Free(&waypoints);
                return result;
            })
        .def(
            "is_routable",
            [](const JPS_RoutingEngine_Wrapper& w, std::tuple<double, double> p) {
                return JPS_RoutingEngine_IsRoutable(w.handle, intoJPS_Point(p));
            })
        .def(
            "mesh",
            [](const JPS_RoutingEngine_Wrapper& w) {
                auto result = JPS_RoutingEngine_Mesh(w.handle);
                using Pt = std::tuple<double, double>;
                using Tri = std::tuple<Pt, Pt, Pt>;
                std::vector<Tri> mesh{};
                mesh.reserve(result.len);
                std::transform(
                    result.triangles,
                    result.triangles + result.len,
                    std::back_inserter(mesh),
                    [](const auto& t) {
                        return std::make_tuple(
                            std::make_tuple(t.points[0].x, t.points[0].y),
                            std::make_tuple(t.points[1].x, t.points[1].y),
                            std::make_tuple(t.points[2].x, t.points[2].y));
                    });
                JPS_TriangleMesh_Free(&result);
                return mesh;
            })
        .def("edges_for", [](const JPS_RoutingEngine_Wrapper& w, uint32_t id) {
            auto res = JPS_RoutingEngine_EdgesFor(w.handle, id);
            using Pt = std::tuple<double, double>;
            using Line = std::tuple<Pt, Pt>;
            std::vector<Line> lines{};
            lines.reserve(res.len);
            std::transform(
                res.lines, res.lines + res.len, std::back_inserter(lines), [](const auto& l) {
                    return std::make_tuple(
                        std::make_tuple(l.points[0].x, l.points[0].y),
                        std::make_tuple(l.points[1].x, l.points[1].y));
                });
            JPS_Lines_Free(&res);
            return lines;
        });
}
