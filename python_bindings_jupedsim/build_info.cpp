// SPDX-License-Identifier: LGPL-3.0-or-later
#include <jupedsim/jupedsim.h>

#include <pybind11/pybind11.h>

namespace py = pybind11;

void init_build_info(py::module_& m)
{
    py::class_<JPS_BuildInfo>(m, "BuildInfo")
        .def_readonly("git_commit_hash", &JPS_BuildInfo::git_commit_hash)
        .def_readonly("git_commit_date", &JPS_BuildInfo::git_commit_date)
        .def_readonly("git_branch", &JPS_BuildInfo::git_branch)
        .def_readonly("compiler", &JPS_BuildInfo::compiler)
        .def_readonly("compiler_version", &JPS_BuildInfo::compiler_version)
        .def_readonly("library_version", &JPS_BuildInfo::library_version);
    m.def("get_build_info", []() { return JPS_GetBuildInfo(); });
}
