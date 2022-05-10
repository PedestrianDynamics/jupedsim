#include "Line.hpp"
#include "Point.hpp"

#include <gtest/gtest.h>

const double PI = acos(-1);

TEST(Line, DefaultCTor)
{
    Line l{};

    ASSERT_EQ(l.GetPoint1(), Point(0, 0));
    ASSERT_EQ(l.GetPoint2(), Point(0, 0));
}

TEST(Line, PointCTor)
{
    Point p1{1, 2};
    Point p2{3, 4};
    Line l{p1, p2};
    ASSERT_EQ(l.GetPoint1(), p1);
    ASSERT_EQ(l.GetPoint2(), p2);
}

TEST(Line, CopyCtor)
{
    Point p1{1, 2};
    Point p2{2, 3};
    Line copyMe{p1, p2};
    Line l{copyMe};
    ASSERT_EQ(l.GetPoint1(), p1);
    ASSERT_EQ(l.GetPoint2(), p2);
}

TEST(Line, ScalarProduct)
{
    for(int i : {-5, -4, -3, -2, -1, 1, 2, 3, 4}) {
        Point P1(PI / i, PI * i);
        Point P2(i, std::sin(PI / i));
        Line L1(P1, P2);
        Point normal = L1.NormalVec();
        Point diff = P2 - P1;
        ASSERT_NEAR(normal.ScalarProduct(diff), 0.0, 1.0e-12);
    }
}

TEST(Line, ShortestPoint)
{
    Point PA(-2, 4);
    Point PB(14, 9);
    Line L1(PA, PB);
    const Point& DPAB = PA - PB;
    for(float i = -20; i < 20; ++i) {
        i = (i == 0) ? 0.5 : i;
        Point P1(i, std::sin(PI / i));
        Point P2 = L1.ShortestPoint(P1);
        double lambda = (P1 - PB).ScalarProduct(DPAB) / DPAB.ScalarProduct(DPAB);
        if(lambda > 1) {
            ASSERT_EQ(P2, PA);
        } else if(lambda < 0) {
            ASSERT_EQ(P2, PB);
        } else {
            ASSERT_NEAR((P2 - P1).ScalarProduct(DPAB), 0.0, 1.0e-12);
        }
    }
}

TEST(Line, DistTo)
{
    Line L1(Point(-10, 2), Point(10, 2));
    for(int i = -10; i < 11; ++i) {
        ASSERT_DOUBLE_EQ(L1.DistTo(Point(i, i)), abs(i - 2));
        ASSERT_DOUBLE_EQ(L1.DistToSquare(Point(i, i)), (i - 2) * (i - 2));
    }
}

TEST(Line, Operators)
{
    for(int i : {-5, -4, -3, -2, -1, 1, 2, 3, 4}) {
        Point P1(PI / i, PI * i);
        Point P2(i, sin(PI / i));
        Line L1(P1, P2);
        Line L2(P1, P2);
        ASSERT_EQ(L1, L2);
        Point P3(i, i);
        Line L3(P2, P3);
        ASSERT_NE(L1, L3);
    }
}

TEST(Line, Length)
{
    Point P1;
    for(int i = -5; i < 5; ++i) {
        i = (i == 0) ? 1 : i;
        Point P2(i, sin(PI / i));
        Line L1(P1, P2);
        double norm = P2.Norm();
        ASSERT_EQ(L1.Length(), norm);

        double normSq = P2.NormSquare();
        ASSERT_EQ(L1.LengthSquare(), normSq);
    }
}

TEST(Line, Overlap)
{
    Point P1(0, 0);
    Point P2(10, 0);
    Line L1(P1, P2);
    for(int i = 0; i < 10; ++i) {
        Line L2(Point(i, 0), Point(i + 4, 0));
        ASSERT_TRUE(L1.Overlapp(L2));
    }

    Line L3(Point(10, 0), Point(15, 0));
    ASSERT_FALSE(L1.Overlapp(L3));
    Line L4(Point(5, -5), Point(5, 5));
    ASSERT_FALSE(L1.Overlapp(L4));
    Line L5(Point(-5, 0), P1);
    ASSERT_FALSE(L1.Overlapp(L5));
}

TEST(Line, Intersection)
{
    Point P1(-1, 0);
    Point P2(1, 0);
    Line L1(P1, P2);

    for(int i = 0; i <= 6; ++i) {
        Line L2(Point(0, -1), Point(cos(i * PI / 6), sin(i * PI / 6)));
        ASSERT_EQ(L1.IntersectionWith(L2), true);
    }

    ASSERT_TRUE(L1.IntersectionWith(Point(-1, -1), Point(-1, 5)));
    ASSERT_TRUE(L1.IntersectionWith(Point(1, -5), Point(1, 0)));
    ASSERT_TRUE(L1.IntersectionWith(L1));

    ASSERT_FALSE(L1.IntersectionWith(Point(-1.0005, -1), Point(-1.0005, 1)));
    ASSERT_FALSE(L1.IntersectionWith(Point(-1, 0.00005), Point(1, 0.00005)));

    Point P3;
    ASSERT_TRUE(L1.IntersectionWith(Point(0.5, 1), Point(0.5, -1), P3));
    ASSERT_TRUE(P3 == Point(0.5, 0));
    ASSERT_TRUE(L1.IntersectionWith(Point(-1, 1), Point(-1, 0), P3) == true);
    ASSERT_TRUE(P3 == Point(-1, 0));
    ASSERT_FALSE(L1.IntersectionWith(Point(-1.04, 1), Point(-1.04, -1), P3));
    ASSERT_TRUE(std::isnan(P3.x));
    ASSERT_TRUE(std::isnan(P3.y));
}

TEST(Line, VerticalHorizontal)
{
    for(int i = 0; i <= 12; ++i) {
        Line L1(Point(0, 0), Point(cos(i * PI / 12), sin(i * PI / 12)));
        if(i == 0 || i == 12) {
            ASSERT_TRUE(L1.IsHorizontal());
        } else if(i == 6) {
            ASSERT_TRUE(L1.IsVertical());
        } else {
            ASSERT_FALSE(L1.IsHorizontal());
            ASSERT_FALSE(L1.IsVertical());
        }
    }
}

TEST(Line, Side)
{
    Point Pleft(-2, 2);
    Point Pright(2, -2);

    ASSERT_TRUE(Line(Point(0, -2), Point(0, 1)).WichSide(Pleft) == 0);
    ASSERT_TRUE(Line(Point(0, -2), Point(0, 1)).IsLeft(Pleft));
    ASSERT_TRUE(Line(Point(0, -2), Point(0, 1)).WichSide(Pright) == 1);
    ASSERT_FALSE(Line(Point(0, -2), Point(0, 1)).IsLeft(Pright));

    for(int i = 0; i <= 3; ++i) { // including horizontal lines
        Line L1(Point(0, 0), Point(cos(i * PI / 6), sin(i * PI / 6)));

        ASSERT_TRUE(L1.WichSide(Pleft) == 0);
        ASSERT_TRUE(L1.IsLeft(Pleft));
        ASSERT_TRUE(L1.WichSide(Pright) == 1);
        ASSERT_FALSE(L1.IsLeft(Pright));
    }
}

TEST(Line, AlmostInLineSegment)
{
    Line L1(Point(0, 6.27), Point(3.62, 6.27));
    Line L2(Point(3.62, 6.27), Point(3.62, 8));

    Point P1(1.5, 6.3);
    Point P2(2.3, 6.3);
    Point P3(3.6, 6.6);
    Point P4(3.6, 7.6);
    Line L3(P1, P2);
    Line L4(P3, P4);
    ASSERT_TRUE(L1.NearlyInLineSegment(P1));
    ASSERT_TRUE(L1.NearlyInLineSegment(P2));
    ASSERT_TRUE(L2.NearlyInLineSegment(P3));
    ASSERT_TRUE(L2.NearlyInLineSegment(P4));
}

class LineCircleIntersection : public ::testing::Test
{
protected:
    Line line{{-1, -1}, {1, -1}};
    Point center{0.5, -1.0};
};

TEST_F(LineCircleIntersection, NoIntersections)
{
    std::vector<Point> intersections = line.IntersectionPointsWithCircle(center, 10);
    ASSERT_TRUE(intersections.empty());
}

TEST_F(LineCircleIntersection, OneIntersection)
{
    std::vector<Point> intersections = line.IntersectionPointsWithCircle(center, 1.);
    ASSERT_EQ(intersections.size(), 1);
    ASSERT_EQ(intersections[0], Point(-0.5, -1.0));
}

TEST_F(LineCircleIntersection, TwoIntersections)
{
    std::vector<Point> intersections = line.IntersectionPointsWithCircle(center, 0.5);
    ASSERT_EQ(intersections.size(), 2);
    ASSERT_EQ(intersections[0], Point(0, -1));
    ASSERT_EQ(intersections[1], Point(1, -1));
}
