// SPDX-License-Identifier: LGPL-3.0-or-later

#include "Timing.hpp"
#include "Tracing.hpp"

#include <fmt/core.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

void init_trace(py::module_& m)
{
    py::class_<ProfilerSingleton>(m, "Trace")
        .def_static(
            "instance",
            []() -> ProfilerSingleton& { return ProfilerSingleton::instance(); },
            py::return_value_policy::reference)
        .def("enable", [](ProfilerSingleton& ps) { ps.enable(); })
        .def("disable", [](ProfilerSingleton& ps) { ps.disable(); })
        .def("is_enabled", [](ProfilerSingleton& ps) { return ps.isEnabled(); })
        .def(
            "push_probe",
            [](ProfilerSingleton& ps, const std::string& name) { ps.pushProbe(name); })
        .def("pop_probe", [](ProfilerSingleton& ps) { ps.popProbe(); })
        .def("dump_and_reset", [](ProfilerSingleton& ps, const std::string& filename) {
            ps.dumpAndReset(filename);
        });
}