// SPDX-License-Identifier: LGPL-3.0-or-later
#include "conversion.hpp"
#include <CollisionFreeSpeedModel.hpp>
#include <CollisionFreeSpeedModelBuilder.hpp>
#include <CollisionFreeSpeedModelData.hpp>

#include <fmt/format.h>
#include <fmt/ranges.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

void init_collision_free_speed_model(py::module_& m)
{
    py::class_<CollisionFreeSpeedModel, OperationalModel>(m, "CollisionFreeSpeedModel");
    py::class_<CollisionFreeSpeedModelBuilder>(m, "CollisionFreeSpeedModelBuilder")
        .def(
            py::init<double, double, double, double>(),
            py::kw_only(),
            py::arg("strength_neighbor_repulsion"),
            py::arg("range_neighbor_repulsion"),
            py::arg("strength_geometry_repulsion"),
            py::arg("range_geometry_repulsion"))
        .def("build", &CollisionFreeSpeedModelBuilder::Build);
    py::class_<CollisionFreeSpeedModelData>(m, "CollisionFreeSpeedModelState")
        .def(
            py::init([](double timeGap, double desiredSpeed, double radius) {
                return CollisionFreeSpeedModelData{
                    .timeGap = timeGap, .v0 = desiredSpeed, .radius = radius};
            }),
            py::kw_only(),
            py::arg("time_gap"),
            py::arg("desired_speed"),
            py::arg("radius"))
        .def_readwrite("time_gap", &CollisionFreeSpeedModelData::timeGap)
        .def_readwrite("desired_speed", &CollisionFreeSpeedModelData::v0)
        .def_readwrite("radius", &CollisionFreeSpeedModelData::radius);
}
