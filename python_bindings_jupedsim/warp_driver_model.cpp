// SPDX-License-Identifier: LGPL-3.0-or-later
#include "warp_driver_model.hpp"

#include "operational_model.hpp"
#include "type_casters.hpp" // IWYU pragma: keep

#include <pybind11/cast.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h> // IWYU pragma: keep

#include <cstdint>

namespace py = pybind11;

void init_warp_driver_model(py::module_& m)
{
    py::class_<WarpDriverModel, OperationalModel, py::smart_holder>(m, "WarpDriverModel")
        .def(
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
    py::class_<WarpDriverModel::State>(m, "WarpDriverModelState")
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
