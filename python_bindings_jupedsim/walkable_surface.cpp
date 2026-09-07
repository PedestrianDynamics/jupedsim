// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Geometry/WalkableSurface.hpp"
#include "type_casters.hpp"

#include <fmt/format.h>
#include <fmt/ranges.h>
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
                fmt::println("received polygon: {}", polygon);
                return ws.AddRegion(std::move(polygon), height);
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
                fmt::println("received polygon: {} | {}", exterior, interior);
                return ws.AddRegion({std::move(exterior), std::move(interior)}, height);
            },
            py::kw_only(),
            py::arg("exterior"),
            py::arg("interior") = std::vector<Point>{},
            py::arg("height") = 0.0)
        .def(
            "connect_regions",
            [](WalkableSurface& ws,
               size_t fromRegion,
               LineSegment fromEdge,
               size_t toRegion,
               LineSegment toEdge) {
                return ws.ConnectRegions(fromRegion, fromEdge, toRegion, toEdge);
            },
            py::kw_only(),
            py::arg("from_region"),
            py::arg("from_edge"),
            py::arg("to_region"),
            py::arg("to_edge"));
}
