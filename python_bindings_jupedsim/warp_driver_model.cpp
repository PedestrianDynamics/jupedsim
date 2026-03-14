// SPDX-License-Identifier: LGPL-3.0-or-later
#include "OperationalModel.hpp"
#include "WarpDriverModel.hpp"
#include "WarpDriverModelBuilder.hpp"
#include "WarpDriverModelData.hpp"

#include <pybind11/pybind11.h>

namespace py = pybind11;

void init_warp_driver_model(py::module_& m)
{
    py::class_<WarpDriverModel, OperationalModel>(m, "WarpDriverModel");
    py::class_<WarpDriverModelBuilder>(m, "WarpDriverModelBuilder")
        .def(
            py::init<double, double, double, double, double, int, double, int>(),
            py::kw_only(),
            py::arg("time_horizon") = 2.0,
            py::arg("step_size") = 0.5,
            py::arg("sigma") = 0.3,
            py::arg("time_uncertainty") = 0.5,
            py::arg("velocity_uncertainty") = 0.2,
            py::arg("num_samples") = 20,
            py::arg("jam_speed_threshold") = 0.1,
            py::arg("jam_step_count") = 10)
        .def("build", &WarpDriverModelBuilder::Build);
    py::class_<WarpDriverModelData>(m, "WarpDriverModelState")
        .def(
            py::init([](double desired_speed, double radius) {
                return WarpDriverModelData{
                    .radius = radius,
                    .v0 = desired_speed,
                    .jamCounter = 0};
            }),
            py::kw_only(),
            py::arg("desired_speed") = 1.2,
            py::arg("radius") = 0.15)
        .def_readwrite("radius", &WarpDriverModelData::radius)
        .def_readwrite("desired_speed", &WarpDriverModelData::v0)
        .def_readonly("jam_counter", &WarpDriverModelData::jamCounter);
}
