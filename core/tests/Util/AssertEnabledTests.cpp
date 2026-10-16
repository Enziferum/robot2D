// The legacy include must be self-contained, including logging and abort support.
#include <robot2D/Core/Assert.hpp>
#include <gtest/gtest.h>
#include <spdlog/sinks/null_sink.h>

#ifndef RB_ASSERTS
    #error This target must enable assertions independently of the build configuration.
#endif

TEST(AssertEnabled, EvaluatesConditionOnceAndDefersMessageArguments) {
    int condition = 0;
    int message = 0;
    RB_ASSERT(++condition == 1);
    RB_CORE_ASSERT(true, "unused {}", ++message);
    EXPECT_EQ(condition, 1);
    EXPECT_EQ(message, 0);
}

TEST(AssertEnabled, BehavesAsOneStatementInConditional) {
    if(true)
        RB_ASSERT(true);
    else
        FAIL();
}

TEST(AssertEnabled, FailureWithoutMessageTerminates) {
    EXPECT_DEATH(RB_ASSERT(false), "Assertion 'false' failed at");
}

TEST(AssertEnabled, FormattedFailureTerminatesWithContext) {
    EXPECT_DEATH(RB_CORE_ASSERT(false, "value {} / {}", 17, 25), "value 17 / 25");
}

TEST(AssertEnabled, LoggingFailureStillTerminatesWithDiagnostic) {
    EXPECT_DEATH({
        auto sink = std::make_shared<spdlog::sinks::null_sink_mt>();
        spdlog::register_logger(std::make_shared<spdlog::logger>("Robot2DCore", sink));
        RB_CORE_ASSERT(false, "fallback-context");
    }, "Assertion logging failed");
}
