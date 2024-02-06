// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "conversion.hpp"
#include "wrapper.hpp"

#include <jupedsim/jupedsim.h>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

void init_geometry(py::module_& m)
{

    py::class_<JPS_Geometry_Wrapper>(m, "Geometry")
        .def(
            "boundary",
            [](const JPS_Geometry_Wrapper& w) {
                const auto len = JPS_Geometry_GetBoundarySize(w.handle);
                const auto data = JPS_Geometry_GetBoundaryData(w.handle);
                return intoTuple(data, data + len);
            })
        .def("holes", [](const JPS_Geometry_Wrapper& w) {
            const auto holeCount = JPS_Geometry_GetHoleCount(w.handle);
            std::vector<std::vector<std::tuple<double, double>>> res{};
            res.reserve(holeCount);
            for(size_t index = 0; index < holeCount; ++index) {
                const auto len = JPS_Geometry_GetHoleSize(w.handle, index, nullptr);
                const auto data = JPS_Geometry_GetHoleData(w.handle, index, nullptr);
                res.emplace_back(intoTuple(data, data + len));
            }
            return res;
        });
    py::class_<JPS_GeometryBuilder_Wrapper>(m, "GeometryBuilder")
        .def(py::init([]() {
            return std::make_unique<JPS_GeometryBuilder_Wrapper>(JPS_GeometryBuilder_Create());
        }))
        .def(
            "add_accessible_area",
            [](const JPS_GeometryBuilder_Wrapper& w,
               std::vector<std::tuple<double, double>> polygon) {
                const auto pts = intoJPS_Point(polygon);
                JPS_GeometryBuilder_AddAccessibleArea(w.handle, pts.data(), pts.size());
            },
            "Add area where agents can move")
        .def(
            "exclude_from_accessible_area",
            [](const JPS_GeometryBuilder_Wrapper& w,
               std::vector<std::tuple<double, double>> polygon) {
                const auto pts = intoJPS_Point(polygon);
                JPS_GeometryBuilder_ExcludeFromAccessibleArea(w.handle, pts.data(), pts.size());
            },
            "Add areas where agents can not move (obstacles)")
        .def(
            "build",
            [](const JPS_GeometryBuilder_Wrapper& w) {
                JPS_ErrorMessage errorMsg{};
                auto result = JPS_GeometryBuilder_Build(w.handle, &errorMsg);
                if(result) {
                    return std::make_unique<JPS_Geometry_Wrapper>(result);
                }
                auto msg = std::string(JPS_ErrorMessage_GetMessage(errorMsg));
                JPS_ErrorMessage_Free(errorMsg);
                throw std::runtime_error{msg};
            },
            "Geometry builder");
}
