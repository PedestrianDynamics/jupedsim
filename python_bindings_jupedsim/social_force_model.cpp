// SPDX-License-Identifier: LGPL-3.0-or-later
#include "conversion.hpp"

#include <SocialForceModel.hpp>
#include <SocialForceModelBuilder.hpp>
#include <SocialForceModelData.hpp>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

void init_social_force_model(py::module_& m)
{
    py::class_<SocialForceModel, OperationalModel>(m, "SocialForceModel");
    py::class_<SocialForceModelBuilder>(m, "SocialForceModelBuilder")
        .def(py::init<double, double>(), py::kw_only(), py::arg("body_force"), py::arg("friction"))
        .def("build", &SocialForceModelBuilder::Build);
    py::class_<SocialForceModelData>(m, "SocialForceModelState")
        .def(
            py::init([](std::tuple<double, double> velocity,
                        double mass,
                        double desiredSpeed,
                        double reactionTime,
                        double agentScale,
                        double obstacleScale,
                        double forceDistance,
                        double radius) {
                return SocialForceModelData{
                    .velocity = intoPoint(velocity),
                    .mass = mass,
                    .desiredSpeed = desiredSpeed,
                    .reactionTime = reactionTime,
                    .agentScale = agentScale,
                    .obstacleScale = obstacleScale,
                    .forceDistance = forceDistance,
                    .radius = radius};
            }),
            py::kw_only(),
            py::arg("velocity"),
            py::arg("mass"),
            py::arg("desired_speed"),
            py::arg("reaction_time"),
            py::arg("agent_scale"),
            py::arg("obstacle_scale"),
            py::arg("force_distance"),
            py::arg("radius"))
        .def_property(
            "velocity",
            [](const SocialForceModelData& obj) { return intoTuple(obj.velocity); },
            [](SocialForceModelData& obj, std::tuple<double, double> pt) {
                obj.velocity = intoPoint(pt);
            })
        .def_readwrite("mass", &SocialForceModelData::mass)
        .def_readwrite("desired_speed", &SocialForceModelData::desiredSpeed)
        .def_readwrite("reaction_time", &SocialForceModelData::reactionTime)
        .def_readwrite("agent_scale", &SocialForceModelData::agentScale)
        .def_readwrite("obstacle_scale", &SocialForceModelData::obstacleScale)
        .def_readwrite("force_distance", &SocialForceModelData::forceDistance)
        .def_readwrite("radius", &SocialForceModelData::radius);
}
