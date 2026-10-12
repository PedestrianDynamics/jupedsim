// SPDX-License-Identifier: LGPL-3.0-or-later
#include "stage.hpp"

#include "conversion.hpp"
#include "type_casters.hpp" // IWYU pragma: keep

#include <pybind11/pybind11.h>
#include <pybind11/stl.h> // IWYU pragma: keep

namespace py = pybind11;

// Expose count_targeting() to each stage without exposing BaseStage.
template <typename T>
static py::class_<T> bind_stage(py::module_& m, const char* name, const char* doc)
{
    py::class_<T> stage(m, name);
    stage.doc() = clean_doc(doc);
    stage.def(
        "count_targeting",
        &T::count_targeting,
        clean_doc(R"(
        Count the agents currently targeting this stage.

        Returns:
            Number of agents whose current target is this stage.
    )")
            .c_str());
    return stage;
}

void init_stage(py::module_& m)
{
    auto waypoint_stage = bind_stage<Waypoint>(m, "WaypointStage", R"(
        Models a waypoint.

        A waypoint is considered to be reached if an agent is within the specified
        distance to the waypoint.
    )");

    auto exit_stage = bind_stage<Exit>(m, "ExitStage", R"(
        Models an exit.

        Agents entering the polygon defining the exit will be removed at the
        beginning of the next iteration, i.e. agents will be inside the specified
        polygon for one frame.
    )");

    auto steering_stage = bind_stage<DirectSteering>(m, "DirectSteeringStage", R"(
        Models a direct steering stage.

        This stage allows a direct control of the target the agent is walking to,
        see :attr:`~jupedsim.Agent.final_target`. It bypasses the tactical and
        strategical level of the simulation; the operational level stays active.
        A direct steering stage can only be used if it is the only stage in a
        journey.
    )");
}
