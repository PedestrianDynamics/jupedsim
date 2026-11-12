// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <functional>
#include <mutex>
#include <thread>
#include <vector>

/// Persistent worker pool for data-parallel loops. The calling thread takes part in every loop,
/// so a pool of size N runs N-1 extra threads. Indices are handed out in chunks from a shared
/// counter, which balances agents whose cost differs widely (e.g. near walls). Between loops
/// workers spin briefly before sleeping, because a simulation starts one loop per iteration and
/// waking sleeping threads costs more than a short spin.
class ThreadPool
{
public:
    /// @param threads total number of threads including the caller; at least 1
    explicit ThreadPool(std::size_t threads);
    ~ThreadPool();
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    ThreadPool(ThreadPool&&) = delete;
    ThreadPool& operator=(ThreadPool&&) = delete;

    std::size_t size() const { return _workers.size() + 1; }

    /// Calls fn(i) for every i in [0, n) and blocks until all calls returned. If calls throw,
    /// the exception of the lowest throwing index is rethrown, the same one a serial loop would
    /// have stopped at; once a call threw, chunks not yet started are skipped.
    template <typename Fn>
    void parallel_for(std::size_t n, std::size_t chunk, Fn&& fn)
    {
        if(n == 0) {
            return;
        }
        if(_workers.empty() || n <= chunk) {
            for(std::size_t i = 0; i < n; ++i) {
                fn(i);
            }
            return;
        }
        run([&fn](std::size_t i) { fn(i); }, n, chunk);
    }

private:
    void run(std::function<void(std::size_t)> body, std::size_t n, std::size_t chunk);
    void work();
    void worker_loop();

    std::vector<std::thread> _workers{};
    std::mutex _mutex{};
    std::condition_variable _start{};
    std::condition_variable _done{};
    alignas(64) std::atomic<std::size_t> _generation{0};
    alignas(64) std::atomic<std::size_t> _running{0};
    alignas(64) std::atomic<bool> _stop{false};

    // The current loop; written before _generation is bumped (release), read after (acquire).
    alignas(64) std::function<void(std::size_t)> _body{};
    std::size_t _n{0};
    std::size_t _chunk{1};
    // Each hot atomic on its own cache line: every thread bumps _next per chunk and reads
    // _failed per chunk; sharing a line with each other or the read-only loop fields above
    // turns every chunk into a cache-line transfer for all threads.
    alignas(64) std::atomic<std::size_t> _next{0};
    alignas(64) std::atomic<bool> _failed{false};
    alignas(64) char _pad{};
    std::mutex _error_mutex{};
    std::size_t _error_index{0};
    std::exception_ptr _error{};

public:
    // SPIKE diagnostics, printed on destruction when JPS_POOL_STATS is set.
    std::atomic<std::uint64_t> stat_busy_ns{0};
    std::uint64_t stat_wall_ns{0};
    std::uint64_t stat_loops{0};
    std::uint64_t stat_items{0};
};
