// SPDX-License-Identifier: LGPL-3.0-or-later
#include <jupedsim/jupedsim.h>

#include <fmt/format.h>
#include <pybind11/pybind11.h>

namespace py = pybind11;

void init_trace(py::module_& m)
{
    py::class_<JPS_Trace>(m, "Trace")
        .def_readonly("iteration_duration", &JPS_Trace::iteration_duration)
        .def_readonly("operational_level_duration", &JPS_Trace::operational_level_duration)
        .def("__repr__", [](const JPS_Trace& t) {
            return fmt::format(
                "Trace( Iteration: {:d}us, OperationalLevel {:d}us)",
                t.iteration_duration,
                t.operational_level_duration);
        });
}
