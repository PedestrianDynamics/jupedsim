// SPDX-License-Identifier: LGPL-3.0-or-later
#include "CollisionFreeSpeedModelV2.hpp"
#include "CollisionFreeSpeedModelV2Builder.hpp"
#include "OperationalModel.hpp"
#include "conversion.hpp"
#include "type_casters.hpp" // IWYU pragma: keep

#include <pybind11/cast.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h> // IWYU pragma: keep

namespace py = pybind11;

void init_collision_free_speed_model_v2(py::module_& m)
{
    py::class_<CollisionFreeSpeedModelV2, OperationalModel, py::smart_holder>(
        m, "CollisionFreeSpeedModelV2");
    py::class_<CollisionFreeSpeedModelV2Builder>(m, "CollisionFreeSpeedModelV2Builder")
        .def(py::init<>())
        .def("build", &CollisionFreeSpeedModelV2Builder::Build);

    py::class_<CollisionFreeSpeedModelV2::State>(m, "CollisionFreeSpeedModelV2State")
        .def_static("_defaults", []() { return CollisionFreeSpeedModelV2::State{}; })
        .def(
            py::init([](std::tuple<double, double> orientation,
                        double strengthNeighborRepulsion,
                        double rangeNeighborRepulsion,
                        double strengthGeometryRepulsion,
                        double rangeGeometryRepulsion,
                        double timeGap,
                        double desiredSpeed,
                        double radius) {
                return CollisionFreeSpeedModelV2::State{
                    .orientation = intoPoint(orientation),
                    .strengthNeighborRepulsion = strengthNeighborRepulsion,
                    .rangeNeighborRepulsion = rangeNeighborRepulsion,
                    .strengthGeometryRepulsion = strengthGeometryRepulsion,
                    .rangeGeometryRepulsion = rangeGeometryRepulsion,
                    .timeGap = timeGap,
                    .v0 = desiredSpeed,
                    .radius = radius};
            }),
            py::kw_only(),
            py::arg("orientation"),
            py::arg("strength_neighbor_repulsion"),
            py::arg("range_neighbor_repulsion"),
            py::arg("strength_geometry_repulsion"),
            py::arg("range_geometry_repulsion"),
            py::arg("time_gap"),
            py::arg("desired_speed"),
            py::arg("radius"))
        .def_readwrite("orientation", &CollisionFreeSpeedModelV2::State::orientation)
        .def_readwrite(
            "strength_neighbor_repulsion",
            &CollisionFreeSpeedModelV2::State::strengthNeighborRepulsion)
        .def_readwrite(
            "range_neighbor_repulsion", &CollisionFreeSpeedModelV2::State::rangeNeighborRepulsion)
        .def_readwrite(
            "strength_geometry_repulsion",
            &CollisionFreeSpeedModelV2::State::strengthGeometryRepulsion)
        .def_readwrite(
            "range_geometry_repulsion", &CollisionFreeSpeedModelV2::State::rangeGeometryRepulsion)
        .def_readwrite("time_gap", &CollisionFreeSpeedModelV2::State::timeGap)
        .def_readwrite("desired_speed", &CollisionFreeSpeedModelV2::State::v0)
        .def_readwrite("radius", &CollisionFreeSpeedModelV2::State::radius);
}
