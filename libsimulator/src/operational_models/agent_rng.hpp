// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <boost/random/splitmix64.hpp>
#include <boost/random/xoshiro.hpp>

#include <array>
#include <cstdint>

/// Random number stream owned by one agent and kept in its model state. An agent's draws
/// depend only on its own stream, never on the order agents are stepped in.
using AgentRng = boost::random::xoshiro128pp;

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
