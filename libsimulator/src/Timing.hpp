// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <chrono>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>

// Helper macro to create a scoped timer probe.
// It creates a scope guard that starts the timer probe when it is created and stops the timer probe
// when it goes out of scope. The log level is used to filter which
// timer probes are active based on the log level set in the Timer object.
// Using it requires "Tracing.hpp". Do not pull it into this header though as it pulls in the huge
// perfetto.h.
#ifndef JPS_SCOPE_CONCAT_IMPL
#define JPS_SCOPE_CONCAT_IMPL(x, y) x##y
#endif
#ifndef JPS_SCOPE_CONCAT
#define JPS_SCOPE_CONCAT(x, y) JPS_SCOPE_CONCAT_IMPL(x, y)
#endif
#ifndef JPS_SCOPED_TIMER
#define JPS_SCOPED_TIMER_AND_TRACE(timer_obj, name, loglevel)                                      \
    auto JPS_SCOPE_CONCAT(_jps_scoped_timer_guard_, __COUNTER__) =                                 \
        (timer_obj).scoped_timer_probe((name), (loglevel));                                        \
    JPS_TRACE_EVENT(name)
#endif

// Helper class to store the start time and duration of a timer entry.
// It also has a flag to indicate whether the timer is currently running or not.
// The duration is summed, hence multiple calls to start and stop will accumulate the duration in
// microseconds.
class TimerEntry
{

public:
    using DurationType = uint64_t;

    TimerEntry() = default;
    ~TimerEntry() = default;
    TimerEntry(const TimerEntry& other) = delete;
    TimerEntry& operator=(const TimerEntry& other) = delete;
    TimerEntry(TimerEntry&& other) noexcept;
    TimerEntry& operator=(TimerEntry&& other) noexcept;
    // Starts the timer entry. If the timer entry is already running, this function does nothing.
    void start();
    // Stops the timer entry. If the timer entry is not running, this function does nothing.
    // Upon stopping the timer entry, the duration of the timer entry is updated with the time
    // elapsed since it was started.
    void stop();
    // Get the duration of the timer entry in microseconds.
    // If the timer is still running, it returns the duration until now.
    // If the timer entry does not exist, it returns 0.
    DurationType get_duration_in_microseconds() const;

private:
    // Last start time of the timer entry.
    std::chrono::high_resolution_clock::time_point _started_at;
    // Duration of the timer entry in microseconds.
    // It is updated with the time elapsed since the last start time when the timer is stopped.
    DurationType _duration_in_microseconds{0};
    // Flag to indicate whether the timer is currently running or not.
    bool _running{false};
};

class Timer
{
    // Log level for the timer. Timer probes with a log level higher than
    // the set log level will not be active and will not record time.
    int _max_log_level{0};
    // Map of timer entry names to their corresponding TimerEntry objects.
    std::unordered_map<std::string, TimerEntry> _timer_map{};

public:
    // Helper class to create a scoped timer probe guard.
    // It starts the timer probe with the given name and log level when
    // it is created and stops the timer probe when it goes out of scope.
    class ScopedTimerProbeGuard
    {
        // Pointer to the Timer object that the guard will operate on.
        // It is set to nullptr if the guard is moved from or if the log level of the timer probe is
        // higher than the log level set in the Timer object, in which case the guard will not be
        // active and will not record time.
        Timer* _stats{nullptr};
        // Name of the timer probe that the guard will operate on.
        std::string _name;

    public:
        ScopedTimerProbeGuard(Timer& stats_ref, std::string_view timer_name, int loglevel)
            : _stats((loglevel <= stats_ref.get_log_level()) ? &stats_ref : nullptr)
            , _name(timer_name)
        {
            if(_stats) {
                _stats->push_timer_probe(_name, loglevel);
            }
        }
        ~ScopedTimerProbeGuard()
        {
            if(_stats) {
                _stats->pop_timer_probe(_name);
            }
        }
        // Delete copy constructor and copy assignment operator to prevent copying of the guard.
        ScopedTimerProbeGuard(const ScopedTimerProbeGuard&) = delete;
        ScopedTimerProbeGuard& operator=(const ScopedTimerProbeGuard&) = delete;
        // Move constructor and move assignment operator to allow moving of the guard.
        ScopedTimerProbeGuard(ScopedTimerProbeGuard&& other) noexcept
            : _stats(other._stats), _name(other._name)
        {
            other._stats = nullptr;
        }
        ScopedTimerProbeGuard& operator=(ScopedTimerProbeGuard&& other) noexcept
        {
            if(this != &other) {
                if(_stats) {
                    _stats->pop_timer_probe(_name);
                }
                _stats = other._stats;
                _name = std::move(other._name);
                other._stats = nullptr;
            }
            return *this;
        }
    };

    ~Timer() = default;
    // Creates a scoped timer probe guard that starts the timer probe with the given name
    // and log level when it is created and stops the timer probe when it goes out of scope.
    [[nodiscard]] inline ScopedTimerProbeGuard
    scoped_timer_probe(const std::string_view name, int loglevel = 0)
    {
        return ScopedTimerProbeGuard(*this, name, loglevel);
    }
    // Starts a timer probe with the given name.
    // If the log level of the timer probe is higher than the log level set in the Timer object,
    // the timer probe will not be active and will not record time.
    // If a timer probe with the same name already exists, that timer probe will be restarted.
    // If tracing is enabled a trace with the same name will also be pushed to the ProfilerSingleton
    // instance.
    void push_timer_probe(std::string_view name, int loglevel = 0);
    // Stops the timer probe with the given name.
    // If the timer probe does not exist, this function does nothing.
    // Upon stopping the timer probe, the duration of the timer entry is updated with the time
    // elapsed since it was started.
    // If tracing is enabled a trace with the same name will also be poped from the
    // ProfilerSingleton instance.
    void pop_timer_probe(const std::string_view name);
    // Returns the duration of the timer entry in microseconds. If the timer is still running, it
    // returns the duration until now. if the timer entry does not exist, it returns 0.
    TimerEntry::DurationType get_duration(const std::string_view name) const;
    // Returns a map of timer entry names to their durations in microseconds.
    // If a timer is still running, it returns the duration until now.
    // If a timer entry does not exist, it is not included in the map.
    // PST: I choose a map here so that we always have the same order of entries when printing them.
    std::map<std::string, TimerEntry::DurationType> get_durations() const;
    // Sets the log level for the timer. Timer probes with a log level higher than the set log
    // level will not be active and will not record time.
    void set_log_level(int level) { _max_log_level = level; };
    // Returns the current log level of the timer.
    int get_log_level() const { return _max_log_level; };
};
