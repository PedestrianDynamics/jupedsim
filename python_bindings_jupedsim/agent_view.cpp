// SPDX-License-Identifier: LGPL-3.0-or-later
#include "AgentView.hpp"
#include "callback_views.hpp"
#include "conversion.hpp"
#include "type_casters.hpp" // IWYU pragma: keep

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/typing.h>

#include <memory>
#include <optional>
#include <utility>
#include <variant>

namespace py = pybind11;

void init_agent_view(py::module_& m)
{
    py::class_<PyNeighborView> neighborView(m, "NeighborView");
    neighborView.doc() = cleanDoc(R"(
        A neighboring agent as seen from the agent that asked for it.

        Obtained from :meth:`~jupedsim.AgentView.other_agents_in_range`. Only valid for the
        duration of the callback it was created in; never store it: reading
        :attr:`state` after the callback has returned raises
        :class:`~jupedsim.SimulationError`.
    )");
    neighborView
        .def_property_readonly(
            "relative_position",
            &PyNeighborView::RelativePosition,
            cleanDoc("Vector from the querying agent to this neighbor.").c_str())
        .def_property_readonly(
            "state",
            [](const PyNeighborView& self) { return stateToPython(*self.Neighbor().state); },
            cleanDoc(R"(
            Model state of this neighbor.

            Treat it as read-only: state may only be changed by returning a new
            state object from ``compute_next_state``.
            )")
                .c_str())
        .def("__repr__", [](const PyNeighborView& self) {
            return py::str("NeighborView(relative_position={!r})")
                .format(py::cast(self.RelativePosition()));
        });

    py::class_<WallView> wallView(m, "WallView");
    wallView.doc() = cleanDoc(R"(
        A wall segment as seen from the agent that asked for it.

        Obtained from :meth:`~jupedsim.AgentView.walls_in_range`. The view is always
        relative to the agent - as if the agent sits at the origin.
    )");
    wallView
        .def_readonly(
            "segment",
            &WallView::segment,
            cleanDoc("The part of the segment within the queried distance, relative to the agent.")
                .c_str())
        .def_readonly(
            "closest_point",
            &WallView::closest_point,
            cleanDoc("The point on the segment closest to the agent.").c_str())
        .def_readonly("distance", &WallView::distance, cleanDoc("Distance to that point.").c_str())
        .def_readonly(
            "normal",
            &WallView::normal,
            cleanDoc(R"(
            Unit vector pointing from the wall towards the agent.

            This is the direction a repulsion acts in. It is ``(0.0, 0.0)`` for an
            agent standing exactly on the wall, where no direction exists.
            )")
                .c_str())
        .def("__repr__", [](const WallView& self) {
            return py::str("WallView(distance={!r}, normal={!r})")
                .format(self.distance, py::cast(self.normal));
        });

    py::class_<PyAgentView> agentView(m, "AgentView");
    agentView.doc() = cleanDoc(R"(
        What an agent perceives of its surroundings, relative to where it stands.

        Passed to
        :meth:`~jupedsim.CustomOperationalModel.check_model_constraint`
        and, as :class:`~jupedsim.AgentStep`, to
        :meth:`~jupedsim.CustomOperationalModel.compute_next_state`.
        Do not store instances beyond the callback they were created in:
        accessing a view after its callback has returned raises
        :class:`~jupedsim.SimulationError`.

        Example:
            Visibility-filtered neighborhood::

                def compute_next_state(self, state, step):
                    boundaries = step.walls_in_range(1.0)
                    neighbors = step.other_agents_in_range(
                        5.0,
                        lambda n: step.no_geometry_between(n),
                    )
    )");
    agentView
        .def(
            "other_agents_in_range",
            [](const PyAgentView& self,
               double radius,
               const std::optional<py::typing::Callable<py::object(const PyNeighborView&)>>&
                   predicate) {
                py::list result{};
                for(const NeighborView& neighbor : self.View().OtherAgentsInRange(radius)) {
                    py::object candidate =
                        py::cast(PyNeighborView{neighbor, self.Scope(), self.Mapper()});
                    if(!predicate || py::bool_((*predicate)(candidate))) {
                        result.append(candidate);
                    }
                }
                return py::typing::List<PyNeighborView>{std::move(result)};
            },
            py::arg("radius"),
            py::arg("predicate") = py::none(),
            cleanDoc(R"(
            Return agents within *radius*, excluding the agent itself.

            Args:
                radius: Search radius.
                predicate: Optional callable ``(neighbor) -> bool``. Only neighbors
                    for which the predicate returns a true value are included. Use
                    :meth:`no_geometry_between` to filter by line-of-sight, or
                    compose multiple predicates with ``lambda n: p1(n) and p2(n)``.

            Returns:
                List of :class:`~jupedsim.NeighborView`. Only valid during the
                current callback.
            )")
                .c_str())
        .def(
            "no_geometry_between",
            [](const PyAgentView& self, const std::variant<Point, PyNeighborView>& target) {
                if(const auto* neighbor = std::get_if<PyNeighborView>(&target)) {
                    return self.View().NoGeometryBetween(neighbor->Neighbor());
                }
                return self.View().NoGeometryBetween(std::get<Point>(target));
            },
            py::arg("target"),
            cleanDoc(R"(
            Return ``True`` when nothing blocks the straight line to *target*.

            Given a :class:`~jupedsim.NeighborView`, this answers whether that
            neighbor can be seen. Given an offset, it answers whether the straight
            line to that point is free of geometry — which is also whether the agent
            can move there, as what blocks the line of sight blocks the step.

            Args:
                target: A :class:`~jupedsim.NeighborView`, or an offset from the
                    agent as ``(dx, dy)``.

            Returns:
                ``True`` if no geometry intersects the line to *target*.
            )")
                .c_str())
        .def(
            "walls_in_range",
            [](const PyAgentView& self, double distance) {
                return intoVec(self.View().WallsInRange(distance));
            },
            py::arg("distance"),
            cleanDoc(R"(
            Return the walls within *distance* of the agent.

            Args:
                distance: Maximum distance.

            Returns:
                List of :class:`~jupedsim.WallView`, each as seen from the agent.
            )")
                .c_str())
        .def(
            "with_neighbor_state_mapping",
            [](const PyAgentView& self, py::typing::Callable<py::object(py::object)> repack) {
                const AgentView& view = self.View();
                auto mapper = std::make_shared<PythonNeighborStateMapper>(std::move(repack));
                return PyAgentView{view.WithNeighborStateMapping(*mapper), self.Scope(), mapper};
            },
            py::arg("repack"),
            cleanDoc(R"(
            Return this view with every neighbor seen through *repack*.

            Use this to delegate a view to a built-in model whose
            ``check_model_constraint`` expects neighbors to carry its own state type::

                def check_model_constraint(self, state, view):
                    cfsm_view = view.with_neighbor_state_mapping(self.as_cfsm_state)
                    return self._cfsm.check_model_constraint(state.sub, cfsm_view)

            This view is not modified, so the model keeps seeing its own states.

            Args:
                repack: Callable ``(neighbor_state) -> state``, called once per
                    neighbor per query. It has to return a built-in model state.

            Returns:
                A new :class:`~jupedsim.AgentView`. Only valid during the current
                callback.
            )")
                .c_str());

    py::class_<PyAgentStep, PyAgentView> agentStep(m, "AgentStep");
    agentStep.doc() = cleanDoc(R"(
        An :class:`~jupedsim.AgentView` plus what only holds for one step.

        Passed to
        :meth:`~jupedsim.CustomOperationalModel.compute_next_state`.
        Accessing a step after its callback has returned raises
        :class:`~jupedsim.SimulationError`.
    )");
    agentStep
        .def(
            "with_neighbor_state_mapping",
            [](const PyAgentStep& self, py::typing::Callable<py::object(py::object)> repack) {
                const AgentStep& step = self.Step();
                auto mapper = std::make_shared<PythonNeighborStateMapper>(std::move(repack));
                return PyAgentStep{step.WithNeighborStateMapping(*mapper), self.Scope(), mapper};
            },
            py::arg("repack"),
            cleanDoc(R"(
            Return this step with every neighbor seen through *repack*.

            Use this to delegate a step to a built-in model whose
            ``compute_next_state`` expects neighbors to carry its own state type::

                def compute_next_state(self, state, step):
                    cfsm_step = step.with_neighbor_state_mapping(self.as_cfsm_state)
                    sub, movement = self._cfsm.compute_next_state(state.sub, cfsm_step)
                    return replace(state, sub=sub), movement

            This step is not modified, so the model keeps seeing its own states.

            Args:
                repack: Callable ``(neighbor_state) -> state``, called once per
                    neighbor per query. It has to return a built-in model state.

            Returns:
                A new :class:`~jupedsim.AgentStep`. Only valid during the current
                callback.
            )")
                .c_str())
        .def_property_readonly(
            "dt",
            [](const PyAgentStep& self) { return self.Step().dt(); },
            cleanDoc("Duration of this simulation step in seconds.").c_str())
        .def_property_readonly(
            "route_orientation",
            [](const PyAgentStep& self) { return self.Step().route_orientation(); },
            cleanDoc(R"(
            Unit vector along the route to the agent's final target.

            Zero when the agent has already reached it.
            )")
                .c_str());
}
