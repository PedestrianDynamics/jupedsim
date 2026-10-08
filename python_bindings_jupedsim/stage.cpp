// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Stage.hpp"

#include "conversion.hpp"
#include "type_casters.hpp" // IWYU pragma: keep

#include <pybind11/pybind11.h>
#include <pybind11/stl.h> // IWYU pragma: keep

namespace py = pybind11;

void init_stage(py::module_& m)
{
    constexpr const char* countTargetingDoc = R"(
        Count the agents currently targeting this stage.

        Returns:
            Number of agents whose current target is this stage.
    )";

    py::class_<WaypointProxy> waypointStage(m, "WaypointStage");
    waypointStage.doc() = cleanDoc(R"(
        Models a waypoint.

        A waypoint is considered to be reached if an agent is within the specified
        distance to the waypoint.
    )");
    waypointStage.def(
        "count_targeting", &WaypointProxy::CountTargeting, cleanDoc(countTargetingDoc).c_str());

    py::class_<ExitProxy> exitStage(m, "ExitStage");
    exitStage.doc() = cleanDoc(R"(
        Models an exit.

        Agents entering the polygon defining the exit will be removed at the
        beginning of the next iteration, i.e. agents will be inside the specified
        polygon for one frame.
    )");
    exitStage.def(
        "count_targeting", &ExitProxy::CountTargeting, cleanDoc(countTargetingDoc).c_str());

    py::class_<DirectSteeringProxy> steeringStage(m, "DirectSteeringStage");
    steeringStage.doc() = cleanDoc(R"(
        Models a direct steering stage.

        This stage allows a direct control of the target the agent is walking to,
        see :attr:`~jupedsim.Agent.final_target`. It bypasses the tactical and
        strategical level of the simulation; the operational level stays active.
        A direct steering stage can only be used if it is the only stage in a
        journey.
    )");
    steeringStage.def(
        "count_targeting",
        &DirectSteeringProxy::CountTargeting,
        cleanDoc(countTargetingDoc).c_str());
}
