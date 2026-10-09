// SPDX-License-Identifier: LGPL-3.0-or-later
#include "geometry/walkable_surface.hpp"

#include "geometry/geometry.hpp" // IWYU pragma: keep
#include "type_casters.hpp"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h> // IWYU pragma: keep
namespace py = pybind11;

void init_walkable_surface(py::module_& m)
{
    py::classh<WalkableSurface>(m, "WalkableSurface")
        .def(py::init())
        .def(
            "add_region",
            [](WalkableSurface& ws, WalkableSurface::Polygon polygon, double height) {
                return ws.add_region(std::move(polygon), height);
            },
            py::kw_only(),
            py::arg("polygon"),
            py::arg("height") = 0.0)
        .def(
            "add_region",
            [](WalkableSurface& ws,
               std::vector<Point> exterior,
               std::vector<std::vector<Point>> interior,
               double height) {
                return ws.add_region({std::move(exterior), std::move(interior)}, height);
            },
            py::kw_only(),
            py::arg("exterior"),
            py::arg("interior") = std::vector<Point>{},
            py::arg("height") = 0.0)
        .def(
            "add_region",
            [](WalkableSurface& ws, const PolyWithHoles& polygon, double height) {
                return ws.add_region(polygon, height);
            },
            py::kw_only(),
            py::arg("polygon"),
            py::arg("height") = 0.0)
        .def(
            "connect_regions",
            [](WalkableSurface& ws,
               size_t from_region,
               LineSegment from_edge,
               size_t to_region,
               LineSegment to_edge) {
                return ws.connect_regions(from_region, from_edge, to_region, to_edge);
            },
            py::kw_only(),
            py::arg("from_region"),
            py::arg("from_edge"),
            py::arg("to_region"),
            py::arg("to_edge"))
        .def("create_geometry", &WalkableSurface::create_geometry);
}
