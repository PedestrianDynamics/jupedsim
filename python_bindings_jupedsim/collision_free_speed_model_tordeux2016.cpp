// SPDX-License-Identifier: LGPL-3.0-or-later
#include "collision_free_speed_model_tordeux2016.hpp"

#include "conversion.hpp"
#include "kernel.hpp"
#include "operational_model.hpp"
#include "type_casters.hpp" // IWYU pragma: keep

#include <pybind11/cast.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h> // IWYU pragma: keep

namespace py = pybind11;

void init_collision_free_speed_model_tordeux2016(py::module_& m)
{
    py::class_<CollisionFreeSpeedModelTordeux2016, OperationalModel, py::smart_holder> model(
        m, "CollisionFreeSpeedModelTordeux2016");
    model.doc() = clean_doc(R"(
        Collision Free Speed Model as published by Tordeux et al. (2016).

        This model is kept as published, so that results stay reproducible against
        the paper. For new simulations use :class:`~jupedsim.CollisionFreeSpeedModel`.

        A general description of the Collision Free Speed Model can be found in the
        originating publication https://arxiv.org/abs/1512.05597. A more detailed
        description can be found at
        https://pedestriandynamics.org/models/collision_free_speed_model/

        The model-level repulsion parameters are carried by the model instance, which
        is passed to the simulation:

        .. code:: python

            sim = jupedsim.Simulation(
                model=jupedsim.CollisionFreeSpeedModelTordeux2016(),
                geometry=...,
            )
            sim.add_agent(
                journey_id=journey_id,
                stage_id=stage_id,
                position=(1.0, 1.0),
                state=jupedsim.CollisionFreeSpeedModelTordeux2016State(desired_speed=1.4),
                region_id=region_id,
            )

        .. warning::

            The model instance is consumed by the ``Simulation`` constructor and must
            not be reused afterwards.

        Args:
            strength_neighbor_repulsion: Strength of the repulsion from neighbors.
            range_neighbor_repulsion: Range of the repulsion from neighbors [m].
            strength_geometry_repulsion: Strength of the repulsion from geometry
                boundaries.
            range_geometry_repulsion: Range of the repulsion from geometry boundaries
                [m].
    )");
    model.def(
        py::init<double, double, double, double>(),
        py::kw_only(),
        py::arg("strength_neighbor_repulsion") = 8.0,
        py::arg("range_neighbor_repulsion") = 0.1,
        py::arg("strength_geometry_repulsion") = 5.0,
        py::arg("range_geometry_repulsion") = 0.02);

    const CollisionFreeSpeedModelTordeux2016::State d{};
    py::class_<CollisionFreeSpeedModelTordeux2016::State> state(
        m, "CollisionFreeSpeedModelTordeux2016State");
    state.doc() = clean_doc(R"(
        Per-agent state of the Collision Free Speed Model.

        All arguments are keyword-only with sensible defaults, and every argument is
        also a read/write attribute of the same name.

        Args:
            orientation: Unit vector the agent faces.
            time_gap: Time constant that describes how fast a pedestrian closes gaps
                [s].
            desired_speed: Maximum (free) walking speed of the agent [m/s].
            radius: Radius of the agent [m].
    )");
    state
        .def(
            py::init([](Vector orientation, double time_gap, double desired_speed, double radius) {
                return CollisionFreeSpeedModelTordeux2016::State{
                    .orientation = orientation,
                    .time_gap = time_gap,
                    .v0 = desired_speed,
                    .radius = radius};
            }),
            py::kw_only(),
            py::arg("orientation") = d.orientation,
            py::arg("time_gap") = d.time_gap,
            py::arg("desired_speed") = d.v0,
            py::arg("radius") = d.radius)
        .def_readwrite("orientation", &CollisionFreeSpeedModelTordeux2016::State::orientation)
        .def_readwrite("time_gap", &CollisionFreeSpeedModelTordeux2016::State::time_gap)
        .def_readwrite("desired_speed", &CollisionFreeSpeedModelTordeux2016::State::v0)
        .def_readwrite("radius", &CollisionFreeSpeedModelTordeux2016::State::radius);
}
