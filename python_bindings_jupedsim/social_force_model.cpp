// SPDX-License-Identifier: LGPL-3.0-or-later
#include "OperationalModel.hpp"
#include "SocialForceModel.hpp"
#include "type_casters.hpp" // IWYU pragma: keep

#include <pybind11/cast.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h> // IWYU pragma: keep

namespace py = pybind11;

void init_social_force_model(py::module_& m)
{
    py::class_<SocialForceModel, OperationalModel, py::smart_holder>(m, "SocialForceModel")
        .def(
            py::init<double, double>(),
            py::kw_only(),
            py::arg("body_force") = 120000,
            py::arg("friction") = 240000);
    const SocialForceModel::State d{};
    py::class_<SocialForceModel::State>(m, "SocialForceModelState")
        .def(
            py::init([](Point position,
                        Point velocity,
                        double mass,
                        double desiredSpeed,
                        double reactionTime,
                        double agentScale,
                        double obstacleScale,
                        double forceDistance,
                        double radius) {
                return SocialForceModel::State{
                    .position = position,
                    .velocity = velocity,
                    .mass = mass,
                    .desiredSpeed = desiredSpeed,
                    .reactionTime = reactionTime,
                    .agentScale = agentScale,
                    .obstacleScale = obstacleScale,
                    .forceDistance = forceDistance,
                    .radius = radius};
            }),
            py::kw_only(),
            py::arg("position") = d.position,
            py::arg("velocity") = d.velocity,
            py::arg("mass") = d.mass,
            py::arg("desired_speed") = d.desiredSpeed,
            py::arg("reaction_time") = d.reactionTime,
            py::arg("agent_scale") = d.agentScale,
            py::arg("obstacle_scale") = d.obstacleScale,
            py::arg("force_distance") = d.forceDistance,
            py::arg("radius") = d.radius)
        .def_property_readonly(
            "orientation",
            [](const SocialForceModel::State& obj) { return obj.velocity.Normalized(); })
        .def_readwrite("position", &SocialForceModel::State::position)
        .def_readwrite("velocity", &SocialForceModel::State::velocity)
        .def_readwrite("mass", &SocialForceModel::State::mass)
        .def_readwrite("desired_speed", &SocialForceModel::State::desiredSpeed)
        .def_readwrite("reaction_time", &SocialForceModel::State::reactionTime)
        .def_readwrite("agent_scale", &SocialForceModel::State::agentScale)
        .def_readwrite("obstacle_scale", &SocialForceModel::State::obstacleScale)
        .def_readwrite("force_distance", &SocialForceModel::State::forceDistance)
        .def_readwrite("radius", &SocialForceModel::State::radius);
}
