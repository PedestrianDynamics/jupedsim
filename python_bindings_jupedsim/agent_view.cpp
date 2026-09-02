// SPDX-License-Identifier: LGPL-3.0-or-later
#include "AgentView.hpp"
#include "EnvironmentQuery.hpp"
#include "OperationalModels/CustomModel/CustomModel.hpp"
#include "conversion.hpp"
#include "python_model.hpp"
#include "type_casters.hpp" // IWYU pragma: keep

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <deque>
#include <utility>
#include <variant>

namespace py = pybind11;

namespace
{
py::object stateToPython(const OperationalModelState& state)
{
    const auto& variant = static_cast<const OperationalModelState&>(state);
    if(const auto* custom = std::get_if<CustomModel::State>(&variant)) {
        // Custom states are unwrapped so that the _CustomModelState transport type
        // never reaches user code.
        return custom->Get<GilSafePyObject>().Get();
    }
    return py::cast(variant);
}

/// Maps neighbor states through a Python callable, e.g. to hand a built-in model neighbors of
/// the state type it expects.
class PythonNeighborStateMapper : public NeighborStateMapper
{
    py::object _repack;
    /// Deque, not vector: MappedTo() hands out references that must survive later calls.
    mutable std::deque<OperationalModelState> _states{};

public:
    explicit PythonNeighborStateMapper(py::object repack) : _repack(std::move(repack)) {}

    const OperationalModelState&
    MapToCurrentState(const OperationalModelState& state) const override
    {
        py::object repacked = _repack(stateToPython(state));
        try {
            _states.emplace_back(repacked.cast<OperationalModelState>());
        } catch(const py::cast_error&) {
            // Anything that is not a built-in state is a custom state, exactly as when an
            // agent is added. This is what a Python model delegating to another Python model
            // returns.
            _states.emplace_back(CustomModel::State{GilSafePyObject{std::move(repacked)}});
        }
        return _states.back();
    }
};
} // namespace

void init_agent_view(py::module_& m)
{
    py::class_<NeighborView>(m, "NeighborView")
        .def_readonly("relative_position", &NeighborView::RelativePosition)
        .def_property_readonly(
            "state", [](const NeighborView& self) { return stateToPython(*self.state); });

    py::class_<PythonNeighborStateMapper>(m, "_NeighborStateMapper")
        .def(py::init<py::object>(), py::arg("repack"));

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
            py::overload_cast<Point>(&AgentView::NoGeometryBetween, py::const_),
            py::arg("relative_position"),
            "True when nothing blocks the straight line to relative_position.")
        .def(
            "no_geometry_between",
            py::overload_cast<const NeighborView&>(&AgentView::NoGeometryBetween, py::const_),
            py::arg("neighbor"),
            "True when the neighbor can be seen from here.")
        .def(
            "walls_in_range",
            [](const AgentView& self, double distance) {
                return intoVec(self.WallsInRange(distance));
            },
            py::arg("distance"),
            "Walls within exact distance of the agent, as seen from it.")
        .def(
            "with_neighbor_state_mapping",
            [](const AgentView& self, const PythonNeighborStateMapper& states) {
                return self.WithNeighborStateMapping(states);
            },
            py::arg("neighbor_states"),
            py::keep_alive<0, 2>(),
            "The same view, but with every neighbor seen through the given mapping.");

    py::class_<AgentStep, AgentView>(m, "AgentStep")
        .def(
            "with_neighbor_state_mapping",
            [](const AgentStep& self, const PythonNeighborStateMapper& states) {
                return self.WithNeighborStateMapping(states);
            },
            py::arg("neighbor_states"),
            py::keep_alive<0, 2>(),
            "The same step, but with every neighbor seen through the given mapping.")
        .def_property_readonly("dt", &AgentStep::dt)
        .def_property_readonly(
            "orientation_to_next_target", &AgentStep::orientation_to_next_target);
}
