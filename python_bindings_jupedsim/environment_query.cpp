// SPDX-License-Identifier: LGPL-3.0-or-later
#include "CollisionGeometry.hpp"
#include "EnvironmentQuery.hpp"
#include "GenericAgent.hpp"
#include "LineSegment.hpp"
#include "conversion.hpp"

#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

void init_environment_query(py::module_& m)
{
    py::class_<EnvironmentQuery>(m, "EnvironmentQuery")
        .def(
            "agents_in_range",
            [](const EnvironmentQuery& self, const GenericAgent& agent, double radius) {
                return self.AgentsInRange(agent.model, radius);
            },
            py::arg("agent"),
            py::arg("radius"),
            "All agents within radius of agent (self excluded).")
        .def(
            "agents_in_range",
            [](const EnvironmentQuery& self,
               const GenericAgent& agent,
               double radius,
               py::function predicate) -> std::vector<GenericAgent> {
                return self.AgentsInRange(
                    agent.model, radius, [&predicate](const GenericAgent& candidate) -> bool {
                        return predicate(py::cast(candidate)).cast<bool>();
                    });
            },
            py::arg("agent"),
            py::arg("radius"),
            py::arg("predicate"),
            "All agents within radius passing predicate(agent)->bool (self excluded).")
        .def(
            "visible_from",
            [](const EnvironmentQuery& self, std::tuple<double, double> pos) -> py::function {
                const CollisionGeometry* geo = &self.Geometry();
                const Point from = intoPoint(pos);
                return py::cpp_function([geo, from](const GenericAgent& candidate) -> bool {
                    return !geo->IntersectsAny(LineSegment{from, candidate.position()});
                });
            },
            py::arg("position"),
            "Returns a predicate: True when position has unobstructed line-of-sight to the "
            "candidate.")
        .def(
            "line_segments_in_range",
            [](const EnvironmentQuery& self, std::tuple<double, double> p) {
                const auto& segs = self.LineSegmentsInRange(intoPoint(p));
                return std::vector<LineSegment>(segs.begin(), segs.end());
            },
            py::arg("position"),
            "Geometry segments near position (approximate distance).")
        .def(
            "line_segments_in_range",
            [](const EnvironmentQuery& self, std::tuple<double, double> p, double distance) {
                return intoVec(self.LineSegmentsInRange(intoPoint(p), distance));
            },
            py::arg("position"),
            py::arg("distance"),
            "Geometry segments within exact distance of position.")
        .def(
            "intersects_any",
            [](const EnvironmentQuery& self, const LineSegment& ls) {
                return self.IntersectsAny(ls);
            },
            py::arg("line_segment"),
            "True when line_segment intersects any geometry boundary.")
        .def(
            "inside_geometry",
            [](const EnvironmentQuery& self, std::tuple<double, double> p) {
                return self.InsideGeometry(intoPoint(p));
            },
            py::arg("position"),
            "True when position lies inside the walkable area.");
}
