// SPDX-License-Identifier: LGPL-3.0-or-later
#include "BuildInfo.hpp"
#include "conversion.hpp"

#include <fmt/format.h>
#include <pybind11/pybind11.h>

namespace py = pybind11;

namespace
{
/// Python's jupedsim.BuildInfo: reads the constants from BuildInfo.hpp.
struct PyBuildInfo {};
} // namespace

void init_build_info(py::module_& m)
{
    py::class_<PyBuildInfo> build_info(m, "BuildInfo");
    build_info.doc() = clean_doc(R"(
        Build information about this jupedsim module.

        Printable, see :func:`get_build_info`.
    )");
    build_info.def(py::init<>())
        .def_property_readonly(
            "git_commit_hash",
            [](const PyBuildInfo&) { return git_commit_hash; },
            clean_doc("SHA1 commit hash this module was built from.").c_str())
        .def_property_readonly(
            "git_commit_date",
            [](const PyBuildInfo&) { return git_commit_date; },
            clean_doc("Date of the commit this module was built from.").c_str())
        .def_property_readonly(
            "git_branch",
            [](const PyBuildInfo&) { return git_branch; },
            clean_doc("Git branch this module was built from.").c_str())
        .def_property_readonly(
            "compiler",
            [](const PyBuildInfo&) { return compiler; },
            clean_doc("Compiler the native code was compiled with.").c_str())
        .def_property_readonly(
            "compiler_version",
            [](const PyBuildInfo&) { return compiler_version; },
            clean_doc("Version of the compiler the native code was compiled with.").c_str())
        .def_property_readonly(
            "library_version",
            [](const PyBuildInfo&) { return library_version; },
            clean_doc("Version of the jupedsim library, e.g. ``2.0.0``.").c_str())
        .def("__repr__", [](const PyBuildInfo&) {
            return fmt::format(
                "JuPedSim {}:\n--------------------------------\nCommit: {} from {} on {}\n"
                "Compiler: {} ({})",
                library_version,
                git_commit_hash,
                git_branch,
                git_commit_date,
                compiler,
                compiler_version);
        });
    m.def(
        "get_build_info",
        [] { return PyBuildInfo{}; },
        clean_doc(R"(
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
