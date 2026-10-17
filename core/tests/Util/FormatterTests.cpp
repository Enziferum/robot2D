#include <gtest/gtest.h>
#include <robot2D/Util/Logger.hpp>

TEST(LogFormatter, UnsignedVectorSupportsDefaultAndDecimal) {
    const robot2D::vec2u value{12, 34};
    EXPECT_EQ(fmt::format("{}", value), "(12, 34)");
    EXPECT_EQ(fmt::format("{:d}", value), "(12, 34)");
}

TEST(LogFormatter, FloatVectorPreservesFractionsAndSign) {
    const robot2D::vec2f value{1.5f, -2.5f};
    EXPECT_EQ(fmt::format("{}", value), "(1.5, -2.5)");
    EXPECT_EQ(fmt::format("{:f}", value), "(1.5, -2.5)");
    EXPECT_EQ(fmt::format("{:e}", value), "(1.5e+00, -2.5e+00)");
}

TEST(LogFormatter, RejectsUnsupportedPresentation) {
    EXPECT_THROW((void)fmt::format(fmt::runtime("{:x}"), robot2D::vec2u{}), fmt::format_error);
    EXPECT_THROW((void)fmt::format(fmt::runtime("{:d}"), robot2D::vec2f{}), fmt::format_error);
}
