// SPDX-License-Identifier: LGPL-3.0-or-later
#include "CollisionFreeSpeedModelV2.hpp"
#include "OperationalModel.hpp"
#include "type_casters.hpp" // IWYU pragma: keep

#include <pybind11/cast.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h> // IWYU pragma: keep

namespace py = pybind11;

void init_collision_free_speed_model_v2(py::module_& m)
{
    py::class_<CollisionFreeSpeedModelV2, OperationalModel, py::smart_holder>(
        m, "CollisionFreeSpeedModelV2")
        .def(py::init<>());
    const CollisionFreeSpeedModelV2::State d{};
    py::class_<CollisionFreeSpeedModelV2::State>(m, "CollisionFreeSpeedModelV2State")
        .def(
            py::init([](Point position,
                        Point orientation,
                        double strengthNeighborRepulsion,
                        double rangeNeighborRepulsion,
                        double strengthGeometryRepulsion,
                        double rangeGeometryRepulsion,
                        double timeGap,
                        double desiredSpeed,
                        double radius) {
                return CollisionFreeSpeedModelV2::State{
                    .position = position,
                    .orientation = orientation,
                    .strengthNeighborRepulsion = strengthNeighborRepulsion,
                    .rangeNeighborRepulsion = rangeNeighborRepulsion,
                    .strengthGeometryRepulsion = strengthGeometryRepulsion,
                    .rangeGeometryRepulsion = rangeGeometryRepulsion,
                    .timeGap = timeGap,
                    .v0 = desiredSpeed,
                    .radius = radius};
            }),
            py::kw_only(),
            py::arg("position") = d.position,
            py::arg("orientation") = d.orientation,
            py::arg("strength_neighbor_repulsion") = d.strengthNeighborRepulsion,
            py::arg("range_neighbor_repulsion") = d.rangeNeighborRepulsion,
            py::arg("strength_geometry_repulsion") = d.strengthGeometryRepulsion,
            py::arg("range_geometry_repulsion") = d.rangeGeometryRepulsion,
            py::arg("time_gap") = d.timeGap,
            py::arg("desired_speed") = d.v0,
            py::arg("radius") = d.radius)
        .def_readwrite("position", &CollisionFreeSpeedModelV2::State::position)
        .def_readwrite("orientation", &CollisionFreeSpeedModelV2::State::orientation)
        .def_readwrite(
            "strength_neighbor_repulsion",
            &CollisionFreeSpeedModelV2::State::strengthNeighborRepulsion)
        .def_readwrite(
            "range_neighbor_repulsion", &CollisionFreeSpeedModelV2::State::rangeNeighborRepulsion)
        .def_readwrite(
            "strength_geometry_repulsion",
            &CollisionFreeSpeedModelV2::State::strengthGeometryRepulsion)
        .def_readwrite(
            "range_geometry_repulsion", &CollisionFreeSpeedModelV2::State::rangeGeometryRepulsion)
        .def_readwrite("time_gap", &CollisionFreeSpeedModelV2::State::timeGap)
        .def_readwrite("desired_speed", &CollisionFreeSpeedModelV2::State::v0)
        .def_readwrite("radius", &CollisionFreeSpeedModelV2::State::radius);
}
