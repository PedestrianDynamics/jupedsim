// SPDX-License-Identifier: LGPL-3.0-or-later

#include "BuildInfo.hpp"

#include <pybind11/pybind11.h>

namespace py = pybind11;

void init_build_info(py::module_& m)
{
    py::module_ build_info = m.def_submodule("buildinfo");
    build_info.attr("git_commit_hash") = GIT_COMMIT_HASH;
    build_info.attr("git_commit_date") = GIT_COMMIT_DATE;
    build_info.attr("git_branch") = GIT_BRANCH;
    build_info.attr("compiler") = COMPILER;
    build_info.attr("compiler_version") = COMPILER_VERSION;
    build_info.attr("library_version") = LIBRARY_VERSION;
}
