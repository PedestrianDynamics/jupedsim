// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Timing.hpp"

#include <fmt/core.h>

#include <chrono>
#include <cstdint>
#include <functional>
#include <utility>

namespace cr = std::chrono;

TimerEntry::TimerEntry(TimerEntry&& other) noexcept
    : _started_at(std::move(other._started_at))
    , _duration_in_microseconds(other._duration_in_microseconds)
    , _running(other._running)
{
    other._duration_in_microseconds = 0;
    other._running = false;
}

TimerEntry& TimerEntry::operator=(TimerEntry&& other) noexcept
{
    if(this != &other) {
        _started_at = std::move(other._started_at);
        _duration_in_microseconds = other._duration_in_microseconds;
        _running = other._running;
        other._duration_in_microseconds = 0;
        other._running = false;
    }
    return *this;
}

void TimerEntry::start()
{
    if(!_running) {
        _running = true;
        _started_at = cr::high_resolution_clock::now();
    }
}

void TimerEntry::stop()
{
    if(_running) {
        _running = false;
        _duration_in_microseconds += std::chrono::duration_cast<std::chrono::microseconds>(
                                         std::chrono::high_resolution_clock::now() - _started_at)
                                         .count();
    }
}

uint64_t TimerEntry::get_duration_in_microseconds() const
{
    if(_running) {
        return _duration_in_microseconds +
               std::chrono::duration_cast<std::chrono::microseconds>(
                   std::chrono::high_resolution_clock::now() - _started_at)
                   .count();
    }
    return _duration_in_microseconds;
}

void Timer::push_timer_probe(std::string_view name, int timer_probe_level)
{
    if(timer_probe_level > _max_log_level) {
        return;
    }
    std::string name_str(name);
    auto iter = _timer_map.find(name_str);
    // use emplace to avoid multiple lookups and unnecessary default construction of TimerEntry
    if(iter == _timer_map.end()) {
        _timer_map.emplace(name_str, TimerEntry()).first->second.start();
    } else {
        iter->second.start();
    }
}

void Timer::pop_timer_probe(const std::string_view name)
{
    // use auto iter = timer_map.find(name) to avoid multiple lookups
    auto iter = _timer_map.find(std::string(name));
    if(iter != _timer_map.end()) {
        iter->second.stop();
    }
}

TimerEntry::DurationType Timer::get_duration(const std::string_view name) const
{
    auto iter = _timer_map.find(std::string(name));
    if(iter != _timer_map.end()) {
        return iter->second.get_duration_in_microseconds();
    }
    return 0;
}

std::map<std::string, TimerEntry::DurationType> Timer::get_durations() const
{
    std::map<std::string, TimerEntry::DurationType> entries;
    for(const auto& [name, trace] : _timer_map) {
        entries.emplace(name, trace.get_duration_in_microseconds());
    }
    return entries;
}