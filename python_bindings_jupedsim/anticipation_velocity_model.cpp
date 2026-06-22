// SPDX-License-Identifier: LGPL-3.0-or-later
#include "AnticipationVelocityModel.hpp"
#include "AnticipationVelocityModelBuilder.hpp"
#include "AnticipationVelocityModelData.hpp"
#include "OperationalModel.hpp"
#include "conversion.hpp"

#include <pybind11/cast.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h> // IWYU pragma: keep

namespace py = pybind11;

void init_anticipation_velocity_model(py::module_& m)
{
    py::class_<AnticipationVelocityModel, OperationalModel, py::smart_holder>(
        m, "AnticipationVelocityModel");
    py::class_<AnticipationVelocityModelBuilder>(m, "AnticipationVelocityModelBuilder")
        .def(
            py::init<double, double>(),
            py::kw_only(),
            py::arg("pushout_strength"),
            py::arg("rng_seed"))
        .def("build", &AnticipationVelocityModelBuilder::Build);
    py::class_<AnticipationVelocityModelData>(m, "AnticipationVelocityModelState")
        .def_static("_defaults", []() { return AnticipationVelocityModelData{}; })
        .def(
            py::init([](std::tuple<double, double> orientation,
                        double strengthNeighborRepulsion,
                        double rangeNeighborRepulsion,
                        double wallBufferDistance,
                        double anticipationTime,
                        double reactionTime,
                        double timeGap,
                        double desiredSpeed,
                        double radius) {
                return AnticipationVelocityModelData{
                    intoPoint(orientation),
                    strengthNeighborRepulsion,
                    rangeNeighborRepulsion,
                    wallBufferDistance,
                    anticipationTime,
                    reactionTime,
                    timeGap,
                    desiredSpeed,
                    radius};
            }),
            py::kw_only(),
            py::arg("orientation"),
            py::arg("strength_neighbor_repulsion"),
            py::arg("range_neighbor_repulsion"),
            py::arg("wall_buffer_distance"),
            py::arg("anticipation_time"),
            py::arg("reaction_time"),
            py::arg("time_gap"),
            py::arg("desired_speed"),
            py::arg("radius"))
        .def_property_readonly(
            "orientation",
            [](const AnticipationVelocityModelData& obj) { return intoTuple(obj.orientation); })
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
