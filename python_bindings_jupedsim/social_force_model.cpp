// SPDX-License-Identifier: LGPL-3.0-or-later
#include "social_force_model.hpp"

#include "conversion.hpp"
#include "operational_model.hpp"
#include "type_casters.hpp" // IWYU pragma: keep

#include <pybind11/cast.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h> // IWYU pragma: keep

namespace py = pybind11;

void init_social_force_model(py::module_& m)
{
    py::class_<SocialForceModel, OperationalModel, py::smart_holder> model(m, "SocialForceModel");
    model.doc() = clean_doc(R"(
        Social Force Model.

        See the scientific publication for more details about this model:
        https://doi.org/10.1038/35035023

        The model-level parameters (``body_force`` and ``friction``) are carried by
        the model instance, which is passed to the simulation:

        .. code:: python

            sim = jupedsim.Simulation(
                model=jupedsim.SocialForceModel(
                    body_force=120000, friction=240000
                ),
                geometry=...,
            )
            sim.add_agent(
                journey_id=journey_id,
                stage_id=stage_id,
                position=(1.0, 1.0),
                state=jupedsim.SocialForceModelState(mass=75.0),
                region_id=region_id,
            )

        .. warning::

            The model instance is consumed by the ``Simulation`` constructor and must
            not be reused afterwards.

        Args:
            body_force: Strength with which an agent is influenced by pushing forces
                from obstacles and neighbors in its direct proximity [kg s^-2] (k).
            friction: Strength with which an agent is influenced by frictional forces
                from obstacles and neighbors in its direct proximity [kg m^-1 s^-1]
                (kappa).
    )");
    model.def(
        py::init<double, double>(),
        py::kw_only(),
        py::arg("body_force") = 120000,
        py::arg("friction") = 240000);

    const SocialForceModel::State d{};
    py::class_<SocialForceModel::State> state(m, "SocialForceModelState");
    state.doc() = clean_doc(R"(
        Per-agent state of the Social Force Model.

        All arguments are keyword-only with sensible defaults, and every argument is
        also a read/write attribute of the same name.

        Args:
            velocity: Current velocity of the agent [m/s].
            mass: Mass of the agent [kg] (m).
            desired_speed: Desired speed of the agent [m/s] (v0).
            reaction_time: Reaction time of the agent [s] (tau).
            agent_scale: How strongly the agent is influenced by pushing forces from
                neighbors [N] (A).
            obstacle_scale: How strongly the agent is influenced by pushing forces
                from obstacles [N] (A).
            force_distance: How much the distance between the agent and obstacles or
                neighbors influences the social forces [m] (B).
            radius: Radius of the space the agent occupies [m] (r).

        The read-only attribute ``orientation`` is the normalized ``velocity``.
    )");
    state
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
