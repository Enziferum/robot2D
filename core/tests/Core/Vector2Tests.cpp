#include <gtest/gtest.h>
#include <robot2D/Core/Vector2.hpp>

#include <type_traits>
#include <utility>

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
    EXPECT_EQ(robot2D::vec2u(2, 3) * 2u, robot2D::vec2u(4, 6));
}

TEST(Core, Vector2Sum) {
    EXPECT_EQ(robot2D::vec2u(2, 3) + robot2D::vec2u(5, 7), robot2D::vec2u(7, 10));
}

TEST(Core, Vector2Minus) {
    EXPECT_EQ(robot2D::vec2u(5, 7) - robot2D::vec2u(2, 3), robot2D::vec2u(3, 4));
}

TEST(Core, Vector2Divide) {
    EXPECT_EQ(robot2D::vec2u(4, 6) / 2u, robot2D::vec2u(2, 3));
}
TEST(Core, Vector2ConstConversionsAndSignedProducts) {
    const robot2D::vec2f source(3.75F, -2.25F);
    EXPECT_EQ(source.as<int>(), robot2D::vec2i(3, -2));
    EXPECT_EQ(source, robot2D::vec2f(3.75F, -2.25F));
    const robot2D::vec2i a(1, 2);
    const robot2D::vec2i b(3, 4);
    EXPECT_EQ(a.dot(b), 11.F);
    EXPECT_EQ(a.cross(b), -2.F);
    EXPECT_EQ(b.cross(a), 2.F);
    EXPECT_EQ(-a, robot2D::vec2i(-1, -2));
}

TEST(Core, Vector2ComparisonsRemainStrictlyComponentwise) {
    EXPECT_FALSE(robot2D::vec2i(1, 5) < robot2D::vec2i(2, 4));
    EXPECT_FALSE(robot2D::vec2i(1, 5) > robot2D::vec2i(2, 4));
    EXPECT_FALSE(robot2D::vec2i(1, 4) < robot2D::vec2i(1, 5));
    EXPECT_TRUE(robot2D::vec2i(1, 4) < robot2D::vec2i(2, 5));
    EXPECT_TRUE(robot2D::vec2i(2, 5) > robot2D::vec2i(1, 4));
}

TEST(Core, Vector2CompoundOperatorsReturnTheSameObject) {
    robot2D::vec2i value(4, 8);
    EXPECT_EQ(&(value += robot2D::vec2i(2, 4)), &value);
    EXPECT_EQ(value, robot2D::vec2i(6, 12));
    EXPECT_EQ(&(value -= robot2D::vec2i(2, 4)), &value);
    EXPECT_EQ(&(value *= 3), &value);
    EXPECT_EQ(value, robot2D::vec2i(12, 24));
    EXPECT_EQ(&(value /= 2), &value);
    EXPECT_EQ(value, robot2D::vec2i(6, 12));
}

namespace {
    template<typename T>
    constexpr bool vector2ConstantOperations() {
        using Vector = robot2D::Vector2<T>;
        const Vector a(4, 8);
        const Vector b(2, 4);
        Vector value = a + b;
        value -= b;
        value += b;
        value *= T{2};
        value /= T{3};
        Vector copied(value);
        Vector moved(std::move(copied));
        value = a;
        value = std::move(moved);
        return value == a && (a - b) == b && (a / T{2}) == b &&
               (b * T{2}) == a && !(a == b) && a != b;
    }

    template<typename T>
    constexpr bool vector2NonthrowingOperations() {
        using Vector = robot2D::Vector2<T>;
        const Vector a(4, 8);
        const Vector b(2, 4);
        Vector value;
        return noexcept(Vector()) && noexcept(Vector(a)) && noexcept(Vector(std::move(value))) &&
               noexcept(value = a) && noexcept(value = std::move(value)) &&
               noexcept(a + b) && noexcept(a - b) && noexcept(-a) &&
               noexcept(a * T{2}) && noexcept(a / T{2}) &&
               noexcept(value += b) && noexcept(value -= b) &&
               noexcept(value *= T{2}) && noexcept(value /= T{2}) &&
               noexcept(a == b) && noexcept(a != b) && noexcept(a.template as<float>());
    }

    static_assert(vector2ConstantOperations<int>());
    static_assert(vector2ConstantOperations<unsigned>());
    static_assert(vector2ConstantOperations<float>());
    static_assert(vector2NonthrowingOperations<int>());
    static_assert(vector2NonthrowingOperations<unsigned>());
    static_assert(vector2NonthrowingOperations<float>());

    static_assert(robot2D::vec2i() == robot2D::vec2i(0, 0));
    static_assert(robot2D::vec2f(3.75F, -2.25F).as<int>() == robot2D::vec2i(3, -2));
    static_assert(robot2D::vec2f(robot2D::vec2i(1, 2)) == robot2D::vec2f(1.F, 2.F));
    static_assert(-robot2D::vec2i(1, 2) == robot2D::vec2i(-1, -2));
    static_assert(robot2D::vec2i(1, 2).dot(robot2D::vec2i(3, 4)) == 11.F);
    static_assert(robot2D::vec2i(1, 2).cross(robot2D::vec2i(3, 4)) == -2.F);
    static_assert(robot2D::vec2i(1, 2) < robot2D::vec2i(2, 3));
    static_assert(robot2D::vec2i(2, 3) > robot2D::vec2i(1, 2));
    static_assert(!(robot2D::vec2i(1, 5) < robot2D::vec2i(2, 4)));
    static_assert(std::is_same_v<decltype(robot2D::vec2i().dot(robot2D::vec2i())), float>);
    static_assert(std::is_same_v<decltype(robot2D::vec2i().cross(robot2D::vec2i())), float>);
    static_assert(noexcept(robot2D::vec2i().dot(robot2D::vec2i())));
    static_assert(noexcept(robot2D::vec2i().cross(robot2D::vec2i())));
    static_assert(noexcept(robot2D::vec2i() < robot2D::vec2i()));
    static_assert(noexcept(robot2D::vec2i() > robot2D::vec2i()));
}
