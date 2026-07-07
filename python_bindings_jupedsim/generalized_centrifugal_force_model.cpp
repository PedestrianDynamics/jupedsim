// SPDX-License-Identifier: LGPL-3.0-or-later
#include "GeneralizedCentrifugalForceModel.hpp"
#include "OperationalModel.hpp"
#include "type_casters.hpp" // IWYU pragma: keep

#include <pybind11/cast.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h> // IWYU pragma: keep

namespace py = pybind11;

void init_generalized_centrifugal_force_model(py::module_& m)
{
    py::class_<GeneralizedCentrifugalForceModel, OperationalModel, py::smart_holder>(
        m, "GeneralizedCentrifugalForceModel")
        .def(
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
    py::class_<GeneralizedCentrifugalForceModel::State>(m, "GeneralizedCentrifugalForceModelState")
        .def(
            py::init([](Point position,
                        Point orientation,
                        double speed,
                        Point desiredDirection,
                        int orientationDelay,
                        double mass,
                        double tau,
                        double desiredSpeed,
                        double av,
                        double amin,
                        double bmin,
                        double bmax) {
                return GeneralizedCentrifugalForceModel::State{
                    .position = position,
                    .orientation = orientation,
                    .speed = speed,
                    .e0 = desiredDirection,
                    .orientationDelay = orientationDelay,
                    .mass = mass,
                    .tau = tau,
                    .v0 = desiredSpeed,
                    .Av = av,
                    .AMin = amin,
                    .BMin = bmin,
                    .BMax = bmax};
            }),
            py::kw_only(),
            py::arg("position") = d.position,
            py::arg("orientation") = d.orientation,
            py::arg("speed") = d.speed,
            py::arg("desired_direction") = d.e0,
            py::arg("orientation_delay") = d.orientationDelay,
            py::arg("mass") = d.mass,
            py::arg("tau") = d.tau,
            py::arg("desired_speed") = d.v0,
            py::arg("a_v") = d.Av,
            py::arg("a_min") = d.AMin,
            py::arg("b_min") = d.BMin,
            py::arg("b_max") = d.BMax)
        .def_readwrite("position", &GeneralizedCentrifugalForceModel::State::position)
        .def_readwrite("orientation", &GeneralizedCentrifugalForceModel::State::orientation)
        .def_readwrite("speed", &GeneralizedCentrifugalForceModel::State::speed)
        .def_readwrite("desired_direction", &GeneralizedCentrifugalForceModel::State::e0)
        .def_readwrite(
            "orientation_delay", &GeneralizedCentrifugalForceModel::State::orientationDelay)
        .def_readwrite("mass", &GeneralizedCentrifugalForceModel::State::mass)
        .def_readwrite("tau", &GeneralizedCentrifugalForceModel::State::tau)
        .def_readwrite("desired_speed", &GeneralizedCentrifugalForceModel::State::v0)
        .def_readwrite("a_v", &GeneralizedCentrifugalForceModel::State::Av)
        .def_readwrite("a_min", &GeneralizedCentrifugalForceModel::State::AMin)
        .def_readwrite("b_min", &GeneralizedCentrifugalForceModel::State::BMin)
        .def_readwrite("b_max", &GeneralizedCentrifugalForceModel::State::BMax);
}
