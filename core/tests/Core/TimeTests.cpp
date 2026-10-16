#include <gtest/gtest.h>
#include <robot2D/Core/Time.hpp>

#include <cmath>
#include <limits>
#include <type_traits>
#include <utility>

TEST(Time, DefaultIsZeroInAllUnits) {
    const robot2D::Time value;
    EXPECT_EQ(value.asMicroSeconds(), 0);
    EXPECT_EQ(value.asMilliSeconds(), 0.F);
    EXPECT_EQ(value.asSeconds(), 0.F);
}

TEST(Time, PreservesExactSignedMicrosecondsIncludingLimits) {
    for(const auto count : {std::numeric_limits<std::int64_t>::min(),
                            std::int64_t{-1000001}, std::int64_t{-1}, std::int64_t{0},
                            std::int64_t{1}, std::int64_t{1000001},
                            std::numeric_limits<std::int64_t>::max()}) {
        const robot2D::Time value(count);
        EXPECT_EQ(value.asMicroSeconds(), count);
    }
}

TEST(Time, ConvertsFractionalSecondsAndMillisecondsWithSign) {
    const robot2D::Time positive(1250000);
    const robot2D::Time negative(-1250000);
    EXPECT_EQ(positive.asSeconds(), 1.25F);
    EXPECT_EQ(positive.asMilliSeconds(), 1250.F);
    EXPECT_EQ(negative.asSeconds(), -1.25F);
    EXPECT_EQ(negative.asMilliSeconds(), -1250.F);
    EXPECT_FLOAT_EQ(robot2D::Time(1).asSeconds(), 0.000001F);
    EXPECT_FLOAT_EQ(robot2D::Time(-1).asMilliSeconds(), -0.001F);
}

TEST(Time, WholeSecondsAreNotRoundedBeforeDivision) {
    // These exact float results used to be off by one ULP after a premature cast.
    EXPECT_EQ(robot2D::Time(16008000000LL).asSeconds(), 16008.F);
    EXPECT_EQ(robot2D::Time(-16008000000LL).asSeconds(), -16008.F);
    EXPECT_EQ(robot2D::Time(16024000000LL).asSeconds(), 16024.F);
}

TEST(Time, WholeMillisecondsAreNotRoundedBeforeDivision) {
    EXPECT_EQ(robot2D::Time(134219000LL).asMilliSeconds(), 134219.F);
    EXPECT_EQ(robot2D::Time(-134219000LL).asMilliSeconds(), -134219.F);
}

TEST(Time, ExtremeConversionsRemainFiniteAndKeepSign) {
    const robot2D::Time minimum(std::numeric_limits<std::int64_t>::min());
    const robot2D::Time maximum(std::numeric_limits<std::int64_t>::max());
    EXPECT_TRUE(std::isfinite(minimum.asSeconds()));
    EXPECT_TRUE(std::isfinite(maximum.asSeconds()));
    EXPECT_TRUE(std::isfinite(minimum.asMilliSeconds()));
    EXPECT_TRUE(std::isfinite(maximum.asMilliSeconds()));
    EXPECT_LT(minimum.asSeconds(), 0.F);
    EXPECT_GT(maximum.asSeconds(), 0.F);
    EXPECT_LT(minimum.asMilliSeconds(), 0.F);
    EXPECT_GT(maximum.asMilliSeconds(), 0.F);
}

namespace {
    constexpr robot2D::Time zero;
    constexpr robot2D::Time fraction(1250000);
    static_assert(zero.asMicroSeconds() == 0 && zero.asSeconds() == 0.F);
    static_assert(fraction.asMicroSeconds() == 1250000);
    static_assert(fraction.asSeconds() == 1.25F && fraction.asMilliSeconds() == 1250.F);
    static_assert(robot2D::Time(-1250000).asSeconds() == -1.25F);
    static_assert(robot2D::Time(16008000000LL).asSeconds() == 16008.F);
    static_assert(robot2D::Time(134219000LL).asMilliSeconds() == 134219.F);
    static_assert(robot2D::Time(std::numeric_limits<std::int64_t>::min()).asMicroSeconds() ==
        std::numeric_limits<std::int64_t>::min());
    static_assert(robot2D::Time(std::numeric_limits<std::int64_t>::max()).asMicroSeconds() ==
        std::numeric_limits<std::int64_t>::max());

    constexpr robot2D::Time copyAndMoveTime() {
        robot2D::Time source(-1250000);
        robot2D::Time copied(source);
        robot2D::Time moved(std::move(copied));
        robot2D::Time result;
        result = source;
        result = std::move(moved);
        return result;
    }

    static_assert(copyAndMoveTime().asMicroSeconds() == -1250000);
    static_assert(std::is_nothrow_constructible_v<robot2D::Time, std::int64_t>);
    static_assert(std::is_nothrow_default_constructible_v<robot2D::Time>);
    static_assert(std::is_nothrow_copy_constructible_v<robot2D::Time>);
    static_assert(std::is_nothrow_copy_assignable_v<robot2D::Time>);
    static_assert(std::is_nothrow_move_constructible_v<robot2D::Time>);
    static_assert(std::is_nothrow_move_assignable_v<robot2D::Time>);
    static_assert(noexcept(fraction.asMicroSeconds()) && noexcept(fraction.asMilliSeconds()) &&
        noexcept(fraction.asSeconds()));
}
