#include <gtest/gtest.h>
#include <robot2D/Core/Clock.hpp>
#include <chrono>

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
