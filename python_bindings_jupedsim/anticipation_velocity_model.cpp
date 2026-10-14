// SPDX-License-Identifier: LGPL-3.0-or-later
#include "anticipation_velocity_model.hpp"

#include "conversion.hpp"
#include "operational_model.hpp"
#include "type_casters.hpp" // IWYU pragma: keep

#include <pybind11/cast.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h> // IWYU pragma: keep

#include <cstdint>

namespace py = pybind11;

void init_anticipation_velocity_model(py::module_& m)
{
    py::class_<AnticipationVelocityModel, OperationalModel, py::smart_holder> model(
        m, "AnticipationVelocityModel");
    model.doc() = clean_doc(R"(
        Anticipation Velocity Model (AVM).

        The AVM incorporates pedestrian anticipation, divided into three phases:
        perception of the current situation, prediction of future situations and
        strategy selection leading to action. A general description of the AVM can be
        found in the originating publication
        https://doi.org/10.1016/j.trc.2021.103464

        The model-level parameters (``pushout_strength`` and ``rng_seed``) are carried
        by the model instance, which is passed to the simulation:

        .. code:: python

            sim = jupedsim.Simulation(
                model=jupedsim.AnticipationVelocityModel(rng_seed=1234),
                geometry=...,
            )
            sim.add_agent(
                journey_id=journey_id,
                stage_id=stage_id,
                position=(1.0, 1.0),
                state=jupedsim.AnticipationVelocityModelState(),
                region_id=region_id,
            )

        .. warning::

            The model instance is consumed by the ``Simulation`` constructor and must
            not be reused afterwards.

        Args:
            pushout_strength: Scale of the outward component added to an agent's
                movement while it is within the critical wall distance. Combined with
                the parallel component of the agent's direction this keeps agents at
                a safe distance from walls and yields smooth gliding along them.
            rng_seed: Seed of the internally used random number generator.
    )");
    model.def(
        py::init<double, uint64_t>(),
        py::kw_only(),
        py::arg("pushout_strength") = 0.3,
        py::arg("rng_seed") = 42);

    const AnticipationVelocityModel::State d{};
    py::class_<AnticipationVelocityModel::State> state(m, "AnticipationVelocityModelState");
    state.doc() = clean_doc(R"(
        Per-agent state of the Anticipation Velocity Model.

        All arguments are keyword-only with sensible defaults, and every argument is
        also a read/write attribute of the same name.

        Args:
            orientation: Unit vector the agent faces.
            strength_neighbor_repulsion: Strength of the repulsion from neighbors.
            range_neighbor_repulsion: Range of the repulsion from neighbors [m].
            wall_buffer_distance: Buffer distance the agent keeps to walls [m].
            anticipation_time: Time span over which the agent anticipates the
                movement of its neighbors [s].
            reaction_time: Time the agent needs to change its direction [s].
            velocity: Current velocity of the agent [m/s].
            time_gap: Time constant that describes how fast a pedestrian closes gaps
                [s].
            desired_speed: Maximum (free) walking speed of the agent [m/s].
            radius: Radius of the agent [m].
    )");
    state
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
