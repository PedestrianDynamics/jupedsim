// SPDX-License-Identifier: LGPL-3.0-or-later
#include "collision_free_speed_model.hpp"

#include "conversion.hpp"
#include "kernel.hpp"
#include "operational_model.hpp"
#include "type_casters.hpp" // IWYU pragma: keep

#include <pybind11/cast.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h> // IWYU pragma: keep

namespace py = pybind11;

void init_collision_free_speed_model(py::module_& m)
{
    py::class_<CollisionFreeSpeedModel, OperationalModel, py::smart_holder> model(
        m, "CollisionFreeSpeedModel");
    model.doc() = clean_doc(R"(
        Collision Free Speed Model.

        The default collision-free speed model. It is based on the model of Tordeux
        et al. (2016) (https://arxiv.org/abs/1512.05597, available unchanged as
        :class:`~jupedsim.CollisionFreeSpeedModelTordeux2016`), with all repulsion
        parameters per agent instead of global, a vision factor on the neighbor
        repulsion (``neighbor_repulsion_anisotropy``) and calibrated default
        parameters.

        The model has no model-level parameters; all parameters are per agent, see
        :class:`~jupedsim.CollisionFreeSpeedModelState`. The model instance is
        passed to the simulation:

        .. code:: python

            sim = jupedsim.Simulation(
                model=jupedsim.CollisionFreeSpeedModel(),
                geometry=...,
            )
            sim.add_agent(
                region.at(1.0, 1.0),
                route=route,
                state=jupedsim.CollisionFreeSpeedModelState(strength_neighbor_repulsion=9.0),
            )

        .. warning::

            The model instance is consumed by the ``Simulation`` constructor and must
            not be reused afterwards.
    )");
    model.def(py::init<>());

    const CollisionFreeSpeedModel::State d{};
    py::class_<CollisionFreeSpeedModel::State> state(m, "CollisionFreeSpeedModelState");
    state.doc() = clean_doc(R"(
        Per-agent state of the Collision Free Speed Model.

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
            neighbor_repulsion_anisotropy: Weight of the vision factor on the
                neighbor repulsion, in [0, 1]. A neighbor counts with the cosine
                of its angle to the desired direction and not at all when it is
                behind (as in the Generalized Centrifugal Force Model). 0 is
                isotropic repulsion, 1 is the full vision factor.
            time_gap: Time constant that describes how fast a pedestrian closes gaps
                [s].
            desired_speed: Maximum (free) walking speed of the agent [m/s].
            radius: Radius of the agent [m].
    )");
    state
        .def(
            py::init([](Vector orientation,
                        double strength_neighbor_repulsion,
                        double range_neighbor_repulsion,
                        double strength_geometry_repulsion,
                        double range_geometry_repulsion,
                        double neighbor_repulsion_anisotropy,
                        double time_gap,
                        double desired_speed,
                        double radius) {
                return CollisionFreeSpeedModel::State{
                    .orientation = orientation,
                    .strength_neighbor_repulsion = strength_neighbor_repulsion,
                    .range_neighbor_repulsion = range_neighbor_repulsion,
                    .strength_geometry_repulsion = strength_geometry_repulsion,
                    .range_geometry_repulsion = range_geometry_repulsion,
                    .neighbor_repulsion_anisotropy = neighbor_repulsion_anisotropy,
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
            py::arg("neighbor_repulsion_anisotropy") = d.neighbor_repulsion_anisotropy,
            py::arg("time_gap") = d.time_gap,
            py::arg("desired_speed") = d.v0,
            py::arg("radius") = d.radius)
        .def_readwrite("orientation", &CollisionFreeSpeedModel::State::orientation)
        .def_readwrite(
            "strength_neighbor_repulsion",
            &CollisionFreeSpeedModel::State::strength_neighbor_repulsion)
        .def_readwrite(
            "range_neighbor_repulsion", &CollisionFreeSpeedModel::State::range_neighbor_repulsion)
        .def_readwrite(
            "strength_geometry_repulsion",
            &CollisionFreeSpeedModel::State::strength_geometry_repulsion)
        .def_readwrite(
            "range_geometry_repulsion", &CollisionFreeSpeedModel::State::range_geometry_repulsion)
        .def_readwrite(
            "neighbor_repulsion_anisotropy",
            &CollisionFreeSpeedModel::State::neighbor_repulsion_anisotropy)
        .def_readwrite("time_gap", &CollisionFreeSpeedModel::State::time_gap)
        .def_readwrite("desired_speed", &CollisionFreeSpeedModel::State::v0)
        .def_readwrite("radius", &CollisionFreeSpeedModel::State::radius);
}
