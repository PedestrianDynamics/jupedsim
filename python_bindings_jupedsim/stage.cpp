// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Stage.hpp"

#include "type_casters.hpp" // IWYU pragma: keep

#include <pybind11/pybind11.h>
#include <pybind11/stl.h> // IWYU pragma: keep

namespace py = pybind11;

void init_stage(py::module_& m)
{
    py::class_<WaypointProxy>(m, "WaypointProxy")
        .def("count_targeting", &WaypointProxy::CountTargeting);
    py::class_<ExitProxy>(m, "ExitProxy").def("count_targeting", &ExitProxy::CountTargeting);
    py::class_<DirectSteeringProxy>(m, "DirectSteeringProxy");
}
