// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <chrono>
#include <optional>

class Trace
{
    std::chrono::high_resolution_clock::time_point startedAt;
    uint64_t& t;

public:
    Trace(uint64_t& _t);
    ~Trace()
    {
        const auto now = std::chrono::high_resolution_clock::now();
        t = std::chrono::duration_cast<std::chrono::microseconds>(now - startedAt).count();
    }
    Trace(const Trace& other) = delete;
    Trace& operator=(const Trace& other) = delete;
    Trace(Trace&& other) = delete;
    Trace& operator=(const Trace&& other) = delete;
};

class PerfStats
{
    uint64_t iterate_duration{};
    uint64_t op_dec_system_run_duration{};
    bool enabled{false};

public:
    std::optional<Trace> TraceIterate();
    std::optional<Trace> TraceOperationalDecisionSystemRun();
    void SetEnabled(bool status) { enabled = status; };
    uint64_t IterationDuration() const { return iterate_duration; };
    uint64_t OpDecSystemRunDuration() const { return op_dec_system_run_duration; };

private:
    std::optional<Trace> trace(uint64_t& v);
};
