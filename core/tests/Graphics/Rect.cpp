#include <gtest/gtest.h>
#include <robot2D/Graphics/Rect.hpp>

TEST(Graphics, Rect) {
    robot2D::IntRect rect{100, 100, 70, 70};
    EXPECT_EQ(rect.lx, 100);
    EXPECT_EQ(rect.ly, 100);
    EXPECT_EQ(rect.width, 70);
    EXPECT_EQ(rect.height, 70);
}

TEST(Graphics, RectEqual) {
    robot2D::IntRect rect{};
    robot2D::IntRect expectRect{};
    EXPECT_EQ(rect, expectRect);
}

TEST(Graphics, Graphics_RectIntersects_Test) {
    robot2D::IntRect rect1{};
    robot2D::IntRect rect2{};
    EXPECT_TRUE(rect1.intersects(rect2));
}

TEST(Graphics, Graphics_RectOverlap_Test) {
    robot2D::IntRect rect1{};
    robot2D::IntRect rect2{};
    robot2D::IntRect overlap{};
    robot2D::IntRect overlapExpect{};
    rect1.intersects(rect2, overlap);
    EXPECT_EQ(overlap, overlapExpect);
}

TEST(Graphics, Graphics_RectContainsPoint_Test) {
    robot2D::vec2i testPoint = {234, 131};
    robot2D::IntRect rect{{100, 100}, {300, 300}};
    EXPECT_TRUE(rect.contains(testPoint));
}

TEST(Graphics, Graphics_RectContainsOtherRect_Test) {
    robot2D::IntRect rect{100, 100, 200, 200};
    robot2D::IntRect insideRect{150, 150, 30, 30};
    EXPECT_TRUE(rect.contains(insideRect));
}

TEST(Graphics, Graphics_RectNotContainsOtherRect_Test) {
    robot2D::IntRect rect{100, 100, 200, 200};
    robot2D::IntRect insideRect{150, 150, 110, 30};
    EXPECT_FALSE(rect.contains(insideRect));
}

TEST(Graphics, Graphics_RectCreateFrom4Anchors_Test) {
//    robot2D::IntRect expectRect;
//    std::array<robot2D::vec2i, 4> anchors;
//    robot2D::IntRect testRect = robot2D::IntRect::create(anchors[0],
//                                                         anchors[1],
//                                                         anchors[2],
//                                                         anchors[3]);
//    EXPECT_EQ(expectRect, testRect);
}
