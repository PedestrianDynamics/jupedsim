// SPDX-License-Identifier: LGPL-3.0-or-later
#include "collision_free_speed_model_v2.hpp"

#include "conversion.hpp"
#include "operational_model.hpp"
#include "type_casters.hpp" // IWYU pragma: keep

#include <pybind11/cast.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h> // IWYU pragma: keep

namespace py = pybind11;

void init_collision_free_speed_model_v2(py::module_& m)
{
    py::class_<CollisionFreeSpeedModelV2, OperationalModel, py::smart_holder> model(
        m, "CollisionFreeSpeedModelV2");
    model.doc() = clean_doc(R"(
        Collision Free Speed Model V2.

        Variant of the Collision Free Speed Model (https://arxiv.org/abs/1512.05597)
        in which all repulsion parameters are per-agent instead of global.

        The model has no model-level parameters; all parameters are per agent, see
        :class:`~jupedsim.CollisionFreeSpeedModelV2State`. The model instance is
        passed to the simulation:

        .. code:: python

            sim = jupedsim.Simulation(
                model=jupedsim.CollisionFreeSpeedModelV2(),
                geometry=...,
            )
            sim.add_agent(
                journey_id=journey_id,
                stage_id=stage_id,
                position=(1.0, 1.0),
                state=jupedsim.CollisionFreeSpeedModelV2State(strength_neighbor_repulsion=9.0),
                region_id=region_id,
            )

        .. warning::

            The model instance is consumed by the ``Simulation`` constructor and must
            not be reused afterwards.
    )");
    model.def(py::init<>());

    const CollisionFreeSpeedModelV2::State d{};
    py::class_<CollisionFreeSpeedModelV2::State> state(m, "CollisionFreeSpeedModelV2State");
    state.doc() = clean_doc(R"(
        Per-agent state of the Collision Free Speed Model V2.

        All arguments are keyword-only with sensible defaults, and every argument is
        also a read/write attribute of the same name.

        Args:
            orientation: Unit vector the agent faces.
            strength_neighbor_repulsion: Strength of the repulsion from neighbors.
            range_neighbor_repulsion: Range of the repulsion from neighbors [m].
            strength_geometry_repulsion: Strength of the repulsion from geometry
                boundaries.
            range_geometry_repulsion: Range of the repulsion from geometry boundaries
                [m].
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
                        double strength_geometry_repulsion,
                        double range_geometry_repulsion,
                        double time_gap,
                        double desired_speed,
                        double radius) {
                return CollisionFreeSpeedModelV2::State{
                    .orientation = orientation,
                    .strength_neighbor_repulsion = strength_neighbor_repulsion,
                    .range_neighbor_repulsion = range_neighbor_repulsion,
                    .strength_geometry_repulsion = strength_geometry_repulsion,
                    .range_geometry_repulsion = range_geometry_repulsion,
                    .time_gap = time_gap,
                    .v0 = desired_speed,
                    .radius = radius};
            }),
            py::kw_only(),
            py::arg("orientation") = d.orientation,
            py::arg("strength_neighbor_repulsion") = d.strength_neighbor_repulsion,
            py::arg("range_neighbor_repulsion") = d.range_neighbor_repulsion,
            py::arg("strength_geometry_repulsion") = d.strength_geometry_repulsion,
            py::arg("range_geometry_repulsion") = d.range_geometry_repulsion,
            py::arg("time_gap") = d.time_gap,
            py::arg("desired_speed") = d.v0,
            py::arg("radius") = d.radius)
        .def_readwrite("orientation", &CollisionFreeSpeedModelV2::State::orientation)
        .def_readwrite(
            "strength_neighbor_repulsion",
            &CollisionFreeSpeedModelV2::State::strength_neighbor_repulsion)
        .def_readwrite(
            "range_neighbor_repulsion", &CollisionFreeSpeedModelV2::State::range_neighbor_repulsion)
        .def_readwrite(
            "strength_geometry_repulsion",
            &CollisionFreeSpeedModelV2::State::strength_geometry_repulsion)
        .def_readwrite(
            "range_geometry_repulsion", &CollisionFreeSpeedModelV2::State::range_geometry_repulsion)
        .def_readwrite("time_gap", &CollisionFreeSpeedModelV2::State::time_gap)
        .def_readwrite("desired_speed", &CollisionFreeSpeedModelV2::State::v0)
        .def_readwrite("radius", &CollisionFreeSpeedModelV2::State::radius);
}
