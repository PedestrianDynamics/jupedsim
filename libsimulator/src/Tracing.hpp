// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <perfetto.h>

PERFETTO_DEFINE_CATEGORIES(
    perfetto::Category("C++").SetDescription("C++ Traces."),
    perfetto::Category("Python").SetDescription("Python Traces."));

#include <functional>
#include <memory>
#include <string>

#ifndef JPS_TRACE_EVENT
#define JPS_TRACE_EVENT(name) TRACE_EVENT("C++", name);
#define JPS_TRACE_EVENT_BEGIN(name) TRACE_EVENT_BEGIN("C++", name)
#define JPS_TRACE_EVENT_END TRACE_EVENT_END("C++")
#if defined(_MSC_VER)
#define JPS_TRACE_FUNC JPS_TRACE_EVENT(__FUNCTION__)
#else
#define JPS_TRACE_FUNC JPS_TRACE_EVENT(__PRETTY_FUNCTION__)
#endif
#endif

// PorfilerSingleton is a wrapper around perfetto::TracingSession to provide a simple interface
// for the rest of the codebase. It is implemented as a singleton to ensure that there is only
// one instance of the profiler throughout the application. The Timer class also accesses the
// profiler to record traces that aline with the timer entries. This allows for a unified
// tracing and timing system that can be easily accessed and used throughout the codebase.
class Profiler
{
    static Profiler profiler;

public:
    static Profiler& instance() noexcept { return profiler; };

    static void enable();
    static void disable();

    static void dumpAndReset(const std::string& filename);
    inline bool isEnabled() const { return enabled; }

private:
    Profiler() = default;
    Profiler(const Profiler&) = delete;
    Profiler& operator=(const Profiler&) = delete;
    Profiler(Profiler&&) = delete;
    Profiler& operator=(Profiler&&) = delete;

    void createSession();
    void writeAndResetSession(const std::string& filename);
    bool enabled{false};
    std::unique_ptr<perfetto::TracingSession> tracing_session{};
};