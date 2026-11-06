// SPDX-License-Identifier: LGPL-3.0-or-later
#include "geometry/geometry.hpp"

#include "cfg_cgal.hpp"
#include "conversion.hpp"
#include "polygon.hpp"
#include "type_casters.hpp" // IWYU pragma: keep

#include <pybind11/pybind11.h>
#include <pybind11/stl.h> // IWYU pragma: keep

namespace py = pybind11;

void init_geometry(py::module_& m)
{
    py::class_<epick::PolyWithHoles> polygon2d(m, "Polygon2D");
    polygon2d.doc() = clean_doc(R"(
        A polygon with holes in the (x, y) plane, e.g. the walkable area of one region.
    )");
    polygon2d.def(
        "as_wkt",
        &as_wkt,
        clean_doc("The polygon as WKT; every coordinate reads back to the exact same value.")
            .c_str());

    // smart_holder: a Simulation shares ownership of its geometry with Python.
    py::class_<Geometry, py::smart_holder>(m, "Geometry")
        .def("is_valid_location", &Geometry::is_valid_location)
        .def(
            "get_location",
            &Geometry::get_location,
            py::arg("x"),
            py::arg("y"),
            py::kw_only(),
            py::arg("region_id"),
            // The returned token points into this geometry.
            py::keep_alive<0, 1>())
        .def("region_count", &Geometry::region_count)
        .def("region_id_per_face", &Geometry::region_id_per_face)
        .def("vertices", &Geometry::vertices)
        .def("triangles", &Geometry::triangles)
        .def(
            "polygon",
            &Geometry::polygon,
            py::kw_only(),
            py::arg("region_id"),
            clean_doc("Polygon2D for the specified region ID").c_str());
}
