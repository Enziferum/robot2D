#include <gtest/gtest.h>
#include <robot2D/Core/Vector3.hpp>

#include <stdexcept>
#include <type_traits>
#include <utility>

TEST(Core, Vector3ConstructionAndConstConversions) {
    EXPECT_EQ(robot2D::vec3i(), robot2D::vec3i(0, 0, 0));
    const robot2D::vec2i xy(2, 3);
    EXPECT_EQ(robot2D::vec3i(xy), robot2D::vec3i(2, 3, 0));
    EXPECT_EQ(robot2D::vec3f(xy), robot2D::vec3f(2.F, 3.F, 0.F));
    const robot2D::vec3f value(3.75F, -2.25F, 5.5F);
    EXPECT_EQ(value.as<int>(), robot2D::vec3i(3, -2, 5));
    EXPECT_EQ(value.asVec2<int>(), robot2D::vec2i(3, -2));
    EXPECT_EQ(value, robot2D::vec3f(3.75F, -2.25F, 5.5F));
}

TEST(Core, Vector3ArithmeticUsesAllComponents) {
    const robot2D::vec3i a(4, 8, 12);
    const robot2D::vec3i b(2, 4, 6);
    EXPECT_EQ(a + b, robot2D::vec3i(6, 12, 18));
    EXPECT_EQ(a - b, b);
    EXPECT_EQ(-b, robot2D::vec3i(-2, -4, -6));
    EXPECT_EQ(a * 2, robot2D::vec3i(8, 16, 24));
    EXPECT_EQ(a / 2, b);
}

TEST(Core, Vector3EqualityIncludesZ) {
    const robot2D::vec3i a(1, 2, 3);
    EXPECT_EQ(a, robot2D::vec3i(1, 2, 3));
    EXPECT_NE(a, robot2D::vec3i(1, 2, 4));
    EXPECT_NE(a, robot2D::vec3i(1, 4, 3));
    EXPECT_NE(a, robot2D::vec3i(4, 2, 3));
}

TEST(Core, Vector3CompoundOperatorsReturnTheSameObject) {
    robot2D::vec3i value(4, 8, 12);
    EXPECT_EQ(&(value += robot2D::vec3i(2, 4, 6)), &value);
    EXPECT_EQ(value, robot2D::vec3i(6, 12, 18));
    EXPECT_EQ(&(value -= robot2D::vec3i(2, 4, 6)), &value);
    EXPECT_EQ(&(value *= 3), &value);
    EXPECT_EQ(value, robot2D::vec3i(12, 24, 36));
    EXPECT_EQ(&(value /= 2), &value);
    EXPECT_EQ(value, robot2D::vec3i(6, 12, 18));
}

namespace {
    template<typename T>
    constexpr bool vector3ConstantOperations() {
        using Vector = robot2D::Vector3<T>;
        const Vector a(4, 8, 12);
        const Vector b(2, 4, 6);
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
    constexpr bool vector3NonthrowingOperations() {
        using Vector = robot2D::Vector3<T>;
        const Vector a(4, 8, 12);
        const Vector b(2, 4, 6);
        Vector value;
        return noexcept(Vector()) && noexcept(Vector(a)) && noexcept(Vector(std::move(value))) &&
               noexcept(value = a) && noexcept(value = std::move(value)) &&
               noexcept(a + b) && noexcept(a - b) && noexcept(-a) &&
               noexcept(a * T{2}) && noexcept(a / T{2}) &&
               noexcept(value += b) && noexcept(value -= b) &&
               noexcept(value *= T{2}) && noexcept(value /= T{2}) &&
               noexcept(a == b) && noexcept(a != b) && noexcept(a.template as<float>());
    }

    static_assert(vector3ConstantOperations<int>());
    static_assert(vector3ConstantOperations<unsigned>());
    static_assert(vector3ConstantOperations<float>());
    static_assert(vector3NonthrowingOperations<int>());
    static_assert(vector3NonthrowingOperations<unsigned>());
    static_assert(vector3NonthrowingOperations<float>());

    static_assert(robot2D::vec3i() == robot2D::vec3i(0, 0, 0));
    static_assert(robot2D::vec3i(robot2D::vec2i(2, 3)) == robot2D::vec3i(2, 3, 0));
    static_assert(robot2D::vec3f(robot2D::vec2i(2, 3)) == robot2D::vec3f(2.F, 3.F, 0.F));
    static_assert(robot2D::vec3f(robot2D::vec3i(1, 2, 3)) == robot2D::vec3f(1.F, 2.F, 3.F));
    static_assert(robot2D::vec3f(3.75F, -2.25F, 5.5F).as<int>() == robot2D::vec3i(3, -2, 5));
    static_assert(robot2D::vec3f(3.75F, -2.25F, 5.5F).asVec2<int>() == robot2D::vec2i(3, -2));
    static_assert(noexcept(robot2D::vec3i(robot2D::vec2i())));
    static_assert(noexcept(robot2D::vec3f(robot2D::vec2i())));
    static_assert(noexcept(robot2D::vec3f().asVec2<int>()));
    static_assert(-robot2D::vec3i(1, 2, 3) == robot2D::vec3i(-1, -2, -3));
}

namespace {
    struct ThrowingScalar {
        explicit ThrowingScalar(int = 0) noexcept {
        }
        ThrowingScalar(const ThrowingScalar&) = default;
        ThrowingScalar& operator=(const ThrowingScalar&) = default;
        ThrowingScalar(ThrowingScalar&&) {
            throw std::runtime_error("scalar move");
        }
        ThrowingScalar& operator=(ThrowingScalar&&) {
            throw std::runtime_error("scalar assignment");
        }
        explicit operator int() const {
            throw std::runtime_error("scalar conversion");
        }
    };

    ThrowingScalar operator+(const ThrowingScalar&, const ThrowingScalar&) {
        throw std::runtime_error("scalar addition");
    }

    template<template<typename> class Vector>
    void expectScalarExceptionsToPropagate() {
        Vector<ThrowingScalar> constructionSource;
        Vector<ThrowingScalar> assignmentSource;
        Vector<ThrowingScalar> source;
        Vector<ThrowingScalar> target;
        EXPECT_THROW((Vector<ThrowingScalar>(std::move(constructionSource))), std::runtime_error);
        EXPECT_THROW(target = std::move(assignmentSource), std::runtime_error);
        EXPECT_THROW(source + target, std::runtime_error);
        EXPECT_THROW(source.template as<int>(), std::runtime_error);
    }

    static_assert(!std::is_nothrow_move_constructible_v<robot2D::Vector2<ThrowingScalar> >);
    static_assert(!std::is_nothrow_move_assignable_v<robot2D::Vector2<ThrowingScalar> >);
    static_assert(!std::is_nothrow_move_constructible_v<robot2D::Vector3<ThrowingScalar> >);
    static_assert(!std::is_nothrow_move_assignable_v<robot2D::Vector3<ThrowingScalar> >);
    static_assert(!noexcept(std::declval<const robot2D::Vector2<ThrowingScalar>&>().as<int>()));
    static_assert(!noexcept(std::declval<const robot2D::Vector3<ThrowingScalar>&>().asVec2<int>()));
}

TEST(Core, Vector2CustomScalarExceptionsPropagate) {
    expectScalarExceptionsToPropagate<robot2D::Vector2>();
}

TEST(Core, Vector3CustomScalarExceptionsPropagate) {
    expectScalarExceptionsToPropagate<robot2D::Vector3>();
}
