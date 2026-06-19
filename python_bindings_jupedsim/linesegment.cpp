// SPDX-License-Identifier: LGPL-3.0-or-later
#include "LineSegment.hpp"

// Keep include for pybind11 typecasters to be picked up
#include "type_casters.hpp" // IWYU pragma: keep

#include <pybind11/pybind11.h>

namespace py = pybind11;

void init_linesegment(py::module_& m)
{
    py::class_<LineSegment>(m, "LineSegment")
        .def_readonly("p1", &LineSegment::p1)
        .def_readonly("p2", &LineSegment::p2)
        .def("shortest_point", &LineSegment::ShortestPoint, py::arg("p"))
        .def("dist_to", &LineSegment::DistTo, py::arg("p"));
}
