// SPDX-License-Identifier: LGPL-3.0-or-later
#include "rule.hpp"

#include "conversion.hpp"
#include "route.hpp"
#include "stage.hpp"
#include "type_casters.hpp" // IWYU pragma: keep

#include <pybind11/pybind11.h>
#include <pybind11/stl.h> // IWYU pragma: keep

#include <cstdint>
#include <tuple>
#include <vector>

namespace py = pybind11;

void init_rule(py::module_& m)
{
    py::class_<PyRule> rule(m, "Rule");
    rule.doc() = clean_doc(R"(
        Describes the rule at a stage.

        This type describes how a agent will proceed after completing its stage.
        This effectively describes the set of outbound edges for a stage.

        There are 6 types of rules currently available:

        * **Fixed rule:** On completion of this rule's stage all agents
          will proceed to the specified next stage.

        * **Round robin rule:** On completion of this rule's stage agents
          will proceed by smooth weighted round-robin. With the weights 3 and 2 the
          agents deciding take the first, second, first, second, first stage, and
          so on. Only the ratio of the weights matters; ties go to the earlier stage.

        * **Least targeted rule:** On completion of this stage agents will
          proceed towards the currently least targeted amongst the specified choices.
          The number of "targeting" agents is the amount of agents currently moving
          towards this stage. This includes agents from different routes.

        * **Least travel time rule:** On completion of this stage each agent
          will proceed towards the choice with the least travel time from where it
          stands.

        * **Stay rule:** Agents completing the stage have arrived and stay there.

        * **None rule:** Leads nowhere; exits use it.
    )");
    rule.def_static(
            "create_fixed_rule",
            [](uint64_t stage_id) { return PyRule{FixedRuleDescription(stage_id)}; },
            py::arg("stage_id"),
            clean_doc(R"(
            Create a fixed rule.

            On completion of this rule's stage all agents will proceed to the
            specified next stage.

            Args:
                stage_id: Id of the stage to move to next.

            Returns:
                The rule.

            Raises:
                SimulationError: If ``stage_id`` is not a valid stage id.
            )")
                .c_str())
        .def_static(
            "create_round_robin_rule",
            [](const std::vector<std::tuple<uint64_t, uint64_t>>& stage_weights) {
                auto weights = std::vector<std::tuple<BaseStage::ID, uint64_t>>{};
                weights.reserve(stage_weights.size());
                for(const auto& [stage_id, weight] : stage_weights) {
                    weights.emplace_back(stage_id, weight);
                }
                return PyRule{RoundRobinDescription(weights)};
            },
            py::arg("stage_weights"),
            clean_doc(R"(
            Create a round-robin rule.

            On completion of this rule's stage agents will proceed by smooth
            weighted round-robin: each decision adds every stage's weight to its
            current weight, takes the stage with the largest current weight (ties:
            the earlier one) and takes the sum of the weights off it. With the
            weights 3 and 2 agents take the first, second, first, second, first
            stage, and so on. Stages of weight 0 are never taken.

            Args:
                stage_weights: ``(stage_id, weight)`` pairs, one per outgoing stage;
                    at least one weight > 0.

            Returns:
                The rule.

            Raises:
                SimulationError: If a stage id is not a valid stage id.
            )")
                .c_str())
        .def_static(
            "create_least_targeted_rule",
            [](const std::vector<uint64_t>& stages) {
                return PyRule{LeastTargetedDescription(into_vec_t<BaseStage::ID>(stages))};
            },
            py::arg("stage_ids"),
            clean_doc(R"(
            Create a least targeted rule.

            On completion of this stage agents will proceed towards the currently
            least targeted amongst the specified choices. The number of "targeting"
            agents is the amount of agents currently moving towards this stage.
            This includes agents from different routes.

            Args:
                stage_ids: Ids of the stages to choose the next target from.

            Returns:
                The rule.

            Raises:
                SimulationError: If a stage id is not a valid stage id.
            )")
                .c_str())
        .def_static(
            "create_least_travel_time_rule",
            [](const std::vector<uint64_t>& stages) {
                return PyRule{LeastTravelTimeDescription(into_vec_t<BaseStage::ID>(stages))};
            },
            py::arg("stage_ids"),
            clean_doc(R"(
            Create a least travel time rule.

            On completion of this stage each agent proceeds towards the specified
            choice with the least travel time from its position, read from the
            floor fields. Ties go to the earlier choice.

            Args:
                stage_ids: Ids of the stages to choose the next target from.

            Returns:
                The rule.

            Raises:
                SimulationError: If a stage id is not a valid stage id.
            )")
                .c_str())
        .def_static(
            "create_stay_rule",
            [] { return PyRule{StayDescription{}}; },
            clean_doc(R"(
            Create a rule that keeps agents at the stage.

            Agents completing the stage have arrived: they keep the stage as their
            target and stay there.

            Returns:
                The rule.
            )")
                .c_str())
        .def_static(
            "create_none_rule",
            [] { return PyRule{NoRuleDescription{}}; },
            clean_doc(R"(
            Create a rule that leads nowhere, for exits.

            The next stage of a stage with this rule is the stage itself.
            Exits use it: agents completing an exit leave the simulation.

            Returns:
                The rule.
            )")
                .c_str());
}
