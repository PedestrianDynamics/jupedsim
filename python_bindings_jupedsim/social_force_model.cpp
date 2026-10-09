// SPDX-License-Identifier: LGPL-3.0-or-later
#include "social_force_model.hpp"

#include "operational_model.hpp"
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
            py::init([](Point velocity,
                        double mass,
                        double desired_speed,
                        double reaction_time,
                        double agent_scale,
                        double obstacle_scale,
                        double force_distance,
                        double radius) {
                return SocialForceModel::State{
                    .velocity = velocity,
                    .mass = mass,
                    .desired_speed = desired_speed,
                    .reaction_time = reaction_time,
                    .agent_scale = agent_scale,
                    .obstacle_scale = obstacle_scale,
                    .force_distance = force_distance,
                    .radius = radius};
            }),
            py::kw_only(),
            py::arg("velocity") = d.velocity,
            py::arg("mass") = d.mass,
            py::arg("desired_speed") = d.desired_speed,
            py::arg("reaction_time") = d.reaction_time,
            py::arg("agent_scale") = d.agent_scale,
            py::arg("obstacle_scale") = d.obstacle_scale,
            py::arg("force_distance") = d.force_distance,
            py::arg("radius") = d.radius)
        .def_property_readonly(
            "orientation",
            [](const SocialForceModel::State& obj) { return obj.velocity.normalized(); })
        .def_readwrite("velocity", &SocialForceModel::State::velocity)
        .def_readwrite("mass", &SocialForceModel::State::mass)
        .def_readwrite("desired_speed", &SocialForceModel::State::desired_speed)
        .def_readwrite("reaction_time", &SocialForceModel::State::reaction_time)
        .def_readwrite("agent_scale", &SocialForceModel::State::agent_scale)
        .def_readwrite("obstacle_scale", &SocialForceModel::State::obstacle_scale)
        .def_readwrite("force_distance", &SocialForceModel::State::force_distance)
        .def_readwrite("radius", &SocialForceModel::State::radius);
}
