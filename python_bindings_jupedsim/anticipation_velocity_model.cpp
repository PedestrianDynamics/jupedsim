// SPDX-License-Identifier: LGPL-3.0-or-later
#include "conversion.hpp"
#include <AnticipationVelocityModel.hpp>
#include <AnticipationVelocityModelBuilder.hpp>
#include <AnticipationVelocityModelData.hpp>

#include <fmt/format.h>
#include <fmt/ranges.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

void init_anticipation_velocity_model(py::module_& m)
{
    py::class_<AnticipationVelocityModel, OperationalModel>(m, "AnticipationVelocityModel");
    py::class_<AnticipationVelocityModelBuilder>(m, "AnticipationVelocityModelBuilder")
        .def(
            py::init<double, double>(),
            py::kw_only(),
            py::arg("pushout_strength"),
            py::arg("rng_seed"))
        .def("build", &AnticipationVelocityModelBuilder::Build);
    py::class_<AnticipationVelocityModelData>(m, "AnticipationVelocityModelState")
        .def(
            py::init([](double strengthNeighborRepulsion,
                        double rangeNeighborRepulsion,
                        double wallBufferDistance,
                        double anticipationTime,
                        double reactionTime,
                        double timeGap,
                        double desiredSpeed,
                        double radius) {
                return AnticipationVelocityModelData{
                    .strengthNeighborRepulsion = strengthNeighborRepulsion,
                    .rangeNeighborRepulsion = rangeNeighborRepulsion,
                    .wallBufferDistance = wallBufferDistance,
                    .anticipationTime = anticipationTime,
                    .reactionTime = reactionTime,
                    .timeGap = timeGap,
                    .v0 = desiredSpeed,
                    .radius = radius};
            }),
            py::kw_only(),
            py::arg("strength_neighbor_repulsion"),
            py::arg("range_neighbor_repulsion"),
            py::arg("wall_buffer_distance"),
            py::arg("anticipation_time"),
            py::arg("reaction_time"),
            py::arg("time_gap"),
            py::arg("desired_speed"),
            py::arg("radius"))
        .def_readwrite(
            "strength_neighbor_repulsion",
            &AnticipationVelocityModelData::strengthNeighborRepulsion)
        .def_readwrite(
            "range_neighbor_repulsion", &AnticipationVelocityModelData::rangeNeighborRepulsion)
        .def_readwrite("wall_buffer_distance", &AnticipationVelocityModelData::wallBufferDistance)
        .def_readwrite("anticipation_time", &AnticipationVelocityModelData::anticipationTime)
        .def_readwrite("reaction_time", &AnticipationVelocityModelData::reactionTime)
        .def_readwrite("velocity", &AnticipationVelocityModelData::velocity)
        .def_readwrite("time_gap", &AnticipationVelocityModelData::timeGap)
        .def_readwrite("desired_speed", &AnticipationVelocityModelData::v0)
        .def_readwrite("radius", &AnticipationVelocityModelData::radius);
}
