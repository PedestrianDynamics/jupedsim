// SPDX-License-Identifier: LGPL-3.0-or-later
#include "warp_driver_model.hpp"

#include "conversion.hpp"
#include "operational_model.hpp"
#include "type_casters.hpp" // IWYU pragma: keep

#include <pybind11/cast.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h> // IWYU pragma: keep

#include <cstdint>

namespace py = pybind11;

void init_warp_driver_model(py::module_& m)
{
    py::class_<WarpDriverModel, OperationalModel, py::smart_holder> model(m, "WarpDriverModel");
    model.doc() = clean_doc(R"(
        WarpDriver Model.

        Based on Wolinski, Lin, and Pettré (2016) -- probabilistic collision
        avoidance using warped intrinsic fields.

        The model-level parameters (the precomputed intrinsic collision-probability
        field controlled by ``sigma``, the sampling and look-ahead parameters and a
        random number generator seeded by ``rng_seed``) are carried by the model
        instance, which is passed to the simulation:

        .. code:: python

            sim = jupedsim.Simulation(
                model=jupedsim.WarpDriverModel(sigma=0.3, rng_seed=42),
                geometry=...,
            )
            sim.add_agent(
                journey_id=journey_id,
                stage_id=stage_id,
                position=(1.0, 1.0),
                state=jupedsim.WarpDriverModelState(),
                region_id=region_id,
            )

        .. warning::

            The model instance is consumed by the ``Simulation`` constructor and must
            not be reused afterwards.

        Args:
            sigma: Gaussian spread of the intrinsic field. Larger values create
                smoother, wider collision zones.
            time_horizon: Look-ahead time for collision prediction [s]. Larger values
                detect collisions earlier but increase computation.
            step_size: Gradient descent step size (alpha). Controls how aggressively
                agents deviate from their projected trajectory.
            time_uncertainty: Time uncertainty parameter (lambda). Spreads the
                collision field along the time axis, so collisions further in the
                future are treated as less certain.
            velocity_uncertainty_x: Longitudinal velocity uncertainty. Compresses the
                collision field along the direction of motion.
            velocity_uncertainty_y: Lateral velocity uncertainty. Expands the
                collision field perpendicular to the direction of motion.
            num_samples: Number of evenly spaced sample points on the projected
                trajectory.
            rng_seed: Seed of the random number generator used for symmetry-breaking
                perturbations and detour side selection. A fixed seed gives
                reproducible runs.
    )");
    model.def(
        py::init<double, double, double, double, double, double, int, uint64_t>(),
        py::kw_only(),
        py::arg("sigma") = 0.3,
        py::arg("time_horizon") = 2.0,
        py::arg("step_size") = 0.5,
        py::arg("time_uncertainty") = 0.5,
        py::arg("velocity_uncertainty_x") = 0.2,
        py::arg("velocity_uncertainty_y") = 0.2,
        py::arg("num_samples") = 20,
        py::arg("rng_seed") = 42);

    const WarpDriverModel::State d{};
    py::class_<WarpDriverModel::State> state(m, "WarpDriverModelState");
    state.doc() = clean_doc(R"(
        Per-agent state of the WarpDriver Model.

        All arguments are keyword-only with sensible defaults, and every argument is
        also a read/write attribute of the same name. The bookkeeping of the stuck
        detection (``stuck_time``, ``displacement_x``, ``displacement_y``,
        ``detour_time`` and ``detour_side``) is managed by the model and rarely set
        by users.

        Args:
            orientation: Unit vector the agent faces.
            radius: Physical radius of the agent [m]. Used in the Minkowski sum for
                collision detection and in the short-range repulsion.
            desired_speed: Free-flow speed the agent tries to maintain [m/s].
            stuck_time: Time the agent has stayed within the stuck radius of its
                anchor position [s].
            displacement_x: Net displacement from the anchor position along x [m].
            displacement_y: Net displacement from the anchor position along y [m].
            detour_time: Remaining duration of the current detour [s]; zero when
                the agent is not detouring.
            detour_side: Side of the lateral detour, ``1`` or ``-1``.
    )");
    state
        .def(
            py::init([](Point orientation,
                        double radius,
                        double desired_speed,
                        double stuck_time,
                        double displacement_x,
                        double displacement_y,
                        double detour_time,
                        int detour_side) {
                return WarpDriverModel::State{
                    .orientation = orientation,
                    .radius = radius,
                    .v0 = desired_speed,
                    .stuck_time = stuck_time,
                    .displacement_x = displacement_x,
                    .displacement_y = displacement_y,
                    .detour_time = detour_time,
                    .detour_side = detour_side};
            }),
            py::kw_only(),
            py::arg("orientation") = d.orientation,
            py::arg("radius") = d.radius,
            py::arg("desired_speed") = d.v0,
            py::arg("stuck_time") = d.stuck_time,
            py::arg("displacement_x") = d.displacement_x,
            py::arg("displacement_y") = d.displacement_y,
            py::arg("detour_time") = d.detour_time,
            py::arg("detour_side") = d.detour_side)
        .def_readwrite("orientation", &WarpDriverModel::State::orientation)
        .def_readwrite("radius", &WarpDriverModel::State::radius)
        .def_readwrite("desired_speed", &WarpDriverModel::State::v0)
        .def_readwrite("stuck_time", &WarpDriverModel::State::stuck_time)
        .def_readwrite("displacement_x", &WarpDriverModel::State::displacement_x)
        .def_readwrite("displacement_y", &WarpDriverModel::State::displacement_y)
        .def_readwrite("detour_time", &WarpDriverModel::State::detour_time)
        .def_readwrite("detour_side", &WarpDriverModel::State::detour_side);
}
