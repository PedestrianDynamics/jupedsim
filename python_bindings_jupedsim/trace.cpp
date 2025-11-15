// SPDX-License-Identifier: LGPL-3.0-or-later
#include "conversion.hpp"

#include <Tracing.hpp>
#include <fmt/format.h>
#include <pybind11/pybind11.h>

namespace py = pybind11;

void init_trace(py::module_& m)
{
    py::class_<PerfStats>(m, "Trace")
        .def_property_readonly(
            "iteration_duration", [](const PerfStats& ps) { return ps.IterationDuration(); })
        .def_property_readonly(
            "operational_level_duration",
            [](const PerfStats& ps) { return ps.OpDecSystemRunDuration(); })
        .def("__repr__", [](const PerfStats& ps) {
            return fmt::format(
                "Trace( Iteration: {:d}us, OperationalLevel {:d}us)",
                ps.IterationDuration(),
                ps.OpDecSystemRunDuration());
        });
}
