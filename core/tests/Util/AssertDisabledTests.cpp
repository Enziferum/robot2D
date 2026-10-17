#include <robot2D/Util/Assert.hpp>
#include <gtest/gtest.h>

#ifdef RB_ASSERTS
    #error This target must disable assertions independently of the build configuration.
#endif

TEST(AssertDisabled, EvaluatesNeitherConditionNorMessageArguments) {
    int calls = 0;
    RB_ASSERT(++calls == 0);
    RB_CORE_ASSERT(++calls == 0, "unused {}", ++calls);
    EXPECT_EQ(calls, 0);
}

TEST(AssertDisabled, BehavesAsOneStatementInConditional) {
    if(true)
        RB_ASSERT(false);
    else
        FAIL();
}
