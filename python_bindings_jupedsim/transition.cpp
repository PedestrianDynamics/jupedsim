// SPDX-License-Identifier: LGPL-3.0-or-later
#include "transition.hpp"

#include "Journey.hpp"
#include "Stage.hpp"
#include "conversion.hpp"
#include "type_casters.hpp" // IWYU pragma: keep

#include <pybind11/pybind11.h>
#include <pybind11/stl.h> // IWYU pragma: keep

#include <cstdint>
#include <tuple>
#include <vector>

namespace py = pybind11;

void init_transition(py::module_& m)
{
    py::class_<PyTransition> transition(m, "Transition");
    transition.doc() = cleanDoc(R"(
        Describes the Transition at a stage.

        This type describes how a agent will proceed after completing its stage.
        This effectively describes the set of outbound edges for a stage.

        There are 3 types of transitions currently available:

        * **Fixed transitions:** On completion of this transitions stage all agents
          will proceed to the specified next stage.

        * **Round robin transitions:** On completion of this transitions stage agents
          will proceed in a weighted round-robin manner. A round-robin transitions
          with 3 outgoing stages and the weights 5, 7, 11 the first 5 agents to make
          a choice will take the first stage, the next 7 the second stage and the
          next 11 the third stage. Next 5 will take the first stage, and so on...

        * **Least targeted transition:** On completion of this stage agents will
          proceed towards the currently least targeted amongst the specified choices.
          The number of "targeting" agents is the amount of agents currently moving
          towards this stage. This includes agents from different journeys.
    )");
    transition
        .def_static(
            "create_fixed_transition",
            [](uint64_t stageId) { return PyTransition{FixedTransitionDescription(stageId)}; },
            py::arg("stage_id"),
            cleanDoc(R"(
            Create a fixed transition.

            On completion of this transitions stage all agents will proceed to the
            specified next stage.

            Args:
                stage_id: Id of the stage to move to next.

            Returns:
                The transition.

            Raises:
                SimulationError: If ``stage_id`` is not a valid stage id.
            )")
                .c_str())
        .def_static(
            "create_round_robin_transition",
            [](const std::vector<std::tuple<uint64_t, uint64_t>>& stageWeights) {
                auto weights = std::vector<std::tuple<BaseStage::ID, uint64_t>>{};
                weights.reserve(stageWeights.size());
                for(const auto& [stage_id, weight] : stageWeights) {
                    weights.emplace_back(stage_id, weight);
                }
                return PyTransition{RoundRobinTransitionDescription(weights)};
            },
            py::arg("stage_weights"),
            cleanDoc(R"(
            Create a round-robin transition.

            Round-robin transitions: On completion of this transitions stage agents
            will proceed in a weighted round-robin manner. A round-robin
            transitions with 3 outgoing stages and the weights 5, 7, 11 the first 5
            agents to make a choice will take the first stage, the next 7 the
            second stage and the next 11 the third stage. Next 5 will take the
            first stage, and so on...

            Args:
                stage_weights: ``(stage_id, weight)`` pairs, one per outgoing stage.

            Returns:
                The transition.

            Raises:
                SimulationError: If a stage id is not a valid stage id.
            )")
                .c_str())
        .def_static(
            "create_least_targeted_transition",
            [](const std::vector<uint64_t>& stages) {
                return PyTransition{
                    LeastTargetedTransitionDescription(intoVecT<BaseStage::ID>(stages))};
            },
            py::arg("stage_ids"),
            cleanDoc(R"(
            Create a least targeted transition.

            On completion of this stage agents will proceed towards the currently
            least targeted amongst the specified choices. The number of "targeting"
            agents is the amount of agents currently moving towards this stage.
            This includes agents from different journeys.

            Args:
                stage_ids: Ids of the stages to choose the next target from.

            Returns:
                The transition.

            Raises:
                SimulationError: If a stage id is not a valid stage id.
            )")
                .c_str())
        .def_static(
            "create_none_transition",
            [] { return PyTransition{NonTransitionDescription{}}; },
            cleanDoc(R"(
            Create a transition that leads nowhere.

            The next stage of a stage with this transition is the stage itself:
            agents completing it keep it as their target and stay there. It is the
            default for every stage of a :class:`~jupedsim.JourneyDescription`.

            Returns:
                The transition.
            )")
                .c_str());
}
