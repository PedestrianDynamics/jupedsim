// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "logger.hpp"

#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

/// Collects jobs and releases them as one batch to a fixed set of worker threads.
///
/// Usage is phase based: submit() any number of jobs, release() them, then wait() for the
/// batch to finish. The thread calling wait() executes jobs as well, so a queue with
/// `worker_count` workers runs up to `worker_count + 1` jobs at once, and a queue without
/// workers runs every job inline in wait().
///
/// submit(), release() and wait() have to be called from one owning thread. Jobs run in no
/// particular order and must not touch the queue. What jobs log is collected and passed to the
/// logger's callbacks by wait(), on the owning thread and in the order the jobs were submitted,
/// so callbacks never run on a worker.
class JobQueue
{
public:
    using Job = std::function<void()>;

    /// Throws if a worker cannot be started; the workers started so far are joined first.
    explicit JobQueue(std::size_t worker_count);
    /// Lets a batch still in flight finish, discarding its error, and joins the workers.
    ~JobQueue();
    JobQueue(const JobQueue& other) = delete;
    JobQueue& operator=(const JobQueue& other) = delete;
    JobQueue(JobQueue&& other) = delete;
    JobQueue& operator=(JobQueue&& other) = delete;

    /// Collects a job for the next batch. Nothing runs until release().
    void submit(Job job);

    /// Hands every job collected so far to the workers and returns immediately.
    /// Throws std::logic_error if the previous batch has not been waited for.
    void release();

    /// Blocks until the released batch is done, helping to execute it. Rethrows the first
    /// exception a job threw; jobs not yet started when that happened are skipped.
    /// Returns at once if no batch is in flight.
    void wait();

    std::size_t worker_count() const { return _workers.size(); }

private:
    struct Batch {
        std::vector<Job> jobs{};
        std::atomic<std::size_t> next{0};
        std::atomic<std::size_t> remaining{0};
        std::atomic<bool> cancelled{false};
        std::mutex error_mutex{};
        std::exception_ptr error{};
        /// What each job logged, by job.
        std::vector<std::vector<logging::Message>> logs{};
    };

    void work();
    void stop_workers();
    /// Executes jobs of `batch` until none is left to claim.
    void drain(Batch& batch);

    std::vector<std::thread> _workers{};
    std::vector<Job> _pending{};
    // Workers hold their own reference, so one waking late only ever sees the batch it was
    // woken for, never one released after it.
    std::shared_ptr<Batch> _batch{};
    bool _in_flight{false};

    std::mutex _mutex{};
    std::condition_variable _work_ready{};
    std::condition_variable _batch_done{};
    std::uint64_t _generation{0};
    bool _stop{false};
};

/// Calls `fn(index)` for every index in [0, n) and blocks until all calls are done. With
/// workers and more than one index, the calls are split into contiguous chunks that run on
/// `queue`, released along with whatever else was collected in it. Otherwise they run on the
/// calling thread and `queue` is left alone.
template <typename Fn>
void parallel_for(JobQueue& queue, std::size_t n, Fn&& fn)
{
    if(queue.worker_count() == 0 || n <= 1) {
        for(std::size_t index = 0; index < n; ++index) {
            fn(index);
        }
        return;
    }
    // A few chunks per thread so that threads finishing early pick up the slack.
    constexpr std::size_t chunks_per_thread = 16;
    const std::size_t chunk_count = std::min(n, (queue.worker_count() + 1) * chunks_per_thread);
    for(std::size_t chunk = 0; chunk < chunk_count; ++chunk) {
        const std::size_t begin = n * chunk / chunk_count;
        const std::size_t end = n * (chunk + 1) / chunk_count;
        queue.submit([begin, end, &fn]() {
            for(std::size_t index = begin; index < end; ++index) {
                fn(index);
            }
        });
    }
    queue.release();
    queue.wait();
}
