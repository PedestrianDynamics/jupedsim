// SPDX-License-Identifier: LGPL-3.0-or-later
#include "generalized_centrifugal_force_model.hpp"

#include "conversion.hpp"
#include "operational_model.hpp"
#include "type_casters.hpp" // IWYU pragma: keep

#include <pybind11/cast.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h> // IWYU pragma: keep

namespace py = pybind11;

void init_generalized_centrifugal_force_model(py::module_& m)
{
    py::class_<GeneralizedCentrifugalForceModel, OperationalModel, py::smart_holder> model(
        m, "GeneralizedCentrifugalForceModel");
    model.doc() = clean_doc(R"(
        Generalized Centrifugal Force Model.

        See the scientific publication for more details about this model:
        https://arxiv.org/abs/1008.4297

        The model-level repulsion parameters are carried by the model instance, which
        is passed to the simulation:

        .. code:: python

            sim = jupedsim.Simulation(
                model=jupedsim.GeneralizedCentrifugalForceModel(),
                geometry=...,
            )
            sim.add_agent(
                journey_id=journey_id,
                stage_id=stage_id,
                position=(1.0, 1.0),
                state=jupedsim.GeneralizedCentrifugalForceModelState(desired_speed=1.0),
                region_id=region_id,
            )

        .. warning::

            The model instance is consumed by the ``Simulation`` constructor and must
            not be reused afterwards.

        Args:
            strength_neighbor_repulsion: Strength of the repulsion from neighbors.
            strength_wall_repulsion: Strength of the repulsion from geometry
                boundaries.
            max_neighbor_interaction_distance: Cut-off radius for the ped-ped
                repulsion [m] (r_c in FIG. 7 of the publication).
            max_geometry_interaction_distance: Cut-off radius for the ped-wall
                repulsion [m] (r_c in FIG. 7).
            max_neighbor_interpolation_distance: Distance below which the ped-ped
                repulsive force is interpolated [m] (r_eps in FIG. 7).
            max_geometry_interpolation_distance: Distance below which the ped-wall
                repulsive force is interpolated [m] (r_eps in FIG. 7).
            max_neighbor_repulsion_force: Maximum ped-ped repulsion force on contact
                of the ellipses (f_m in FIG. 7).
            max_geometry_repulsion_force: Maximum ped-wall repulsion force on contact
                of the ellipses (f_m in FIG. 7).
    )");
    model.def(
        py::init<double, double, double, double, double, double, double, double>(),
        py::kw_only(),
        py::arg("strength_neighbor_repulsion") = 0.3,
        py::arg("strength_wall_repulsion") = 0.2,
        py::arg("max_neighbor_interaction_distance") = 2,
        py::arg("max_geometry_interaction_distance") = 2,
        py::arg("max_neighbor_interpolation_distance") = 0.1,
        py::arg("max_geometry_interpolation_distance") = 0.1,
        py::arg("max_neighbor_repulsion_force") = 9,
        py::arg("max_geometry_repulsion_force") = 3);

    const GeneralizedCentrifugalForceModel::State d{};
    py::class_<GeneralizedCentrifugalForceModel::State> state(
        m, "GeneralizedCentrifugalForceModelState");
    state.doc() = clean_doc(R"(
        Per-agent state of the Generalized Centrifugal Force Model.

        All arguments are keyword-only with sensible defaults, and every argument is
        also a read/write attribute of the same name.

        Args:
            orientation: Unit vector the agent faces. Has to be normalized to length
                1.0.
            speed: Current speed of the agent [m/s].
            desired_direction: Desired direction of the agent (e0).
            orientation_delay: Number of steps since the agent was added; eases the
                initial turn toward the target. Managed by the model, rarely set by
                users.
            mass: Mass of the agent [kg].
            tau: Time constant that describes how fast the agent accelerates to its
                desired speed [s].
            desired_speed: Maximum speed of the agent [m/s] (v0).
            a_v: Stretch of the ellipse semi-axis along the movement vector.
            a_min: Minimum length of the ellipse semi-axis along the movement vector
                [m].
            b_min: Minimum length of the ellipse semi-axis orthogonal to the
                movement vector [m].
            b_max: Maximum length of the ellipse semi-axis orthogonal to the
                movement vector [m].
    )");
    state
        .def(
            py::init([](Point orientation,
                        double speed,
                        Point desired_direction,
                        int orientation_delay,
                        double mass,
                        double tau,
                        double desired_speed,
                        double av,
                        double amin,
                        double bmin,
                        double bmax) {
                return GeneralizedCentrifugalForceModel::State{
                    .orientation = orientation,
                    .speed = speed,
                    .e0 = desired_direction,
                    .orientation_delay = orientation_delay,
                    .mass = mass,
                    .tau = tau,
                    .v0 = desired_speed,
                    .av = av,
                    .a_min = amin,
                    .b_min = bmin,
                    .b_max = bmax};
            }),
            py::kw_only(),
            py::arg("orientation") = d.orientation,
            py::arg("speed") = d.speed,
            py::arg("desired_direction") = d.e0,
            py::arg("orientation_delay") = d.orientation_delay,
            py::arg("mass") = d.mass,
            py::arg("tau") = d.tau,
            py::arg("desired_speed") = d.v0,
            py::arg("a_v") = d.av,
            py::arg("a_min") = d.a_min,
            py::arg("b_min") = d.b_min,
            py::arg("b_max") = d.b_max)
        .def_readwrite("orientation", &GeneralizedCentrifugalForceModel::State::orientation)
        .def_readwrite("speed", &GeneralizedCentrifugalForceModel::State::speed)
        .def_readwrite("desired_direction", &GeneralizedCentrifugalForceModel::State::e0)
        .def_readwrite(
            "orientation_delay", &GeneralizedCentrifugalForceModel::State::orientation_delay)
        .def_readwrite("mass", &GeneralizedCentrifugalForceModel::State::mass)
        .def_readwrite("tau", &GeneralizedCentrifugalForceModel::State::tau)
        .def_readwrite("desired_speed", &GeneralizedCentrifugalForceModel::State::v0)
        .def_readwrite("a_v", &GeneralizedCentrifugalForceModel::State::av)
        .def_readwrite("a_min", &GeneralizedCentrifugalForceModel::State::a_min)
        .def_readwrite("b_min", &GeneralizedCentrifugalForceModel::State::b_min)
        .def_readwrite("b_max", &GeneralizedCentrifugalForceModel::State::b_max);
}
