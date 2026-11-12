// SPDX-License-Identifier: LGPL-3.0-or-later
#include "thread_pool.hpp"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <utility>

#if defined(__linux__)
#include <pthread.h>
#include <sched.h>
#endif

#if defined(__x86_64__) || defined(_M_X64)
#include <immintrin.h>
#endif

namespace
{
void cpu_relax()
{
#if defined(__x86_64__) || defined(_M_X64)
    _mm_pause();
#else
    std::this_thread::yield();
#endif
}

/// How long a thread spins for the next loop before it sleeps. SPIKE: JPS_POOL_SPIN_US.
std::chrono::microseconds spin_time()
{
    static const std::chrono::microseconds value = [] {
        const char* env = std::getenv("JPS_POOL_SPIN_US");
        return std::chrono::microseconds{env != nullptr ? std::strtol(env, nullptr, 10) : 200};
    }();
    return value;
}

template <typename Done>
bool spin_until(Done&& done)
{
    const auto deadline = std::chrono::steady_clock::now() + spin_time();
    while(!done()) {
        for(int i = 0; i < 64; ++i) {
            cpu_relax();
        }
        if(std::chrono::steady_clock::now() > deadline) {
            return false;
        }
    }
    return true;
}
} // namespace

ThreadPool::ThreadPool(std::size_t threads)
{
    const std::size_t extra = threads > 1 ? threads - 1 : 0;
    _workers.reserve(extra);
    for(std::size_t i = 0; i < extra; ++i) {
        _workers.emplace_back([this] { worker_loop(); });
    }
#if defined(__linux__)
    // SPIKE: JPS_PIN=1 pins thread k (caller = 0) to CPU k * JPS_PIN_STRIDE (default 1).
    if(const char* pin = std::getenv("JPS_PIN"); pin != nullptr && pin[0] == '1') {
        const char* stride_env = std::getenv("JPS_PIN_STRIDE");
        const long stride = stride_env != nullptr ? std::strtol(stride_env, nullptr, 10) : 1;
        const auto pin_to = [](pthread_t thread, long cpu) {
            cpu_set_t set;
            CPU_ZERO(&set);
            CPU_SET(static_cast<int>(cpu), &set);
            pthread_setaffinity_np(thread, sizeof(set), &set);
        };
        pin_to(pthread_self(), 0);
        for(std::size_t i = 0; i < _workers.size(); ++i) {
            pin_to(_workers[i].native_handle(), static_cast<long>(i + 1) * stride);
        }
    }
#endif
}

ThreadPool::~ThreadPool()
{
    if(std::getenv("JPS_POOL_STATS") != nullptr && stat_loops > 0) {
        const double busy = static_cast<double>(stat_busy_ns.load()) / 1e9;
        const double wall = static_cast<double>(stat_wall_ns) / 1e9;
        std::fprintf(
            stderr,
            "POOL threads=%zu loops=%llu items=%llu wall=%.3fs busy=%.3fs utilization=%.1f%% "
            "busy/item=%.2fus\n",
            size(),
            static_cast<unsigned long long>(stat_loops),
            static_cast<unsigned long long>(stat_items),
            wall,
            busy,
            100.0 * busy / (wall * static_cast<double>(size())),
            1e6 * busy / static_cast<double>(stat_items));
    }
    {
        std::lock_guard lock{_mutex};
        _stop.store(true);
    }
    _start.notify_all();
    for(auto& worker : _workers) {
        worker.join();
    }
}

void ThreadPool::run(std::function<void(std::size_t)> body, std::size_t n, std::size_t chunk)
{
    const auto t0 = std::chrono::steady_clock::now();
    _body = std::move(body);
    _n = n;
    _chunk = std::max<std::size_t>(chunk, 1);
    _next.store(0, std::memory_order_relaxed);
    _failed.store(false, std::memory_order_relaxed);
    _error = nullptr;
    _running.store(_workers.size(), std::memory_order_relaxed);
    {
        // Under the lock, so a worker between its predicate check and its wait cannot miss it.
        std::lock_guard lock{_mutex};
        _generation.fetch_add(1, std::memory_order_release);
    }
    _start.notify_all();
    work();
    const auto finished = [this] { return _running.load(std::memory_order_acquire) == 0; };
    if(!spin_until(finished)) {
        std::unique_lock lock{_mutex};
        _done.wait(lock, finished);
    }
    _body = nullptr;
    stat_wall_ns +=
        std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - t0)
            .count();
    ++stat_loops;
    stat_items += n;
    if(_error) {
        std::rethrow_exception(std::exchange(_error, nullptr));
    }
}

void ThreadPool::work()
{
    const auto t0 = std::chrono::steady_clock::now();
    while(!_failed.load(std::memory_order_relaxed)) {
        const std::size_t begin = _next.fetch_add(_chunk, std::memory_order_relaxed);
        if(begin >= _n) {
            break;
        }
        const std::size_t end = std::min(begin + _chunk, _n);
        for(std::size_t i = begin; i < end; ++i) {
            try {
                _body(i);
            } catch(...) {
                std::lock_guard lock{_error_mutex};
                if(!_error || i < _error_index) {
                    _error = std::current_exception();
                    _error_index = i;
                }
                _failed.store(true, std::memory_order_relaxed);
                break;
            }
        }
    }
    stat_busy_ns +=
        std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - t0)
            .count();
}

void ThreadPool::worker_loop()
{
    std::size_t seen = 0;
    while(true) {
        const auto ready = [&] {
            return _stop.load(std::memory_order_acquire) ||
                   _generation.load(std::memory_order_acquire) != seen;
        };
        if(!spin_until(ready)) {
            std::unique_lock lock{_mutex};
            _start.wait(lock, ready);
        }
        if(_stop.load(std::memory_order_acquire)) {
            return;
        }
        seen = _generation.load(std::memory_order_acquire);
        work();
        if(_running.fetch_sub(1, std::memory_order_acq_rel) == 1) {
            std::lock_guard lock{_mutex};
            _done.notify_one();
        }
    }
}
