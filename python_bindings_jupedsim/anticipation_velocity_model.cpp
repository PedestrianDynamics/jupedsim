// SPDX-License-Identifier: LGPL-3.0-or-later
#include "anticipation_velocity_model.hpp"

#include "operational_model.hpp"
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
            py::arg("rng_seed") = 42);
    const AnticipationVelocityModel::State d{};
    py::class_<AnticipationVelocityModel::State>(m, "AnticipationVelocityModelState")
        .def(
            py::init([](Point orientation,
                        double strength_neighbor_repulsion,
                        double range_neighbor_repulsion,
                        double wall_buffer_distance,
                        double anticipation_time,
                        double reaction_time,
                        Point velocity,
                        double time_gap,
                        double desired_speed,
                        double radius) {
                return AnticipationVelocityModel::State{
                    .orientation = orientation,
                    .strength_neighbor_repulsion = strength_neighbor_repulsion,
                    .range_neighbor_repulsion = range_neighbor_repulsion,
                    .wall_buffer_distance = wall_buffer_distance,
                    .anticipation_time = anticipation_time,
                    .reaction_time = reaction_time,
                    .velocity = velocity,
                    .time_gap = time_gap,
                    .v0 = desired_speed,
                    .radius = radius};
            }),
            py::kw_only(),
            py::arg("orientation") = d.orientation,
            py::arg("strength_neighbor_repulsion") = d.strength_neighbor_repulsion,
            py::arg("range_neighbor_repulsion") = d.range_neighbor_repulsion,
            py::arg("wall_buffer_distance") = d.wall_buffer_distance,
            py::arg("anticipation_time") = d.anticipation_time,
            py::arg("reaction_time") = d.reaction_time,
            py::arg("velocity") = d.velocity,
            py::arg("time_gap") = d.time_gap,
            py::arg("desired_speed") = d.v0,
            py::arg("radius") = d.radius)
        .def_readwrite("orientation", &AnticipationVelocityModel::State::orientation)
        .def_readwrite(
            "strength_neighbor_repulsion",
            &AnticipationVelocityModel::State::strength_neighbor_repulsion)
        .def_readwrite(
            "range_neighbor_repulsion", &AnticipationVelocityModel::State::range_neighbor_repulsion)
        .def_readwrite(
            "wall_buffer_distance", &AnticipationVelocityModel::State::wall_buffer_distance)
        .def_readwrite("anticipation_time", &AnticipationVelocityModel::State::anticipation_time)
        .def_readwrite("reaction_time", &AnticipationVelocityModel::State::reaction_time)
        .def_readwrite("velocity", &AnticipationVelocityModel::State::velocity)
        .def_readwrite("time_gap", &AnticipationVelocityModel::State::time_gap)
        .def_readwrite("desired_speed", &AnticipationVelocityModel::State::v0)
        .def_readwrite("radius", &AnticipationVelocityModel::State::radius);
}
