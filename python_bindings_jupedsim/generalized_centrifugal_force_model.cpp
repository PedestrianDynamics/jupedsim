// SPDX-License-Identifier: LGPL-3.0-or-later
#include "generalized_centrifugal_force_model.hpp"

#include "operational_model.hpp"
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
