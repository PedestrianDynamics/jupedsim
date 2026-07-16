// SPDX-License-Identifier: LGPL-3.0-or-later
#include "NeighborQuery.hpp"
#include "OperationalModels/CustomModel/CustomModel.hpp"
#include "conversion.hpp"
#include "python_model.hpp"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <tuple>
#include <variant>

namespace py = pybind11;

void init_neighbor_query(py::module_& m)
{
    py::class_<NeighborQuery>(m, "NeighborQuery")
        .def(
            "get_neighboring_agents",
            [](const NeighborQuery& self, std::tuple<double, double> pos, double radius) {
                const auto states = self(intoPoint(pos), radius);
                py::list result;
                for(const auto& state : states) {
                    // Custom-model simulations get the user's own state objects back.
                    if(const auto* custom = std::get_if<CustomModel::State>(&state)) {
                        result.append(StateObject(*custom));
                    } else {
                        result.append(py::cast(state));
                    }
                }
                return result;
            },
            py::arg("pos"),
            py::arg("radius"),
            "Get the model states of agents within a certain radius of a position, excluding "
            "the agent this query is bound to.");
}
