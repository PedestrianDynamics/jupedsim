// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <cstdint>
#include <limits>

/// Counter-based random numbers for one agent in one iteration.
///
/// The stream depends only on (seed, iteration, agent index), never on the order in which
/// agents are processed, so results are identical for any number of threads. SplitMix64 is used
/// both to derive the stream from the key and to advance it. Satisfies
/// UniformRandomBitGenerator, so it works with the <random> distributions.
class StepRng
{
    std::uint64_t _state;

    static constexpr std::uint64_t mix(std::uint64_t z)
    {
        z = (z ^ (z >> 30U)) * 0xbf58476d1ce4e5b9ULL;
        z = (z ^ (z >> 27U)) * 0x94d049bb133111ebULL;
        return z ^ (z >> 31U);
    }

public:
    using result_type = std::uint64_t;

    StepRng(std::uint64_t seed, std::uint64_t iteration, std::uint64_t agent_index)
        : _state(mix(seed ^ mix(iteration ^ mix(agent_index + 0x9e3779b97f4a7c15ULL))))
    {
    }

    static constexpr result_type min() { return 0; }
    static constexpr result_type max() { return std::numeric_limits<result_type>::max(); }

    result_type operator()()
    {
        _state += 0x9e3779b97f4a7c15ULL;
        return mix(_state);
    }
};
