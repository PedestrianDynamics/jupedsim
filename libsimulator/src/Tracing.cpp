// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Tracing.hpp"
#include <optional>

namespace cr = std::chrono;

Trace::Trace(uint64_t& _t) : startedAt(cr::high_resolution_clock::now()), t(_t)
{
}

std::optional<Trace> PerfStats::trace(uint64_t& v)
{
    if(enabled) {
        return std::optional<Trace>{std::in_place, v};
    } else {
        return std::nullopt;
    }
}
std::optional<Trace> PerfStats::TraceIterate()
{
    return trace(iterate_duration);
}

std::optional<Trace> PerfStats::TraceOperationalDecisionSystemRun()
{
    return trace(op_dec_system_run_duration);
}
