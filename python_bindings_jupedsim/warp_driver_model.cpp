// SPDX-License-Identifier: LGPL-3.0-or-later
#include "OperationalModel.hpp"
#include "WarpDriverModel.hpp"
#include "WarpDriverModelBuilder.hpp"
#include "conversion.hpp"
#include "type_casters.hpp" // IWYU pragma: keep

#include <pybind11/cast.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h> // IWYU pragma: keep

#include <tuple>

namespace py = pybind11;

void init_warp_driver_model(py::module_& m)
{
    py::class_<WarpDriverModel, OperationalModel, py::smart_holder>(m, "WarpDriverModel");
    py::class_<WarpDriverModelBuilder>(m, "WarpDriverModelBuilder")
        .def(
            py::init<double, double, double, double, double, double>(),
            py::kw_only(),
            py::arg("time_horizon") = 2.0,
            py::arg("step_size") = 0.5,
            py::arg("sigma") = 0.3,
            py::arg("time_uncertainty") = 0.5,
            py::arg("velocity_uncertainty_x") = 0.2,
            py::arg("velocity_uncertainty_y") = 0.2)
        .def("build", &WarpDriverModelBuilder::Build);
    py::class_<WarpDriverModel::State>(m, "WarpDriverModelState")
        .def(
            py::init([](std::tuple<double, double> orientation,
                        double desired_speed,
                        double radius) {
                return WarpDriverModel::State{
                    .orientation = intoPoint(orientation), .radius = radius, .v0 = desired_speed};
            }),
            py::kw_only(),
            py::arg("orientation"),
            py::arg("desired_speed") = 1.2,
            py::arg("radius") = 0.15)
        .def_readwrite("orientation", &WarpDriverModel::State::orientation)
        .def_readwrite("radius", &WarpDriverModel::State::radius)
        .def_readwrite("desired_speed", &WarpDriverModel::State::v0);
}
