// SPDX-License-Identifier: LGPL-3.0-or-later
#include "stage.hpp"

#include "conversion.hpp"
#include "type_casters.hpp" // IWYU pragma: keep

#include <pybind11/pybind11.h>
#include <pybind11/stl.h> // IWYU pragma: keep

namespace py = pybind11;

void init_stage(py::module_& m)
{
    constexpr const char* count_targeting_doc = R"(
        Count the agents currently targeting this stage.

        Returns:
            Number of agents whose current target is this stage.
    )";

    py::class_<WaypointProxy> waypoint_stage(m, "WaypointStage");
    waypoint_stage.doc() = clean_doc(R"(
        Models a waypoint.

        A waypoint is considered to be reached if an agent is within the specified
        distance to the waypoint.
    )");
    waypoint_stage.def(
        "count_targeting", &WaypointProxy::count_targeting, clean_doc(count_targeting_doc).c_str());

    py::class_<ExitProxy> exit_stage(m, "ExitStage");
    exit_stage.doc() = clean_doc(R"(
        Models an exit.

        Agents entering the polygon defining the exit will be removed at the
        beginning of the next iteration, i.e. agents will be inside the specified
        polygon for one frame.
    )");
    exit_stage.def(
        "count_targeting", &ExitProxy::count_targeting, clean_doc(count_targeting_doc).c_str());

    py::class_<DirectSteeringProxy> steering_stage(m, "DirectSteeringStage");
    steering_stage.doc() = clean_doc(R"(
        Models a direct steering stage.

        This stage allows a direct control of the target the agent is walking to,
        see :attr:`~jupedsim.Agent.final_target`. It bypasses the tactical and
        strategical level of the simulation; the operational level stays active.
        A direct steering stage can only be used if it is the only stage in a
        journey.
    )");
    steering_stage.def(
        "count_targeting",
        &DirectSteeringProxy::count_targeting,
        clean_doc(count_targeting_doc).c_str());
}
