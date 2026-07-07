// SPDX-License-Identifier: LGPL-3.0-or-later
#include "OperationalModel.hpp"
#include "WarpDriverModel.hpp"
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
            py::init([](Point position,
                        Point orientation,
                        double radius,
                        double desiredSpeed,
                        double stuckTime,
                        double anchorX,
                        double anchorY,
                        double detourTime,
                        int detourSide) {
                return WarpDriverModel::State{
                    .position = position,
                    .orientation = orientation,
                    .radius = radius,
                    .v0 = desiredSpeed,
                    .stuckTime = stuckTime,
                    .anchorX = anchorX,
                    .anchorY = anchorY,
                    .detourTime = detourTime,
                    .detourSide = detourSide};
            }),
            py::kw_only(),
            py::arg("position") = d.position,
            py::arg("orientation") = d.orientation,
            py::arg("radius") = d.radius,
            py::arg("desired_speed") = d.v0,
            py::arg("stuck_time") = d.stuckTime,
            py::arg("anchor_x") = d.anchorX,
            py::arg("anchor_y") = d.anchorY,
            py::arg("detour_time") = d.detourTime,
            py::arg("detour_side") = d.detourSide)
        .def_readwrite("position", &WarpDriverModel::State::position)
        .def_readwrite("orientation", &WarpDriverModel::State::orientation)
        .def_readwrite("radius", &WarpDriverModel::State::radius)
        .def_readwrite("desired_speed", &WarpDriverModel::State::v0)
        .def_readwrite("stuck_time", &WarpDriverModel::State::stuckTime)
        .def_readwrite("anchor_x", &WarpDriverModel::State::anchorX)
        .def_readwrite("anchor_y", &WarpDriverModel::State::anchorY)
        .def_readwrite("detour_time", &WarpDriverModel::State::detourTime)
        .def_readwrite("detour_side", &WarpDriverModel::State::detourSide);
}
