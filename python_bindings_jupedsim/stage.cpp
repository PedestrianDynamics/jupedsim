// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "conversion.hpp"
#include "wrapper.hpp"

#include <jupedsim/jupedsim.h>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

void init_stage(py::module_& m)
{
    py::class_<JPS_NotifiableQueueProxy_Wrapper>(m, "NotifiableQueueProxy")
        .def(
            "count_targeting",
            [](const JPS_NotifiableQueueProxy_Wrapper& w) {
                return JPS_NotifiableQueueProxy_GetCountTargeting(w.handle);
            })
        .def(
            "count_enqueued",
            [](const JPS_NotifiableQueueProxy_Wrapper& w) {
                return JPS_NotifiableQueueProxy_GetCountEnqueued(w.handle);
            })
        .def(
            "pop",
            [](JPS_NotifiableQueueProxy_Wrapper& w, size_t count) {
                JPS_NotifiableQueueProxy_Pop(w.handle, count);
            })
        .def("enqueued", [](const JPS_NotifiableQueueProxy_Wrapper& w) {
            const JPS_AgentId* ids{};
            const auto count = JPS_NotifiableQueueProxy_GetEnqueued(w.handle, &ids);
            return std::vector<JPS_AgentId>{ids, ids + count};
        });
    py::enum_<JPS_WaitingSetState>(m, "WaitingSetState")
        .value("Active", JPS_WaitingSet_Active)
        .value("Inactive", JPS_WaitingSet_Inactive);
    py::class_<JPS_WaitingSetProxy_Wrapper>(m, "WaitingSetProxy")
        .def(
            "count_targeting",
            [](const JPS_WaitingSetProxy_Wrapper& w) {
                return JPS_WaitingSetProxy_GetCountTargeting(w.handle);
            })
        .def(
            "count_waiting",
            [](const JPS_WaitingSetProxy_Wrapper& w) {
                return JPS_WaitingSetProxy_GetCountWaiting(w.handle);
            })
        .def(
            "waiting",
            [](const JPS_WaitingSetProxy_Wrapper& w) {
                const JPS_AgentId* ids{};
                const auto count = JPS_WaitingSetProxy_GetWaiting(w.handle, &ids);
                return std::vector<JPS_AgentId>{ids, ids + count};
            })
        .def_property(
            "state",
            [](const JPS_WaitingSetProxy_Wrapper& w) {
                return JPS_WaitingSetProxy_GetWaitingSetState(w.handle);
            },
            [](JPS_WaitingSetProxy_Wrapper& w, JPS_WaitingSetState state) {
                JPS_WaitingSetProxy_SetWaitingSetState(w.handle, state);
            });
    py::class_<JPS_WaypointProxy_Wrapper>(m, "WaypointProxy")
        .def("count_targeting", [](const JPS_WaypointProxy_Wrapper& w) {
            return JPS_WaypointProxy_GetCountTargeting(w.handle);
        });
    py::class_<JPS_ExitProxy_Wrapper>(m, "ExitProxy")
        .def("count_targeting", [](const JPS_ExitProxy_Wrapper& w) {
            return JPS_ExitProxy_GetCountTargeting(w.handle);
        });
    py::class_<JPS_DirectSteeringProxy_Wrapper>(m, "DirectSteeringProxy");
}
