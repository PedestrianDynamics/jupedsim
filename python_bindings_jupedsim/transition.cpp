// SPDX-License-Identifier: LGPL-3.0-or-later
#include "route.hpp"
#include "stage.hpp"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h> // IWYU pragma: keep

#include <cstdint>
#include <tuple>
#include <vector>

namespace py = pybind11;

// What follows a stage, by stage id. The public transitions, which also carry their source, are
// defined in Python and passed to the simulation as these.
void init_transition(py::module_& m)
{
    py::class_<FixedTransitionDescription>(m, "_FixedTransitionDescription")
        .def(py::init([](uint64_t next) { return FixedTransitionDescription(next); }));
    py::class_<RoundRobinTransitionDescription>(m, "_RoundRobinTransitionDescription")
        .def(py::init([](const std::vector<std::tuple<uint64_t, uint64_t>>& weighted_stages) {
            std::vector<std::tuple<BaseStage::ID, uint64_t>> weights{};
            for(const auto& [stage_id, weight] : weighted_stages) {
                weights.emplace_back(stage_id, weight);
            }
            return RoundRobinTransitionDescription(weights);
        }));
    py::class_<LeastTargetedTransitionDescription>(m, "_LeastTargetedTransitionDescription")
        .def(py::init([](const std::vector<uint64_t>& candidates) {
            std::vector<BaseStage::ID> ids{};
            for(const auto id : candidates) {
                ids.emplace_back(id);
            }
            return LeastTargetedTransitionDescription(ids);
        }));
}
