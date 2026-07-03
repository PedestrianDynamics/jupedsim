// SPDX-License-Identifier: LGPL-3.0-or-later
#include "CollisionFreeSpeedModel.hpp"
#include "CollisionFreeSpeedModelBuilder.hpp"
#include "OperationalModel.hpp"
#include "conversion.hpp"
#include "type_casters.hpp" // IWYU pragma: keep

#include <pybind11/cast.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h> // IWYU pragma: keep

namespace py = pybind11;

void init_collision_free_speed_model(py::module_& m)
{
    py::class_<CollisionFreeSpeedModel, OperationalModel, py::smart_holder>(
        m, "CollisionFreeSpeedModel");
    py::class_<CollisionFreeSpeedModelBuilder>(m, "CollisionFreeSpeedModelBuilder")
        .def(
            py::init<double, double, double, double>(),
            py::kw_only(),
            py::arg("strength_neighbor_repulsion"),
            py::arg("range_neighbor_repulsion"),
            py::arg("strength_geometry_repulsion"),
            py::arg("range_geometry_repulsion"))
        .def("build", &CollisionFreeSpeedModelBuilder::Build);
    py::class_<CollisionFreeSpeedModel::State>(m, "CollisionFreeSpeedModelState")
        .def_static("_defaults", []() { return CollisionFreeSpeedModel::State{}; })
        .def(
            py::init([](std::tuple<double, double> orientation,
                        double timeGap,
                        double desiredSpeed,
                        double radius) {
                return CollisionFreeSpeedModel::State{
                    .orientation = intoPoint(orientation),
                    .timeGap = timeGap,
                    .v0 = desiredSpeed,
                    .radius = radius};
            }),
            py::kw_only(),
            py::arg("orientation"),
            py::arg("time_gap"),
            py::arg("desired_speed"),
            py::arg("radius"))
        .def_readwrite("orientation", &CollisionFreeSpeedModel::State::orientation)
        .def_readwrite("time_gap", &CollisionFreeSpeedModel::State::timeGap)
        .def_readwrite("desired_speed", &CollisionFreeSpeedModel::State::v0)
        .def_readwrite("radius", &CollisionFreeSpeedModel::State::radius);
}
