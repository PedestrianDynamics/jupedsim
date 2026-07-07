// SPDX-License-Identifier: LGPL-3.0-or-later
#include "CollisionFreeSpeedModelV3.hpp"
#include "OperationalModel.hpp"
#include "type_casters.hpp" // IWYU pragma: keep

#include <pybind11/cast.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h> // IWYU pragma: keep

namespace py = pybind11;

void init_collision_free_speed_model_v3(py::module_& m)
{
    py::class_<CollisionFreeSpeedModelV3, OperationalModel, py::smart_holder>(
        m, "CollisionFreeSpeedModelV3")
        .def(py::init<>());
    const CollisionFreeSpeedModelV3::State d{};
    py::class_<CollisionFreeSpeedModelV3::State>(m, "CollisionFreeSpeedModelV3State")
        .def(
            py::init([](Point position,
                        Point orientation,
                        double strengthNeighborRepulsion,
                        double rangeNeighborRepulsion,
                        double strengthGeometryRepulsion,
                        double rangeGeometryRepulsion,
                        double rangeXScale,
                        double rangeYScale,
                        double thetaMaxUpperBound,
                        double agentBuffer,
                        double timeGap,
                        double desiredSpeed,
                        double radius,
                        double headingAngle) {
                return CollisionFreeSpeedModelV3::State{
                    .position = position,
                    .orientation = orientation,
                    .strengthNeighborRepulsion = strengthNeighborRepulsion,
                    .rangeNeighborRepulsion = rangeNeighborRepulsion,
                    .strengthGeometryRepulsion = strengthGeometryRepulsion,
                    .rangeGeometryRepulsion = rangeGeometryRepulsion,
                    .rangeXScale = rangeXScale,
                    .rangeYScale = rangeYScale,
                    .thetaMaxUpperBound = thetaMaxUpperBound,
                    .agentBuffer = agentBuffer,
                    .timeGap = timeGap,
                    .v0 = desiredSpeed,
                    .radius = radius,
                    .headingAngle = headingAngle};
            }),
            py::kw_only(),
            py::arg("position") = d.position,
            py::arg("orientation") = d.orientation,
            py::arg("strength_neighbor_repulsion") = d.strengthNeighborRepulsion,
            py::arg("range_neighbor_repulsion") = d.rangeNeighborRepulsion,
            py::arg("strength_geometry_repulsion") = d.strengthGeometryRepulsion,
            py::arg("range_geometry_repulsion") = d.rangeGeometryRepulsion,
            py::arg("range_x_scale") = d.rangeXScale,
            py::arg("range_y_scale") = d.rangeYScale,
            py::arg("theta_max_upper_bound") = d.thetaMaxUpperBound,
            py::arg("agent_buffer") = d.agentBuffer,
            py::arg("time_gap") = d.timeGap,
            py::arg("desired_speed") = d.v0,
            py::arg("radius") = d.radius,
            py::arg("heading_angle") = d.headingAngle)
        .def_readwrite("position", &CollisionFreeSpeedModelV3::State::position)
        .def_readwrite("orientation", &CollisionFreeSpeedModelV3::State::orientation)
        .def_readwrite(
            "strength_neighbor_repulsion",
            &CollisionFreeSpeedModelV3::State::strengthNeighborRepulsion)
        .def_readwrite(
            "range_neighbor_repulsion", &CollisionFreeSpeedModelV3::State::rangeNeighborRepulsion)
        .def_readwrite(
            "strength_geometry_repulsion",
            &CollisionFreeSpeedModelV3::State::strengthGeometryRepulsion)
        .def_readwrite(
            "range_geometry_repulsion", &CollisionFreeSpeedModelV3::State::rangeGeometryRepulsion)
        .def_readwrite("range_x_scale", &CollisionFreeSpeedModelV3::State::rangeXScale)
        .def_readwrite("range_y_scale", &CollisionFreeSpeedModelV3::State::rangeYScale)
        .def_readwrite(
            "theta_max_upper_bound", &CollisionFreeSpeedModelV3::State::thetaMaxUpperBound)
        .def_readwrite("agent_buffer", &CollisionFreeSpeedModelV3::State::agentBuffer)
        .def_readwrite("time_gap", &CollisionFreeSpeedModelV3::State::timeGap)
        .def_readwrite("desired_speed", &CollisionFreeSpeedModelV3::State::v0)
        .def_readwrite("radius", &CollisionFreeSpeedModelV3::State::radius)
        .def_readwrite("heading_angle", &CollisionFreeSpeedModelV3::State::headingAngle);
}
