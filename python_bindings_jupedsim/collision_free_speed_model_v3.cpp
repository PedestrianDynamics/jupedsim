// SPDX-License-Identifier: LGPL-3.0-or-later
#include "collision_free_speed_model_v3.hpp"

#include "conversion.hpp"
#include "operational_model.hpp"
#include "type_casters.hpp" // IWYU pragma: keep

#include <pybind11/cast.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h> // IWYU pragma: keep

namespace py = pybind11;

void init_collision_free_speed_model_v3(py::module_& m)
{
    py::class_<CollisionFreeSpeedModelV3, OperationalModel, py::smart_holder> model(
        m, "CollisionFreeSpeedModelV3");
    model.doc() = clean_doc(R"(
        Collision Free Speed Model V3.

        Variant of the Collision Free Speed Model with anisotropic neighbor
        interaction and relaxed heading dynamics. The walking direction is obtained
        by rotating a reference direction (the desired direction bent by wall
        repulsion) away from the most relevant forward neighbor, while the walking
        speed follows from a scalar spacing via the optimal-velocity relation. The
        heading angle is relaxed in time to suppress jitter.

        The model has no model-level parameters; all parameters are per agent, see
        :class:`~jupedsim.CollisionFreeSpeedModelV3State`. The model instance is
        passed to the simulation:

        .. code:: python

            sim = jupedsim.Simulation(
                model=jupedsim.CollisionFreeSpeedModelV3(),
                geometry=...,
            )
            sim.add_agent(
                journey_id=journey_id,
                stage_id=stage_id,
                position=(1.0, 1.0),
                state=jupedsim.CollisionFreeSpeedModelV3State(desired_speed=1.4),
                region_id=region_id,
            )

        .. warning::

            The model instance is consumed by the ``Simulation`` constructor and must
            not be reused afterwards.
    )");
    model.def(py::init<>());

    const CollisionFreeSpeedModelV3::State d{};
    py::class_<CollisionFreeSpeedModelV3::State> state(m, "CollisionFreeSpeedModelV3State");
    state.doc() = clean_doc(R"(
        Per-agent state of the Collision Free Speed Model V3.

        All arguments are keyword-only with sensible defaults, and every argument is
        also a read/write attribute of the same name.

        Args:
            orientation: Unit vector the agent faces.
            strength_neighbor_repulsion: Maximum turning angle [rad], clamped against
                ``theta_max_upper_bound``.
            range_neighbor_repulsion: Base perception range [m]; combined with the
                scale factors below to form the anisotropic decay lengths
                ``r_x = range_neighbor_repulsion * range_x_scale`` and
                ``r_y = range_neighbor_repulsion * range_y_scale``.
            strength_geometry_repulsion: Strength of the wall repulsion that bends the
                reference direction.
            range_geometry_repulsion: Decay length of the wall repulsion [m].
            range_x_scale: Longitudinal aspect factor of the perception field.
            range_y_scale: Lateral aspect factor of the perception field.
            theta_max_upper_bound: Hard upper bound on the per-step turning angle
                [rad].
            agent_buffer: Buffer distance subtracted from the spacing in the
                optimal-velocity relation [m]; shifts the speed-zero point to a
                positive spacing.
            time_gap: Time gap in the optimal-velocity relation [s].
            desired_speed: Maximum (free) walking speed of the agent [m/s].
            radius: Radius of the agent [m].
            heading_angle: Persistent heading used by the temporal relaxation [rad].
                Managed by the model, rarely set by users.
    )");
    state
        .def(
            py::init([](Point orientation,
                        double strength_neighbor_repulsion,
                        double range_neighbor_repulsion,
                        double strength_geometry_repulsion,
                        double range_geometry_repulsion,
                        double range_x_scale,
                        double range_y_scale,
                        double theta_max_upper_bound,
                        double agent_buffer,
                        double time_gap,
                        double desired_speed,
                        double radius,
                        double heading_angle) {
                return CollisionFreeSpeedModelV3::State{
                    .orientation = orientation,
                    .strength_neighbor_repulsion = strength_neighbor_repulsion,
                    .range_neighbor_repulsion = range_neighbor_repulsion,
                    .strength_geometry_repulsion = strength_geometry_repulsion,
                    .range_geometry_repulsion = range_geometry_repulsion,
                    .range_x_scale = range_x_scale,
                    .range_y_scale = range_y_scale,
                    .theta_max_upper_bound = theta_max_upper_bound,
                    .agent_buffer = agent_buffer,
                    .time_gap = time_gap,
                    .v0 = desired_speed,
                    .radius = radius,
                    .heading_angle = heading_angle};
            }),
            py::kw_only(),
            py::arg("orientation") = d.orientation,
            py::arg("strength_neighbor_repulsion") = d.strength_neighbor_repulsion,
            py::arg("range_neighbor_repulsion") = d.range_neighbor_repulsion,
            py::arg("strength_geometry_repulsion") = d.strength_geometry_repulsion,
            py::arg("range_geometry_repulsion") = d.range_geometry_repulsion,
            py::arg("range_x_scale") = d.range_x_scale,
            py::arg("range_y_scale") = d.range_y_scale,
            py::arg("theta_max_upper_bound") = d.theta_max_upper_bound,
            py::arg("agent_buffer") = d.agent_buffer,
            py::arg("time_gap") = d.time_gap,
            py::arg("desired_speed") = d.v0,
            py::arg("radius") = d.radius,
            py::arg("heading_angle") = d.heading_angle)
        .def_readwrite("orientation", &CollisionFreeSpeedModelV3::State::orientation)
        .def_readwrite(
            "strength_neighbor_repulsion",
            &CollisionFreeSpeedModelV3::State::strength_neighbor_repulsion)
        .def_readwrite(
            "range_neighbor_repulsion", &CollisionFreeSpeedModelV3::State::range_neighbor_repulsion)
        .def_readwrite(
            "strength_geometry_repulsion",
            &CollisionFreeSpeedModelV3::State::strength_geometry_repulsion)
        .def_readwrite(
            "range_geometry_repulsion", &CollisionFreeSpeedModelV3::State::range_geometry_repulsion)
        .def_readwrite("range_x_scale", &CollisionFreeSpeedModelV3::State::range_x_scale)
        .def_readwrite("range_y_scale", &CollisionFreeSpeedModelV3::State::range_y_scale)
        .def_readwrite(
            "theta_max_upper_bound", &CollisionFreeSpeedModelV3::State::theta_max_upper_bound)
        .def_readwrite("agent_buffer", &CollisionFreeSpeedModelV3::State::agent_buffer)
        .def_readwrite("time_gap", &CollisionFreeSpeedModelV3::State::time_gap)
        .def_readwrite("desired_speed", &CollisionFreeSpeedModelV3::State::v0)
        .def_readwrite("radius", &CollisionFreeSpeedModelV3::State::radius)
        .def_readwrite("heading_angle", &CollisionFreeSpeedModelV3::State::heading_angle);
}
