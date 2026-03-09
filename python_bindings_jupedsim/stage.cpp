// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Stage.hpp"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h> // IWYU pragma: keep

namespace py = pybind11;

void init_stage(py::module_& m)
{
    py::class_<NotifiableQueueProxy>(m, "NotifiableQueueProxy")
        .def("count_targeting", &NotifiableQueueProxy::CountTargeting)
        .def("count_enqueued", &NotifiableQueueProxy::CountEnqueued)
        .def("pop", &NotifiableQueueProxy::Pop)
        .def("enqueued", &NotifiableQueueProxy::Enqueued);

    py::enum_<WaitingSetState>(m, "WaitingSetState")
        .value("Active", WaitingSetState::Active)
        .value("Inactive", WaitingSetState::Inactive);

    py::class_<NotifiableWaitingSetProxy>(m, "WaitingSetProxy")
        .def("count_targeting", &NotifiableWaitingSetProxy::CountTargeting)
        .def("count_waiting", &NotifiableWaitingSetProxy::CountWaiting)
        .def("waiting", &NotifiableWaitingSetProxy::Waiting)
        .def_property(
            "state",
            [](const NotifiableWaitingSetProxy& p) { return p.State(); },
            [](NotifiableWaitingSetProxy& p, WaitingSetState state) { p.State(state); });
    py::class_<WaypointProxy>(m, "WaypointProxy")
        .def("count_targeting", &WaypointProxy::CountTargeting);
    py::class_<ExitProxy>(m, "ExitProxy").def("count_targeting", &ExitProxy::CountTargeting);
    py::class_<DirectSteeringProxy>(m, "DirectSteeringProxy");
}
