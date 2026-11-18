// SPDX-License-Identifier: LGPL-3.0-or-later
#include "job_queue.hpp"

#include "logger.hpp"

#include <cstddef>
#include <cstdint>
#include <exception>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <utility>

JobQueue::JobQueue(std::size_t worker_count)
{
    _workers.reserve(worker_count);
    try {
        for(std::size_t index = 0; index < worker_count; ++index) {
            _workers.emplace_back([this]() { work(); });
        }
    } catch(...) {
        // Destroying a joinable thread terminates the process.
        stop_workers();
        throw;
    }
}

JobQueue::~JobQueue()
{
    try {
        wait();
    } catch(...) {
        // Nobody is left to report the error to.
    }
    stop_workers();
}

void JobQueue::stop_workers()
{
    {
        const std::lock_guard lock(_mutex);
        _stop = true;
    }
    _work_ready.notify_all();
    for(auto& worker : _workers) {
        worker.join();
    }
}

void JobQueue::submit(Job job)
{
    _pending.push_back(std::move(job));
}

void JobQueue::release()
{
    if(_in_flight) {
        throw std::logic_error("JobQueue::release() called before waiting for the previous batch");
    }
    auto batch = std::make_shared<Batch>();
    batch->jobs.swap(_pending);
    batch->remaining = batch->jobs.size();
    batch->logs.resize(batch->jobs.size());
    _in_flight = true;
    {
        const std::lock_guard lock(_mutex);
        _batch = std::move(batch);
        ++_generation;
    }
    _work_ready.notify_all();
}

void JobQueue::wait()
{
    if(!_in_flight) {
        return;
    }
    // Only the owning thread replaces _batch, so reading it here needs no lock.
    auto& batch = *_batch;
    drain(batch);
    {
        std::unique_lock lock(_mutex);
        _batch_done.wait(lock, [&batch]() { return batch.remaining == 0; });
    }
    _in_flight = false;
    for(const auto& messages : batch.logs) {
        for(const auto& message : messages) {
            logging::Logger::instance().log(message);
        }
    }
    if(batch.error) {
        std::rethrow_exception(std::exchange(batch.error, nullptr));
    }
}

void JobQueue::work()
{
    std::uint64_t seen = 0;
    while(true) {
        std::shared_ptr<Batch> batch{};
        {
            std::unique_lock lock(_mutex);
            _work_ready.wait(lock, [this, seen]() { return _stop || _generation != seen; });
            if(_stop) {
                return;
            }
            seen = _generation;
            batch = _batch;
        }
        drain(*batch);
    }
}

void JobQueue::drain(Batch& batch)
{
    while(true) {
        const std::size_t index = batch.next.fetch_add(1);
        if(index >= batch.jobs.size()) {
            return;
        }
        if(!batch.cancelled) {
            try {
                const logging::Logger::Collect collect{batch.logs[index]};
                batch.jobs[index]();
            } catch(...) {
                const std::lock_guard lock(batch.error_mutex);
                if(!batch.error) {
                    batch.error = std::current_exception();
                }
                batch.cancelled = true;
            }
        }
        if(batch.remaining.fetch_sub(1) == 1) {
            // Taking the lock orders this notification after wait() checked its predicate.
            const std::lock_guard lock(_mutex);
            _batch_done.notify_all();
        }
    }
}
