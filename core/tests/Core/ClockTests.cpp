#include <gtest/gtest.h>
#include <robot2D/Core/Clock.hpp>

#include <chrono>
#include <thread>

namespace {
    using Monotonic = std::chrono::steady_clock;

    std::int64_t microsecondsBetween(Monotonic::time_point end, Monotonic::time_point start) {
        return std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    }
}

TEST(Clock, DurationIsNonnegativeMonotonicAndReadOnly) {
    const robot2D::Clock clock;
    auto previous = clock.duration().asMicroSeconds();
    EXPECT_GE(previous, 0);
    for(int iteration = 0; iteration < 128; ++iteration) {
        const auto current = clock.duration().asMicroSeconds();
        EXPECT_GE(current, previous);
        previous = current;
    }
    EXPECT_EQ(clock.getRestartsCount(), 0);
}

TEST(Clock, FirstRestartMeasuresFromConstruction) {
    const auto beforeConstruction = Monotonic::now();
    robot2D::Clock clock;
    const auto afterConstruction = Monotonic::now();
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    const auto beforeRestart = Monotonic::now();
    const auto elapsed = clock.restart().asMicroSeconds();
    const auto afterRestart = Monotonic::now();
    // Bracket the actual observations; no deadline depends on scheduler latency.
    EXPECT_GE(elapsed, microsecondsBetween(beforeRestart, afterConstruction));
    EXPECT_LE(elapsed, microsecondsBetween(afterRestart, beforeConstruction));
    EXPECT_EQ(clock.getRestartsCount(), 1);
}

TEST(Clock, SubsequentRestartMeasuresOnlyTheLatestInterval) {
    robot2D::Clock clock;
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    const auto beforeFirst = Monotonic::now();
    clock.restart();
    const auto afterFirst = Monotonic::now();
    const auto beforeSecond = Monotonic::now();
    const auto elapsed = clock.restart().asMicroSeconds();
    const auto afterSecond = Monotonic::now();
    EXPECT_GE(elapsed, microsecondsBetween(beforeSecond, afterFirst));
    // A restart may include the sub-microsecond remainder from the preceding call.
    EXPECT_LE(elapsed, microsecondsBetween(afterSecond, beforeFirst) + 1);
    EXPECT_EQ(clock.getRestartsCount(), 2);
}

TEST(Clock, RestartDoesNotResetLifetimeDuration) {
    robot2D::Clock clock;
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    const auto before = clock.duration().asMicroSeconds();
    clock.restart();
    EXPECT_GE(clock.duration().asMicroSeconds(), before);
    clock.restart();
    EXPECT_GE(clock.duration().asMicroSeconds(), before);
    EXPECT_EQ(clock.getRestartsCount(), 2);
}

TEST(Clock, RapidRestartsAllowZeroIntervalsAndCountEveryCall) {
    robot2D::Clock clock;
    for(std::size_t count = 1; count <= 32; ++count) {
        EXPECT_GE(clock.restart().asMicroSeconds(), 0);
        EXPECT_EQ(clock.getRestartsCount(), count);
    }
}

TEST(Clock, CopyPreservesLifetimeAndHasIndependentRestartState) {
    robot2D::Clock original;
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    original.restart();
    auto copy = original;
    EXPECT_EQ(copy.getRestartsCount(), 1);
    const auto lifetime = original.duration().asMicroSeconds();
    EXPECT_GE(copy.duration().asMicroSeconds(), lifetime);
    copy.restart();
    EXPECT_EQ(copy.getRestartsCount(), 2);
    EXPECT_EQ(original.getRestartsCount(), 1);
    EXPECT_GE(copy.duration().asMicroSeconds(), lifetime);
}

TEST(Clock, RapidRestartsPreserveElapsedMicroseconds) {
    robot2D::Clock clock;
    clock.restart();
    const auto begin = std::chrono::steady_clock::now();
    std::int64_t accumulated = 0;
    for(unsigned i = 0; i < 200000; ++i)
        accumulated += clock.restart().asMicroSeconds();
    const auto end = std::chrono::steady_clock::now();
    accumulated += clock.restart().asMicroSeconds();
    const auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();
    // The summed intervals enclose the wall-time sample. Allow clock sampling skew,
    // not a performance threshold: sub-microsecond fractions must accumulate.
    EXPECT_GE(accumulated + 100, elapsed);
}
