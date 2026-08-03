// SPDX-License-Identifier: LGPL-3.0-or-later
#include "AgentView.hpp"
#include "EnvironmentQuery.hpp"
#include "OperationalModels/CustomModel/CustomModel.hpp"
#include "conversion.hpp"
#include "python_model.hpp"
#include "type_casters.hpp" // IWYU pragma: keep

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <variant>

namespace py = pybind11;

void init_agent_view(py::module_& m)
{
    py::class_<NeighborView>(m, "NeighborView")
        .def_readonly("relative_position", &NeighborView::RelativePosition)
        .def_property_readonly("state", [](const NeighborView& self) -> py::object {
            const auto& state = *self.state;
            if(const auto* custom = std::get_if<CustomModel::State>(&state)) {
                // Custom states are unwrapped so that the _CustomModelState transport type
                // never reaches user code.
                return custom->Get<GilSafePyObject>().Get();
            }
            return py::cast(state);
        });

    py::class_<WallView>(m, "WallView")
        .def_readonly("segment", &WallView::segment)
        .def_readonly("closest_point", &WallView::closest_point)
        .def_readonly("distance", &WallView::distance)
        .def_readonly("normal", &WallView::normal);

    py::class_<AgentView>(m, "AgentView")
        .def(
            "other_agents_in_range",
            [](const AgentView& self, double radius) { return self.OtherAgentsInRange(radius); },
            py::arg("radius"),
            "Neighbors within radius, each as the vector pointing to it.")
        .def(
            "no_geometry_between",
            [](const AgentView& self,
               std::tuple<double, double> relative_position,
               const std::vector<WallView>& walls) {
                return self.NoGeometryBetween(intoPoint(relative_position), walls);
            },
            py::arg("relative_position"),
            py::arg("walls"),
            "True when nothing blocks the straight line to relative_position.")
        .def(
            "walls_nearby",
            [](const AgentView& self) { return intoVec(self.WallsNearby()); },
            "Geometry segments in the grid cells around the agent.")
        .def(
            "walls_in_range",
            [](const AgentView& self, double distance) {
                return intoVec(self.WallsInRange(distance));
            },
            py::arg("distance"),
            "Walls within exact distance of the agent, as seen from it.");

    py::class_<AgentStep, AgentView>(m, "AgentStep")
        .def_property_readonly("dt", &AgentStep::dt)
        .def_property_readonly(
            "orientation_to_next_target", &AgentStep::orientation_to_next_target);
}
