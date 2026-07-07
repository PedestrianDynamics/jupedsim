// SPDX-License-Identifier: LGPL-3.0-or-later
#include "AnticipationVelocityModel.hpp"
#include "OperationalModel.hpp"
#include "type_casters.hpp" // IWYU pragma: keep

#include <pybind11/cast.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h> // IWYU pragma: keep

#include <cstdint>

namespace py = pybind11;

void init_anticipation_velocity_model(py::module_& m)
{
    py::class_<AnticipationVelocityModel, OperationalModel, py::smart_holder>(
        m, "AnticipationVelocityModel")
        .def(
            py::init<double, uint64_t>(),
            py::kw_only(),
            py::arg("pushout_strength") = 0.3,
            py::arg("rng_seed"));
    const AnticipationVelocityModel::State d{};
    py::class_<AnticipationVelocityModel::State>(m, "AnticipationVelocityModelState")
        .def(
            py::init([](Point position,
                        Point orientation,
                        double strengthNeighborRepulsion,
                        double rangeNeighborRepulsion,
                        double wallBufferDistance,
                        double anticipationTime,
                        double reactionTime,
                        Point velocity,
                        double timeGap,
                        double desiredSpeed,
                        double radius) {
                return AnticipationVelocityModel::State{
                    .position = position,
                    .orientation = orientation,
                    .strengthNeighborRepulsion = strengthNeighborRepulsion,
                    .rangeNeighborRepulsion = rangeNeighborRepulsion,
                    .wallBufferDistance = wallBufferDistance,
                    .anticipationTime = anticipationTime,
                    .reactionTime = reactionTime,
                    .velocity = velocity,
                    .timeGap = timeGap,
                    .v0 = desiredSpeed,
                    .radius = radius};
            }),
            py::kw_only(),
            py::arg("position") = d.position,
            py::arg("orientation") = d.orientation,
            py::arg("strength_neighbor_repulsion") = d.strengthNeighborRepulsion,
            py::arg("range_neighbor_repulsion") = d.rangeNeighborRepulsion,
            py::arg("wall_buffer_distance") = d.wallBufferDistance,
            py::arg("anticipation_time") = d.anticipationTime,
            py::arg("reaction_time") = d.reactionTime,
            py::arg("velocity") = d.velocity,
            py::arg("time_gap") = d.timeGap,
            py::arg("desired_speed") = d.v0,
            py::arg("radius") = d.radius)
        .def_readwrite("position", &AnticipationVelocityModel::State::position)
        .def_readwrite("orientation", &AnticipationVelocityModel::State::orientation)
        .def_readwrite(
            "strength_neighbor_repulsion",
            &AnticipationVelocityModel::State::strengthNeighborRepulsion)
        .def_readwrite(
            "range_neighbor_repulsion", &AnticipationVelocityModel::State::rangeNeighborRepulsion)
        .def_readwrite(
            "wall_buffer_distance", &AnticipationVelocityModel::State::wallBufferDistance)
        .def_readwrite("anticipation_time", &AnticipationVelocityModel::State::anticipationTime)
        .def_readwrite("reaction_time", &AnticipationVelocityModel::State::reactionTime)
        .def_readwrite("velocity", &AnticipationVelocityModel::State::velocity)
        .def_readwrite("time_gap", &AnticipationVelocityModel::State::timeGap)
        .def_readwrite("desired_speed", &AnticipationVelocityModel::State::v0)
        .def_readwrite("radius", &AnticipationVelocityModel::State::radius);
}
