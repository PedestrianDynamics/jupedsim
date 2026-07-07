// SPDX-License-Identifier: LGPL-3.0-or-later
#include "CollisionFreeSpeedModel.hpp"
#include "OperationalModel.hpp"
#include "type_casters.hpp" // IWYU pragma: keep

#include <pybind11/cast.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h> // IWYU pragma: keep

namespace py = pybind11;

void init_collision_free_speed_model(py::module_& m)
{
    py::class_<CollisionFreeSpeedModel, OperationalModel, py::smart_holder>(
        m, "CollisionFreeSpeedModel")
        .def(
            py::init<double, double, double, double>(),
            py::kw_only(),
            py::arg("strength_neighbor_repulsion") = 8.0,
            py::arg("range_neighbor_repulsion") = 0.1,
            py::arg("strength_geometry_repulsion") = 5.0,
            py::arg("range_geometry_repulsion") = 0.02);
    const CollisionFreeSpeedModel::State d{};
    py::class_<CollisionFreeSpeedModel::State>(m, "CollisionFreeSpeedModelState")
        .def(
            py::init([](Point position,
                        Point orientation,
                        double timeGap,
                        double desiredSpeed,
                        double radius) {
                return CollisionFreeSpeedModel::State{
                    .position = position,
                    .orientation = orientation,
                    .timeGap = timeGap,
                    .v0 = desiredSpeed,
                    .radius = radius};
            }),
            py::kw_only(),
            py::arg("position") = d.position,
            py::arg("orientation") = d.orientation,
            py::arg("time_gap") = d.timeGap,
            py::arg("desired_speed") = d.v0,
            py::arg("radius") = d.radius)
        .def_readwrite("position", &CollisionFreeSpeedModel::State::position)
        .def_readwrite("orientation", &CollisionFreeSpeedModel::State::orientation)
        .def_readwrite("time_gap", &CollisionFreeSpeedModel::State::timeGap)
        .def_readwrite("desired_speed", &CollisionFreeSpeedModel::State::v0)
        .def_readwrite("radius", &CollisionFreeSpeedModel::State::radius);
}
