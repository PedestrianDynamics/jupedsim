// SPDX-License-Identifier: LGPL-3.0-or-later
#include "kernel.hpp"
#include "line_segment.hpp"
#include "test_common.hpp"

#include <fmt/printf.h>
#include <gtest/gtest.h>

#include <cmath>
#include <type_traits>

TEST(Kernel, IsUnitLength)
{
    EXPECT_TRUE(Vector(1, 0).is_unit_length());
    EXPECT_TRUE(Vector(-1, 0).is_unit_length());
    EXPECT_TRUE(Vector(0, -1).is_unit_length());
    EXPECT_TRUE(Vector(0, 1).is_unit_length());
    EXPECT_TRUE(Vector(1, 1).normalized().is_unit_length());
    EXPECT_FALSE(Vector(-1, -1).is_unit_length());
}

TEST(Kernel, DefaultIsOrigin)
{
    const Point p{};
    EXPECT_EQ(p.x(), 0.0);
    EXPECT_EQ(p.y(), 0.0);
}

TEST(Kernel, NormalizedOfZeroIsZero)
{
    EXPECT_EQ(Vector(0, 0).normalized(), Vector(0, 0));
    const auto [norm, direction] = Vector(0, 0).norm_and_normalized();
    EXPECT_EQ(norm, 0.0);
    EXPECT_EQ(direction, Vector(0, 0));
}

TEST(Kernel, NormAndNormalized)
{
    const auto [norm, direction] = Vector(3, 4).norm_and_normalized();
    EXPECT_DOUBLE_EQ(norm, 5.0);
    EXPECT_DOUBLE_EQ(direction.x(), 0.6);
    EXPECT_DOUBLE_EQ(direction.y(), 0.8);
}

TEST(Kernel, DivisionByZeroOrNegativeReturnsOperandUnchanged)
{
    EXPECT_EQ(Vector(1, 2) / 0.0, Vector(1, 2));
    EXPECT_EQ(Vector(1, 2) / -2.0, Vector(1, 2));
    EXPECT_EQ(Vector(1, 2) / 2.0, Vector(0.5, 1.0));
}

TEST(Kernel, Rotate)
{
    EXPECT_EQ(Vector(1, 2).rotate(0.0, 1.0), Vector(-2, 1));
    EXPECT_EQ(Vector(1, 2).rotate90_deg(), Vector(-2, 1));
}

TEST(Kernel, EllipseCoordinatesRoundTrip)
{
    const Point center{2, 3};
    const double c = std::cos(0.5);
    const double s = std::sin(0.5);
    const Vector local = Point(5, -1).transform_to_ellipse_coordinates(center, c, s);
    const Point world = local.transform_to_cartesian_coordinates(center, c, s);
    EXPECT_NEAR(world.x(), 5.0, 1e-12);
    EXPECT_NEAR(world.y(), -1.0, 1e-12);
}

TEST(Kernel, OrderIsLexicographic)
{
    EXPECT_TRUE(Point(1, 9) < Point(2, 0));
    EXPECT_TRUE(Point(1, 0) < Point(1, 1));
    EXPECT_FALSE(Point(1, 1) < Point(1, 1));
}

TEST(Kernel, IsZeroLength)
{
    EXPECT_TRUE(Vector(1e-7, -1e-7).is_zero_length());
    EXPECT_FALSE(Vector(1e-5, 0).is_zero_length());
}

// A concept, because a requires-expression only yields false in a templated context.
template <typename A, typename B>
concept Addable = requires(A a, B b) { a + b; };

template <typename A, typename B>
concept Subtractable = requires(A a, B b) { a - b; };

template <typename T>
concept Scalable = requires(T t) {
    t * 2.0;
    t / 2.0;
};

static_assert(!Addable<Point, Point>);
static_assert(Addable<Point, Vector>);
static_assert(!Addable<Vector, Point>);
static_assert(Subtractable<Point, Vector>);
static_assert(!Subtractable<Vector, Point>);
static_assert(Scalable<Vector>);
static_assert(!Scalable<Point>);
static_assert(!std::is_convertible_v<Point, Vector>);
static_assert(!std::is_convertible_v<Vector, Point>);
static_assert(std::is_trivially_copyable_v<Point> && sizeof(Point) == 16);
static_assert(std::is_trivially_copyable_v<Vector> && sizeof(Vector) == 16);

TEST(Kernel, TypedArithmetic)
{
    const Vector offset = Point(3, 4) - Point(1, 1);
    EXPECT_EQ(offset, Vector(2, 3));
    EXPECT_EQ(Point(1, 1) + offset, Point(3, 4));
    EXPECT_EQ(Point(3, 4) - offset, Point(1, 1));
}

TEST(Kernel, FramesAndMidpoint)
{
    EXPECT_EQ(offset_from_origin(Point(1, 2)), Vector(1, 2));
    EXPECT_EQ(at_offset(Vector(1, 2)), Point(1, 2));
    EXPECT_EQ(midpoint(Point(0, 0), Point(2, 4)), Point(1, 2));
}

TEST(Kernel, DefaultsAreZero)
{
    EXPECT_EQ(Point{}, Point(0, 0));
    EXPECT_EQ(Vector{}, Vector(0, 0));
    EXPECT_EQ(LineSegment{}.p1, Point(0, 0));
    EXPECT_EQ(LineSegment{}.p2, Point(0, 0));
}

TEST(Kernel, CgalAccessorsExposeStorage)
{
    const Point p{1, 2};
    const Vector v{3, 4};
    EXPECT_EQ(&p.cgal(), &p.cgal());
    EXPECT_EQ(p.cgal(), K::Point_2(1, 2));
    EXPECT_EQ(v.cgal(), K::Vector_2(3, 4));
    EXPECT_EQ(Point(K::Point_2(1, 2)), p);
    EXPECT_EQ(Vector(K::Vector_2(3, 4)), v);
}

TEST(Kernel, SignOfZeroIsPreserved)
{
    const Vector zero{0, 0};
    EXPECT_TRUE(std::signbit((-zero).x()));
    EXPECT_TRUE(std::signbit((-zero).y()));
    EXPECT_FALSE(std::signbit((Vector{} - zero).x()));
    EXPECT_FALSE(std::signbit((Vector{} - zero).y()));
}
