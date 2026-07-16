// SPDX-License-Identifier: LGPL-3.0-or-later
#include "NeighborQuery.hpp"

#include "conversion.hpp"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <tuple>

namespace py = pybind11;

void init_neighbor_query(py::module_& m)
{
    py::class_<NeighborQuery>(m, "NeighborQuery")
        .def(
            "get_neighboring_agents",
            [](const NeighborQuery& self, std::tuple<double, double> pos, double radius) {
                return self(intoPoint(pos), radius);
            },
            py::arg("pos"),
            py::arg("radius"),
            "Get the model states of agents within a certain radius of a position, excluding "
            "the agent this query is bound to.");
}
