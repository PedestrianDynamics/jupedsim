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
            "other_agents_in_range",
            [](const EnvironmentQuery& self, const GenericAgent& agent, double radius) {
                return self.OtherAgentsInRange(agent.model, radius);
            },
            py::arg("agent"),
            py::arg("radius"),
            "All agents within radius of agent (self excluded).")
        .def(
            "no_wall_between",
            [](const EnvironmentQuery& self,
               std::tuple<double, double> from,
               std::tuple<double, double> to) -> bool {
                return self.NoGeometryBetween(intoPoint(from), intoPoint(to));
            },
            py::arg("from"),
            py::arg("to"),
            "Returns a bool: True when position from has a line-of-sight unobstructed by wall to "
            "the candidate position to.")
        .def(
            "line_segments_in_range",
            [](const EnvironmentQuery& self, std::tuple<double, double> p, double distance) {
                return intoVec(self.LineSegmentsInRange(intoPoint(p), distance));
            },
            py::arg("position"),
            py::arg("distance"),
            "Geometry segments within exact distance of position.")
        .def(
            "inside_geometry",
            [](const EnvironmentQuery& self, std::tuple<double, double> p) {
                return self.InsideGeometry(intoPoint(p));
            },
            py::arg("position"),
            "True when position lies inside the walkable area.");
}
