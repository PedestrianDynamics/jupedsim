// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "operational_model_type.hpp"
#include "simulation_error.hpp"

#include <boost/random/splitmix64.hpp>
#include <boost/random/xoshiro.hpp>

#include <array>
#include <cstdint>
#include <optional>

/// Random number stream owned by one agent and kept in its model state. An agent's draws
/// depend only on its own stream, never on the order agents are stepped in.
using AgentRng = boost::random::xoshiro128pp;

/// The stream as a model state holds it: empty until the model seeded the state. A state that
/// was never seeded would otherwise draw the same numbers as every other unseeded one.
using AgentRngSlot = std::optional<AgentRng>;

/// The stream in `slot`, which `model` is about to draw from. Throws if the state was never
/// seeded, which happens when a custom model delegates to `model` without forwarding the seeding.
inline AgentRng& seeded_stream(AgentRngSlot& slot, OperationalModelType model)
{
    if(!slot) {
        throw SimulationError(
            "{} was handed a state whose random number stream was never seeded. A custom model "
            "delegating to it has to seed the state with OperationalModel.seed_state(), "
            "e.g. from CustomOperationalModel.seed_agent_state().",
            to_string(model));
    }
    return *slot;
}

/// Hands out the streams of the agents added to a simulation, one after the other, drawn from a
/// generator seeded once with the model's seed. The same seed and the same order of additions
/// give every agent the same stream.
class AgentRngSeeder
{
    // splitmix64 is what xoshiro's authors recommend to seed it with. Its raw 64 bit draws are
    // used as they are: a std distribution on top would differ between standard libraries.
    boost::random::splitmix64 _generator;

public:
    explicit AgentRngSeeder(std::uint64_t seed) : _generator(seed) {}

    /// The stream of the next agent.
    AgentRng next()
    {
        // Not AgentRng's single value constructor: it takes 32 bits and would truncate the draws.
        const std::uint64_t first = _generator();
        const std::uint64_t second = _generator();
        std::array<std::uint32_t, 4> words{
            static_cast<std::uint32_t>(first >> 32U),
            static_cast<std::uint32_t>(first),
            static_cast<std::uint32_t>(second >> 32U),
            static_cast<std::uint32_t>(second)};
        auto begin = words.begin();
        return AgentRng{begin, words.end()};
    }
};
