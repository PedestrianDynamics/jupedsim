// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Ellipse.hpp"

#include <gtest/gtest.h>

class EllipseEllipseDistance : public ::testing::Test
{
protected:
    JEllipse E1{};
    const double a = 2.0; // semi-axis
    const double b = 1.5; // orthogonal semi-axis
    void SetUp() override
    {
        E1.SetCenter(Point(0, 0));
        E1.SetV0(1);
        E1.SetV(Point(0, 0));
        E1.SetAmin(a);
        E1.SetBmax(b);
    }
};

TEST_F(EllipseEllipseDistance, Case1)
{
    const double x2 = 10;
    const double y2 = 0;
    JEllipse E2{};
    E2.SetCenter(Point(x2, y2));
    E2.SetV0(1);
    E2.SetV(Point(0, 0));
    E2.SetAmin(a);
    E2.SetBmax(b);
    double dist{};
    const auto effdist = E1.EffectiveDistanceToEllipse(E2, &dist);

    ASSERT_EQ(effdist, (fabs(x2) - 2 * a));
    ASSERT_EQ(dist, fabs(x2));
}
TEST_F(EllipseEllipseDistance, Case2)
{
    const double x2 = -10;
    const double y2 = 0;
    JEllipse E2{};
    E2.SetCenter(Point(x2, y2));
    E2.SetV0(1);
    E2.SetV(Point(0, 0));
    E2.SetAmin(a);
    E2.SetBmax(b);
    double dist{};
    const auto effdist = E1.EffectiveDistanceToEllipse(E2, &dist);

    ASSERT_EQ(effdist, (fabs(x2) - 2 * a));
    ASSERT_EQ(dist, fabs(x2));
}

TEST_F(EllipseEllipseDistance, Contact)
{
    const double x2 = 2 * a;
    const double y2 = 0;
    JEllipse E2{};
    E2.SetCenter(Point(x2, y2));
    E2.SetV0(1);
    E2.SetV(Point(0, 0));
    E2.SetAmin(a);
    E2.SetBmax(b);
    double dist{};
    const auto effdist = E1.EffectiveDistanceToEllipse(E2, &dist);

    ASSERT_EQ(effdist, (fabs(x2) - 2 * a));
    ASSERT_EQ(dist, fabs(x2));
}

TEST_F(EllipseEllipseDistance, Overlap)
{
    const double x2 = 2 * a - 1.;
    const double y2 = 0;
    JEllipse E2{};
    E2.SetCenter(Point(x2, y2));
    E2.SetV0(1);
    E2.SetV(Point(0, 0));
    E2.SetAmin(a);
    E2.SetBmax(b);
    double dist{};
    const auto effdist = E1.EffectiveDistanceToEllipse(E2, &dist);

    ASSERT_EQ(effdist, (fabs(x2) - 2 * a));
    ASSERT_EQ(dist, fabs(x2));
}

TEST_F(EllipseEllipseDistance, YAxis)
{
    const double x2 = 0.;
    const double y2 = 5;
    JEllipse E2{};
    E2.SetCenter(Point(x2, y2));
    E2.SetV0(1);
    E2.SetV(Point(0, 0));
    E2.SetAmin(a);
    E2.SetBmax(b);
    double dist{};
    const auto effdist = E1.EffectiveDistanceToEllipse(E2, &dist);

    ASSERT_EQ(effdist, (fabs(y2) - 2 * b));
    ASSERT_EQ(dist, fabs(y2));
}

TEST_F(EllipseEllipseDistance, YAxis2)
{
    const double x2 = 0.;
    const double y2 = -5;
    JEllipse E2{};
    E2.SetCenter(Point(x2, y2));
    E2.SetV0(1);
    E2.SetV(Point(0, 0));
    E2.SetAmin(a);
    E2.SetBmax(b);
    double dist{};
    const auto effdist = E1.EffectiveDistanceToEllipse(E2, &dist);

    ASSERT_EQ(effdist, (fabs(y2) - 2 * b));
    ASSERT_EQ(dist, fabs(y2));
}

TEST_F(EllipseEllipseDistance, YAxis3)
{
    const double x2 = 0.;
    const double y2 = 2 * b;
    JEllipse E2{};
    E2.SetCenter(Point(x2, y2));
    E2.SetV0(1);
    E2.SetV(Point(0, 0));
    E2.SetAmin(a);
    E2.SetBmax(b);
    double dist{};
    const auto effdist = E1.EffectiveDistanceToEllipse(E2, &dist);

    ASSERT_EQ(effdist, (fabs(y2) - 2 * b));
    ASSERT_EQ(dist, fabs(y2));
}

TEST_F(EllipseEllipseDistance, YAxis4)
{
    const double x2 = 0.;
    const double y2 = -2 * b;
    JEllipse E2{};
    E2.SetCenter(Point(x2, y2));
    E2.SetV0(1);
    E2.SetV(Point(0, 0));
    E2.SetAmin(a);
    E2.SetBmax(b);
    double dist{};
    const auto effdist = E1.EffectiveDistanceToEllipse(E2, &dist);

    ASSERT_EQ(effdist, (fabs(y2) - 2 * b));
    ASSERT_EQ(dist, fabs(y2));
}

TEST_F(EllipseEllipseDistance, YAxisOverlap)
{
    const double x2 = 0.;
    const double y2 = -2 * b + b;
    JEllipse E2{};
    E2.SetCenter(Point(x2, y2));
    E2.SetV0(1);
    E2.SetV(Point(0, 0));
    E2.SetAmin(a);
    E2.SetBmax(b);
    double dist;
    const auto effdist = E1.EffectiveDistanceToEllipse(E2, &dist);

    ASSERT_EQ(effdist, (fabs(y2) - 2 * b));
    ASSERT_EQ(dist, fabs(y2));
}

TEST_F(EllipseEllipseDistance, YAxisOverlap2)
{
    const double x2 = 0.;
    const double y2 = 2 * b - b;
    JEllipse E2{};
    E2.SetCenter(Point(x2, y2));
    E2.SetV0(1);
    E2.SetV(Point(0, 0));
    E2.SetAmin(a);
    E2.SetBmax(b);
    double dist{};
    const auto effdist = E1.EffectiveDistanceToEllipse(E2, &dist);

    ASSERT_EQ(effdist, (fabs(y2) - 2 * b));
    ASSERT_EQ(dist, fabs(y2));
}

TEST_F(EllipseEllipseDistance, TotalOverlap)
{
    const double x2 = 0.;
    const double y2 = 0;
    JEllipse E2{};
    E2.SetCenter(Point(x2, y2));
    E2.SetV0(1);
    E2.SetV(Point(0, 0));
    E2.SetAmin(a);
    E2.SetBmax(b);
    double dist{};
    const auto effdist = E1.EffectiveDistanceToEllipse(E2, &dist);

    ASSERT_EQ(effdist, (fabs(x2) - 2 * a));
    ASSERT_EQ(dist, fabs(x2));
}

TEST_F(EllipseEllipseDistance, TotalOverlap2)
{
    const double x2 = 0.001;
    const double y2 = 0;
    JEllipse E2{};
    E2.SetCenter(Point(x2, y2));
    E2.SetV0(1);
    E2.SetV(Point(0, 0));
    E2.SetAmin(a);
    E2.SetBmax(b);
    double dist{};
    const auto effdist = E1.EffectiveDistanceToEllipse(E2, &dist);

    ASSERT_EQ(effdist, (fabs(x2) - 2 * a));
    ASSERT_EQ(dist, fabs(x2));
}

class EllipseLineSegmentDistance : public ::testing::Test
{
protected:
    JEllipse E{};
    const double a = 2.0; // semi-axis
    const double b = 1.5; // orthogonal semi-axis

    void SetUp() override
    {
        E.SetCenter(Point(0, 0));
        E.SetV0(1);
        E.SetV(Point(0, 0));
        E.SetAmin(a);
        E.SetBmax(b);
    };
};

TEST_F(EllipseLineSegmentDistance, ParallelY)
{
    Point P1(2 * a, 0);
    Point P2(2 * a, 3);
    Line L(P1, P2);

    ASSERT_EQ(E.EffectiveDistanceToLine(L), a);
}

TEST_F(EllipseLineSegmentDistance, ParallelY2)
{
    Point P1(-3 * a, 0);
    Point P2(-3 * a, 3);
    Line L(P1, P2);

    ASSERT_EQ(E.EffectiveDistanceToLine(L), 2 * a);
}

TEST_F(EllipseLineSegmentDistance, ParallelX)
{
    Point P1(0, 2 * b);
    Point P2(3, 2 * b);
    Line L(P1, P2);

    ASSERT_EQ(E.EffectiveDistanceToLine(L), b);
}

TEST_F(EllipseLineSegmentDistance, ParallelX2)
{
    Point P1(0, -2 * b);
    Point P2(3, -2 * b);
    Line L(P1, P2);

    ASSERT_EQ(E.EffectiveDistanceToLine(L), b);
}

class EllipseArea : public ::testing::Test
{
protected:
    const double pi = std::acos(-1);
    const double a = 2.0;
    const double b = 4.0;
    JEllipse E{};
    void SetUp() override
    {
        E.SetCenter(Point(0, 0));
        E.SetV0(1);
        E.SetV(Point(0, 0));
        E.SetAmin(a);
        E.SetBmax(b); // if v=0 then getEb is Bmax
    }
};

TEST_F(EllipseArea, Case1)
{
    ASSERT_DOUBLE_EQ(E.GetArea(), a * b * pi);
}

TEST_F(EllipseArea, Case2)
{
    E.SetCenter(Point(10, -4));
    ASSERT_DOUBLE_EQ(E.GetArea(), a * b * pi);
}

TEST_F(EllipseArea, Case3)
{
    E.SetCenter(Point(10, -4));
    E.SetAmin(b);
    E.SetBmax(a); // if v=0 then getEb is Bmax
    ASSERT_DOUBLE_EQ(E.GetArea(), a * b * pi);
}

TEST_F(EllipseArea, Case4)
{
    E.SetCenter(Point(-12.2, 5.1));
    const auto a1 = a * 0.6;
    const auto b1 = b * 1.3;
    E.SetAmin(a);
    E.SetBmax(b); // if v=0 then getEb is Bmax
    ASSERT_DOUBLE_EQ(E.GetArea(), a1 * b1 * pi);
}

TEST(Ellipse, ContainsPoint)
{
    const double a = 2.0;
    const double b = 10.0;
    JEllipse E;
    E.SetCenter(Point(0, 0));
    E.SetV0(1);
    E.SetV(Point(0, 0));
    E.SetAmin(a);
    E.SetBmax(b);
    // P == E.center
    ASSERT_TRUE(E.IsInside(Point(0, 0)));
    // P ~ E.center
    ASSERT_TRUE(E.IsInside(Point(0, -0.3)));
    // P ~ E.center
    ASSERT_TRUE(E.IsInside(Point(-0.73, -0.3)));
    // P  semi-axis
    ASSERT_FALSE(E.IsInside(Point(a, 0)));
    // P  semi-axis
    ASSERT_FALSE(E.IsInside(Point(-a, 0)));
    // P ~ semi-axis
    ASSERT_FALSE(E.IsInside(Point(0, b)));
    // P ~ semi-axis
    ASSERT_FALSE(E.IsInside(Point(0, -b)));
    // P outside
    ASSERT_FALSE(E.IsInside(Point(a, -b)));
    // P outside
    ASSERT_FALSE(E.IsInside(Point(2 * a, 3.1 * b)));
    // P outside
    ASSERT_FALSE(E.IsInside(Point(3 * a, -3.5 * b)));
    // P outside
    ASSERT_FALSE(E.IsInside(Point(-5 * a, -2 * b)));
    // P outside
    ASSERT_FALSE(E.IsInside(Point(-1.1 * a, -1.1 * b)));
};

TEST(Ellipse, PointOnEllipse)
{

    const double a = 2.0;
    const double b = 10.0;
    JEllipse E;

    E.SetCenter(Point(0, 0));
    E.SetV0(1);
    E.SetV(Point(0, 0));
    E.SetAmin(a);
    E.SetBmax(b);

    // P == E.center
    ASSERT_FALSE(E.IsOn(Point(0, 0)));
    // P ~ E.center
    ASSERT_FALSE(E.IsOn(Point(0, -0.3)));
    // P ~ E.center
    ASSERT_FALSE(E.IsOn(Point(-0.73, -0.3)));
    // P  semi-axis
    ASSERT_TRUE(E.IsOn(Point(a, 0)));
    // P  semi-axis
    ASSERT_TRUE(E.IsOn(Point(-a, 0)));
    // P ~ semi-axis
    ASSERT_TRUE(E.IsOn(Point(0, b)));
    // P ~ semi-axis
    ASSERT_TRUE(E.IsOn(Point(0, -b)));
    // P outside
    ASSERT_FALSE(E.IsOn(Point(a, -b)));
    // P outside
    ASSERT_FALSE(E.IsOn(Point(2 * a, 3.1 * b)));
    // P outside
    ASSERT_FALSE(E.IsOn(Point(3 * a, -3.5 * b)));
    // P outside
    ASSERT_FALSE(E.IsOn(Point(-5 * a, -2 * b)));
    // P outside
    ASSERT_FALSE(E.IsOn(Point(-1.1 * a, -1.1 * b)));
}
