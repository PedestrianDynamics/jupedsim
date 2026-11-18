// SPDX-License-Identifier: LGPL-3.0-or-later
#include "job_queue.hpp"
#include "logger.hpp"

#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

namespace
{
class JobQueueWithWorkers : public ::testing::TestWithParam<std::size_t>
{
};

/// Spins until `counter` reaches `target` or a generous deadline passes.
bool wait_for(const std::atomic<int>& counter, int target)
{
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(10);
    while(counter < target) {
        if(std::chrono::steady_clock::now() > deadline) {
            return false;
        }
        std::this_thread::yield();
    }
    return true;
}
} // namespace

INSTANTIATE_TEST_SUITE_P(WorkerCounts, JobQueueWithWorkers, ::testing::Values(0, 1, 4));

TEST_P(JobQueueWithWorkers, RunsEveryJobExactlyOnce)
{
    JobQueue queue{GetParam()};
    std::vector<std::atomic<int>> runs(500);
    for(auto& run : runs) {
        queue.submit([&run]() { ++run; });
    }
    queue.release();
    queue.wait();
    for(const auto& run : runs) {
        EXPECT_EQ(run, 1);
    }
}

TEST_P(JobQueueWithWorkers, CanBeReusedForManyBatches)
{
    JobQueue queue{GetParam()};
    std::atomic<int> total{0};
    for(int batch = 0; batch < 200; ++batch) {
        for(int job = 0; job < 10; ++job) {
            queue.submit([&total]() { ++total; });
        }
        queue.release();
        queue.wait();
        EXPECT_EQ(total, (batch + 1) * 10);
    }
}

TEST_P(JobQueueWithWorkers, AnEmptyBatchFinishesImmediately)
{
    JobQueue queue{GetParam()};
    queue.release();
    queue.wait();
}

TEST_P(JobQueueWithWorkers, WaitWithoutReleaseReturns)
{
    JobQueue queue{GetParam()};
    queue.wait();
}

TEST_P(JobQueueWithWorkers, RethrowsTheJobsExceptionAndStaysUsable)
{
    JobQueue queue{GetParam()};
    queue.submit([]() { throw std::runtime_error("job failed"); });
    queue.release();
    EXPECT_THROW(queue.wait(), std::runtime_error);

    std::atomic<int> runs{0};
    queue.submit([&runs]() { ++runs; });
    queue.release();
    queue.wait();
    EXPECT_EQ(runs, 1);
}

TEST_P(JobQueueWithWorkers, ReleasingTwiceWithoutWaitingIsAnError)
{
    JobQueue queue{GetParam()};
    queue.release();
    EXPECT_THROW(queue.release(), std::logic_error);
    queue.wait();
}

TEST_P(JobQueueWithWorkers, JobsSubmittedWhileInFlightBelongToTheNextBatch)
{
    JobQueue queue{GetParam()};
    std::atomic<int> first{0};
    std::atomic<int> second{0};
    queue.submit([&first]() { ++first; });
    queue.release();
    queue.submit([&second]() { ++second; });
    queue.wait();
    EXPECT_EQ(first, 1);
    EXPECT_EQ(second, 0);

    queue.release();
    queue.wait();
    EXPECT_EQ(second, 1);
}

TEST_P(JobQueueWithWorkers, ParallelForVisitsEveryIndexOnce)
{
    JobQueue queue{GetParam()};
    for(const std::size_t n : {0, 1, 7, 1000}) {
        std::vector<std::atomic<int>> visits(n);
        parallel_for(queue, n, [&visits](std::size_t index) { ++visits[index]; });
        for(const auto& visit : visits) {
            EXPECT_EQ(visit, 1);
        }
    }
}

TEST_P(JobQueueWithWorkers, PassesOnWhatJobsLogInOrderOnTheOwningThread)
{
    std::vector<std::string> messages{};
    std::vector<std::thread::id> threads{};
    auto& logger = logging::Logger::instance();
    logger.set_warning_callback([&](const std::string& message) {
        messages.push_back(message);
        threads.push_back(std::this_thread::get_id());
    });
    JobQueue queue{GetParam()};
    parallel_for(queue, 1000, [&logger](std::size_t index) {
        logger.log_warning_message(std::to_string(index));
    });
    logger.clear_all_callbacks();

    std::vector<std::string> expected{};
    for(std::size_t index = 0; index < 1000; ++index) {
        expected.push_back(std::to_string(index));
    }
    EXPECT_EQ(messages, expected);
    EXPECT_EQ(threads, std::vector<std::thread::id>(1000, std::this_thread::get_id()));
}

TEST_P(JobQueueWithWorkers, ParallelForRethrows)
{
    JobQueue queue{GetParam()};
    EXPECT_THROW(
        parallel_for(
            queue,
            100,
            [](std::size_t index) {
                if(index == 42) {
                    throw std::runtime_error("index failed");
                }
            }),
        std::runtime_error);
}

TEST(JobQueue, WithoutWorkersNothingRunsBeforeWait)
{
    JobQueue queue{0};
    std::atomic<int> runs{0};
    queue.submit([&runs]() { ++runs; });
    EXPECT_EQ(runs, 0);
    queue.release();
    EXPECT_EQ(runs, 0);
    queue.wait();
    EXPECT_EQ(runs, 1);
}

TEST(JobQueue, ReleasedJobsRunWithoutWaiting)
{
    JobQueue queue{1};
    std::atomic<int> runs{0};
    queue.submit([&runs]() { ++runs; });
    queue.release();
    EXPECT_TRUE(wait_for(runs, 1));
    queue.wait();
}

TEST(JobQueue, JobsRunConcurrently)
{
    // Each job blocks until all of them have started, which only finishes if they run at the
    // same time: three workers plus the waiting thread.
    constexpr int job_count = 4;
    JobQueue queue{job_count - 1};
    std::atomic<int> started{0};
    std::atomic<int> met{0};
    for(int job = 0; job < job_count; ++job) {
        queue.submit([&]() {
            ++started;
            if(wait_for(started, job_count)) {
                ++met;
            }
        });
    }
    queue.release();
    queue.wait();
    EXPECT_EQ(met, job_count);
}

TEST(JobQueue, DestructionFinishesTheBatchInFlight)
{
    std::atomic<int> runs{0};
    {
        JobQueue queue{2};
        for(int job = 0; job < 50; ++job) {
            queue.submit([&runs]() { ++runs; });
        }
        queue.release();
    }
    EXPECT_EQ(runs, 50);
}
