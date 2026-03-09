// SPDX-License-Identifier: LGPL-3.0-or-later
#include "CollisionGeometry.hpp"
#include "GeometryBuilder.hpp"
#include "conversion.hpp"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h> // IWYU pragma: keep

#include <tuple>
#include <vector>

namespace py = pybind11;

void init_geometry(py::module_& m)
{
    py::class_<CollisionGeometry>(m, "Geometry")
        .def(
            "boundary",
            [](const CollisionGeometry& geo) {
                return intoTuples(std::get<0>(geo.AccessibleArea()));
            })
        .def("holes", [](const CollisionGeometry& geo) {
            const auto holes = std::get<1>(geo.AccessibleArea());
            std::vector<std::vector<std::tuple<double, double>>> res{};
            res.reserve(holes.size());
            for(const auto& hole : holes) {
                res.emplace_back(intoTuples(hole));
            }
        });
    py::class_<GeometryBuilder>(m, "GeometryBuilder")
        .def(py::init<>())
        .def(
            "add_accessible_area",
            [](GeometryBuilder& builder, const std::vector<std::tuple<double, double>>& points) {
                builder.AddAccessibleArea(intoPoints(points));
            })
        .def(
            "exclude_from_accessible_area",
            [](GeometryBuilder& builder, const std::vector<std::tuple<double, double>>& points) {
                builder.ExcludeFromAccessibleArea(intoPoints(points));
            })
        .def("build", &GeometryBuilder::Build);
}
