// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <perfetto.h>

PERFETTO_DEFINE_CATEGORIES(perfetto::Category("main").SetDescription("Main iteration over agents"));

#include <functional>
#include <memory>
#include <string>

#ifndef JPS_SCOPE_CONCAT_IMPL
#define JPS_SCOPE_CONCAT_IMPL(x, y) x##y
#endif
#ifndef JPS_SCOPE_CONCAT
#define JPS_SCOPE_CONCAT(x, y) JPS_SCOPE_CONCAT_IMPL(x, y)
#endif
#ifndef JPS_SCOPED_PROBE
#define JPS_SCOPED_PROBE(profiler_obj, name)                                                       \
    auto JPS_SCOPE_CONCAT(_jps_scoped_probe_guard_, __COUNTER__) =                                 \
        (profiler_obj).scopedProbe((name))
#endif

// PorfilerSingleton is a wrapper around perfetto::TracingSession to provide a simple interface for
// the rest of the codebase. It is implemented as a singleton to ensure that there is only one
// instance of the profiler throughout the application. The Timer class also accesses the profiler
// to record traces that aline with the timer entries.
// This allows for a unified tracing and timing system that can be easily accessed and used
// throughout the codebase.
class ProfilerSingleton
{
    static ProfilerSingleton profiler;

public:
    static ProfilerSingleton& instance() noexcept { return profiler; };
    class ScopedProbeGuard
    {
        ProfilerSingleton& profiler = ProfilerSingleton::instance();

    public:
        ScopedProbeGuard() = default;

        ScopedProbeGuard(const std::string_view name) { profiler.pushProbe(name); }
        ~ScopedProbeGuard() { profiler.popProbe(); }
        ScopedProbeGuard(const ScopedProbeGuard&) = delete;
        ScopedProbeGuard& operator=(const ScopedProbeGuard&) = delete;
        ScopedProbeGuard(ScopedProbeGuard&& other) noexcept = delete;
        ScopedProbeGuard& operator=(ScopedProbeGuard&& other) = delete;
    };

    void enable();
    void disable();
    inline void pushProbe(const std::string_view name)
    {
        if(enabled) {
            TRACE_EVENT_BEGIN("main", perfetto::DynamicString{name.data()});
        }
    }
    inline void popProbe()
    {
        if(enabled) {
            TRACE_EVENT_END("main");
        }
    }
    [[nodiscard]] inline ScopedProbeGuard scopedProbe(const std::string_view name)
    {
        return ScopedProbeGuard(name);
    }

    void dumpAndReset(const std::string& filename);
    inline bool isEnabled() const { return enabled; }

private:
    ProfilerSingleton() = default;
    ProfilerSingleton(const ProfilerSingleton&) = delete;
    ProfilerSingleton& operator=(const ProfilerSingleton&) = delete;

    void createSession();
    void writeAndResetSession(const std::string& filename);
    bool enabled{false};
    std::unique_ptr<perfetto::TracingSession> tracing_session{};
};