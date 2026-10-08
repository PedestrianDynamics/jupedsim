// SPDX-License-Identifier: LGPL-3.0-or-later
#include "BuildInfo.hpp"
#include "conversion.hpp"

#include <fmt/format.h>
#include <pybind11/pybind11.h>

namespace py = pybind11;

namespace
{
/// Python's jupedsim.BuildInfo: reads the constants from BuildInfo.hpp.
struct PyBuildInfo {
};
} // namespace

void init_build_info(py::module_& m)
{
    py::class_<PyBuildInfo> buildInfo(m, "BuildInfo");
    buildInfo.doc() = cleanDoc(R"(
        Build information about this jupedsim module.

        Printable, see :func:`get_build_info`.
    )");
    buildInfo.def(py::init<>())
        .def_property_readonly(
            "git_commit_hash",
            [](const PyBuildInfo&) { return GIT_COMMIT_HASH; },
            cleanDoc("SHA1 commit hash this module was built from.").c_str())
        .def_property_readonly(
            "git_commit_date",
            [](const PyBuildInfo&) { return GIT_COMMIT_DATE; },
            cleanDoc("Date of the commit this module was built from.").c_str())
        .def_property_readonly(
            "git_branch",
            [](const PyBuildInfo&) { return GIT_BRANCH; },
            cleanDoc("Git branch this module was built from.").c_str())
        .def_property_readonly(
            "compiler",
            [](const PyBuildInfo&) { return COMPILER; },
            cleanDoc("Compiler the native code was compiled with.").c_str())
        .def_property_readonly(
            "compiler_version",
            [](const PyBuildInfo&) { return COMPILER_VERSION; },
            cleanDoc("Version of the compiler the native code was compiled with.").c_str())
        .def_property_readonly(
            "library_version",
            [](const PyBuildInfo&) { return LIBRARY_VERSION; },
            cleanDoc("Version of the jupedsim library, e.g. ``2.0.0``.").c_str())
        .def("__repr__", [](const PyBuildInfo&) {
            return fmt::format(
                "JuPedSim {}:\n--------------------------------\nCommit: {} from {} on {}\n"
                "Compiler: {} ({})",
                LIBRARY_VERSION,
                GIT_COMMIT_HASH,
                GIT_BRANCH,
                GIT_COMMIT_DATE,
                COMPILER,
                COMPILER_VERSION);
        });
    m.def(
        "get_build_info",
        [] { return PyBuildInfo{}; },
        cleanDoc(R"(
        Get build information about jupedsim.

        The received :class:`BuildInfo` is printable, e.g.

        .. code:: python

            print(get_build_info())

        This will display a human-readable string stating
        basic information about this library.

        Returns:
            Build information of this module.
        )")
            .c_str());
}
