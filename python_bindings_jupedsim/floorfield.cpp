// SPDX-License-Identifier: LGPL-3.0-or-later
#include "floorfield_cxx/lib.h"

#include <pybind11/pybind11.h>
#include <string>

namespace py = pybind11;

void init_floorfield(py::module_& m)
{
    py::module_ sub = m.def_submodule("floorfield");
    sub.def(
        "add",
        [](int32_t a, int32_t b) { return floorfield::add(a, b); },
        py::arg("a"),
        py::arg("b"),
        "Add two integers in Rust.");
    sub.def(
        "greeting",
        []() {
            auto s = floorfield::greeting();
            return std::string(s.data(), s.length());
        },
        "Return the greeting string from Rust.");
}
