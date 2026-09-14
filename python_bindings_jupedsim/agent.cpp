// SPDX-License-Identifier: LGPL-3.0-or-later
#include "GenericAgent.hpp"
#include "conversion.hpp"

#include <pybind11/cast.h>
#include <pybind11/detail/common.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h> // IWYU pragma: keep

#include <tuple>

namespace py = pybind11;

void init_agent(py::module_& m)
{
    // TRANSIENT ONLY: this class wraps a reference to an agent owned by the
    // simulation. Instances are only valid for the duration of a single
    // property access on a Python agent handle or a single custom-model
    // callback. They must never be stored across Simulation.iterate(). This is
    // not the Python-visible Agent; the public Agent is a handle that resolves
    // this wrapper freshly on every attribute access.
    py::class_<GenericAgent>(m, "Agent")
        .def_property_readonly("id", [](const GenericAgent& agent) { return agent.id.getID(); })
        .def_property_readonly(
            "journey_id", [](const GenericAgent& agent) { return agent.journeyId.getID(); })
        .def_property_readonly(
            "stage_id", [](const GenericAgent& agent) { return agent.stageId.getID(); })
        .def_property_readonly(
            "position", [](const GenericAgent& agent) { return intoTuple(agent.location.xy()); })
        .def_property_readonly(
            "location",
            // The token points into geometry the simulation owns; this agent wrapper in turn
            // keeps the simulation alive. keep_alive needs the cpp_function spelling here.
            py::cpp_function(
                [](const GenericAgent& agent) { return agent.location; }, py::keep_alive<0, 1>()))
        .def_property_readonly(
            "final_target",
            [](const GenericAgent& agent) { return intoTuple(agent.finalTarget.xy()); })
        .def_property_readonly(
            "next_target", [](const GenericAgent& agent) { return intoTuple(agent.nextTarget); })
        .def_property_readonly(
            "state",
            [](GenericAgent& agent) -> OperationalModelState& { return agent.state; },
            py::return_value_policy::reference);
}
