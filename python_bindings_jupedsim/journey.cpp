// SPDX-License-Identifier: LGPL-3.0-or-later
#include "conversion.hpp"
#include <Journey.hpp>
#include <Stage.hpp>

#include <map>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

using JourneyDesc = std::map<uint64_t, TransitionDescription>;

void init_journey(py::module_& m)
{
    py::class_<JourneyDesc>(m, "JourneyDescription")
        .def(py::init([]() { return JourneyDesc{}; }))
        .def(py::init([](const std::vector<uint64_t>& ids) {
            auto desc = JourneyDesc{};
            for(auto id : ids) {
                desc[id] = NonTransitionDescription{};
            }
            return desc;
        }))
        .def(
            "add",
            [](JourneyDesc& desc, uint64_t id) {
                desc.insert(std::make_pair(id, NonTransitionDescription{}));
            })
        .def(
            "add",
            [](JourneyDesc& desc, const std::vector<uint64_t>& ids) {
                for(const auto& id : ids) {
                    desc.insert(std::make_pair(id, NonTransitionDescription{}));
                }
            })
        .def(
            "set_transition_for_stage",
            [](JourneyDesc& desc, uint64_t stageId, TransitionDescription& transition) {
                auto iter = desc.find(stageId);
                if(iter == std::end(desc)) {
                    throw std::runtime_error(fmt::format(
                        "Could not set transition for given stage id {}. Stage not found.",
                        stageId));
                }
                iter->second = transition;
            });
}
