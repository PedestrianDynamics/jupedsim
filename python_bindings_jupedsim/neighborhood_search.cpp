// SPDX-License-Identifier: LGPL-3.0-or-later
#include "GenericAgent.hpp"
#include "NeighborhoodSearch.hpp"
#include "conversion.hpp"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

void init_neighborhood_search(py::module_& m)
{
    py::class_<NeighborhoodSearch<GenericAgent>>(m, "NeighborhoodSearch")
        .def(
            "get_neighboring_agents",
            [](NeighborhoodSearch<GenericAgent>& self,
               const std::tuple<double, double>& pos,
               double radius) { return self.GetNeighboringAgents(intoPoint(pos), radius); },
            py::arg("pos"),
            py::arg("radius"),
            "Get agents within a certain radius of a position (by value)");
}
