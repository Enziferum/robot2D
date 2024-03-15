#include <gtest/gtest.h>
#include <robot2D/Core/Vector2.hpp>

TEST(Core, Vector2Construct) {
    robot2D::vec2u vec2U{2, 3};
    EXPECT_EQ(vec2U.x, 2u);
    EXPECT_EQ(vec2U.y, 3u);
}

TEST(Core, Vector2Equal) {
    robot2D::vec2u vec2U{2, 3};
    robot2D::vec2u vec2Ueq{2, 3};
    robot2D::vec2u vec2Unoteq{5, 7};

    EXPECT_EQ(vec2U, vec2Ueq);
    EXPECT_FALSE(vec2U == vec2Unoteq);
}

TEST(Core, Vector2More) {
    robot2D::vec2u test{10, 20};
    robot2D::vec2u less{1, 2};
    robot2D::vec2u more{20, 30};

    EXPECT_TRUE(test > less);
    EXPECT_FALSE(test > more);
}

TEST(Core, Vector2Less) {
    robot2D::vec2u test{10, 20};
    robot2D::vec2u less{1, 2};
    robot2D::vec2u more{20, 30};

    EXPECT_TRUE(test < more);
    EXPECT_FALSE(test < less);
}

TEST(Core, Vector2Multiply) {
    robot2D::vec2u vec2U{2, 3};
    robot2D::vec2u vec2Ueq{2, 3};
    robot2D::vec2u vec2Unoteq{5, 7};

    EXPECT_EQ(vec2U, vec2Ueq);
    EXPECT_FALSE(vec2U == vec2Unoteq);
}

TEST(Core, Vector2Sum) {
    robot2D::vec2u vec2U{2, 3};
    robot2D::vec2u vec2Ueq{2, 3};
    robot2D::vec2u vec2Unoteq{5, 7};

    EXPECT_EQ(vec2U, vec2Ueq);
    EXPECT_FALSE(vec2U == vec2Unoteq);
}

TEST(Core, Vector2Minus) {
    robot2D::vec2u vec2U{2, 3};
    robot2D::vec2u vec2Ueq{2, 3};
    robot2D::vec2u vec2Unoteq{5, 7};

    EXPECT_EQ(vec2U, vec2Ueq);
    EXPECT_FALSE(vec2U == vec2Unoteq);
}

TEST(Core, Vector2Divide) {
    robot2D::vec2u vec2U{2, 3};
    robot2D::vec2u vec2Ueq{2, 3};
    robot2D::vec2u vec2Unoteq{5, 7};

    EXPECT_EQ(vec2U, vec2Ueq);
    EXPECT_FALSE(vec2U == vec2Unoteq);
}