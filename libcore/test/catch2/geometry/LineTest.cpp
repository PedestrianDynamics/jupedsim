/*
 * This file is part of JuPedSim.
 *
 * JuPedSim is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * JuPedSim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with JuPedSim. If not, see <http://www.gnu.org/licenses/>.
 *
 **/

#include "geometry/Line.h"

#include <catch2/catch.hpp>
#include <cmath>

const double PI = std::acos(-1);

TEST_CASE("geometry/Line", "[geometry][Line]")
{
    SECTION("Default ctor")
    {
        Line L1;
        REQUIRE(L1.GetPoint1()._x == 0);
        REQUIRE(L1.GetPoint1()._y == 0);
        REQUIRE(L1.GetPoint2()._x == 0);
        REQUIRE(L1.GetPoint2()._y == 0);
    }

    SECTION("Non Default ctor")
    {
        for(int i : {-5, -4, -3, -2, -1, 1, 2, 3, 4}) {
            Point P1(std::cos(PI / i), std::sin(PI * i));
            Point P2(i, i * i);

            Line L3(P1, P2);
            Line L4(L3);

            REQUIRE(P1 == L3.GetPoint1());
            REQUIRE(P2 == L3.GetPoint2());

            REQUIRE(P1 == L4.GetPoint1());
            REQUIRE(P2 == L4.GetPoint2());
        }
    }


    SECTION("Line Normal Vec")
    {
        for(int i : {-5, -4, -3, -2, -1, 1, 2, 3, 4}) {
            Point P1(PI / i, PI * i);
            Point P2(i, std::sin(PI / i));
            Line L1(P1, P2);
            Point normal = L1.NormalVec();
            Point diff   = P2 - P1;
            REQUIRE(normal.ScalarProduct(diff) == Approx(0).margin(1E-12));
        }
    }

    SECTION("Line Shortest Point")
    {
        Point PA(-2, 4);
        Point PB(14, 9);
        Line L1(PA, PB);
        const Point & DPAB = PA - PB;
        for(float i = -20; i < 20; ++i) {
            i = (i == 0) ? 0.5 : i;
            Point P1(i, std::sin(PI / i));
            Point P2      = L1.ShortestPoint(P1);
            double lambda = (P1 - PB).ScalarProduct(DPAB) / DPAB.ScalarProduct(DPAB);
            if(lambda > 1) {
                REQUIRE(P2 == PA);
            } else if(lambda < 0) {
                REQUIRE(P2 == PB);
            } else {
                REQUIRE((P2 - P1).ScalarProduct(DPAB) == Approx(0).margin(1E-12));
            }
        }
    }


    SECTION("Line Dist To")
    {
        Line L1(Point(-10, 2), Point(10, 2));
        for(int i = -10; i < 11; ++i) {
            REQUIRE(L1.DistTo(Point(i, i)) == Approx(abs(i - 2)));
            REQUIRE(L1.DistToSquare(Point(i, i)) == Approx((i - 2) * (i - 2)));
        }
    }

    SECTION("Line Operators")
    {
        for(int i : {-5, -4, -3, -2, -1, 1, 2, 3, 4}) {
            Point P1(PI / i, PI * i);
            Point P2(i, sin(PI / i));
            Line L1(P1, P2);
            Line L2(P1, P2);
            REQUIRE(L1 == L2);
            Point P3(i, i);
            Line L3(P2, P3);
            REQUIRE(L1 != L3);
        }
    }

    SECTION("Line Length")
    {
        Point P1;
        for(int i = -5; i < 5; ++i) {
            i = (i == 0) ? 1 : i;
            Point P2(i, sin(PI / i));
            Line L1(P1, P2);
            double norm = P2.Norm();
            REQUIRE(L1.Length() == norm);

            double normSq = P2.NormSquare();
            REQUIRE(L1.LengthSquare() == normSq);
        }
    }

    SECTION("Line Overlap")
    {
        Point P1(0, 0);
        Point P2(10, 0);
        Line L1(P1, P2);
        for(int i = 0; i < 10; ++i) {
            Line L2(Point(i, 0), Point(i + 4, 0));
            REQUIRE(L1.Overlapp(L2));
        }

        Line L3(Point(10, 0), Point(15, 0));
        REQUIRE_FALSE(L1.Overlapp(L3));
        Line L4(Point(5, -5), Point(5, 5));
        REQUIRE_FALSE(L1.Overlapp(L4));
        Line L5(Point(-5, 0), P1);
        REQUIRE_FALSE(L1.Overlapp(L5));
    }

    SECTION("Line Intersection")
    {
        Point P1(-1, 0);
        Point P2(1, 0);
        Line L1(P1, P2);

        for(int i = 0; i <= 6; ++i) {
            Line L2(Point(0, -1), Point(cos(i * PI / 6), sin(i * PI / 6)));
            REQUIRE(L1.IntersectionWith(L2) == true);
        }

        REQUIRE(L1.IntersectionWith(Point(-1, -1), Point(-1, 5)));
        REQUIRE(L1.IntersectionWith(Point(1, -5), Point(1, 0)));
        REQUIRE(L1.IntersectionWith(L1));

        REQUIRE_FALSE(L1.IntersectionWith(Point(-1.0005, -1), Point(-1.0005, 1)));
        REQUIRE_FALSE(L1.IntersectionWith(Point(-1, 0.00005), Point(1, 0.00005)));


        Point P3;
        REQUIRE(L1.IntersectionWith(Point(0.5, 1), Point(0.5, -1), P3));
        REQUIRE(P3 == Point(0.5, 0));
        REQUIRE(L1.IntersectionWith(Point(-1, 1), Point(-1, 0), P3) == true);
        REQUIRE(P3 == Point(-1, 0));
        REQUIRE_FALSE(L1.IntersectionWith(Point(-1.04, 1), Point(-1.04, -1), P3));
        REQUIRE(std::isnan(P3._x));
        REQUIRE(std::isnan(P3._y));
    }

    SECTION("Line Horizontal Vertical")
    {
        for(int i = 0; i <= 12; ++i) {
            Line L1(Point(0, 0), Point(cos(i * PI / 12), sin(i * PI / 12)));
            if(i == 0 || i == 12) {
                REQUIRE(L1.IsHorizontal());
            } else if(i == 6) {
                REQUIRE(L1.IsVertical());
            } else {
                REQUIRE_FALSE(L1.IsHorizontal());
                REQUIRE_FALSE(L1.IsVertical());
            }
        }
    }

    SECTION("Line Which Side")
    {
        Point Pleft(-2, 2);
        Point Pright(2, -2);


        REQUIRE(Line(Point(0, -2), Point(0, 1)).WichSide(Pleft) == 0);
        REQUIRE(Line(Point(0, -2), Point(0, 1)).IsLeft(Pleft));
        REQUIRE(Line(Point(0, -2), Point(0, 1)).WichSide(Pright) == 1);
        REQUIRE_FALSE(Line(Point(0, -2), Point(0, 1)).IsLeft(Pright));

        for(int i = 0; i <= 3; ++i) { // including horizontal lines
            Line L1(Point(0, 0), Point(cos(i * PI / 6), sin(i * PI / 6)));

            REQUIRE(L1.WichSide(Pleft) == 0);
            REQUIRE(L1.IsLeft(Pleft));
            REQUIRE(L1.WichSide(Pright) == 1);
            REQUIRE_FALSE(L1.IsLeft(Pright));
        }
    }

    SECTION("Line almost in LineSegment")
    {
        Line L1(Point(0, 6.27), Point(3.62, 6.27));
        Line L2(Point(3.62, 6.27), Point(3.62, 8));

        Point P1(1.5, 6.3);
        Point P2(2.3, 6.3);
        Point P3(3.6, 6.6);
        Point P4(3.6, 7.6);
        Line L3(P1, P2);
        Line L4(P3, P4);
        REQUIRE(L1.NearlyInLineSegment(P1));
        REQUIRE(L1.NearlyInLineSegment(P2));
        REQUIRE(L2.NearlyInLineSegment(P3));
        REQUIRE(L2.NearlyInLineSegment(P4));
    }
}

TEST_CASE("geometry/Line/IsInLineSegment", "[geometry][Line][IsInLineSegment]")
{
    Line line({-1.5, 1.2}, {1.6, -2.});
    std::random_device rd;
    std::mt19937 mt(rd());

    SECTION("Is on line segment")
    {
        std::uniform_real_distribution<double> dist(std::numeric_limits<double>::epsilon(), 1);
        for(int i = 0; i < 100; ++i) {
            Point connection(
                {line.GetPoint2()._x - line.GetPoint1()._x,
                 line.GetPoint2()._y - line.GetPoint1()._y});
            Point p = line.GetPoint1() + (connection * dist(mt));
            REQUIRE(line.IsInLineSegment(p));
        }
    }

    SECTION("Is on line segment")
    {
        std::uniform_real_distribution<double> dist(std::numeric_limits<double>::epsilon(), 1);
        for(int i = 0; i < 100; ++i) {
            Point connection(
                {line.GetPoint2()._x - line.GetPoint1()._x,
                 line.GetPoint2()._y - line.GetPoint1()._y});
            Point p = line.GetPoint1() + (connection * dist(mt));
            p._x += J_EPS / 5.;
            p._y += J_EPS / 5.;

            REQUIRE_FALSE(line.IsInLineSegment(p));
        }
    }

    SECTION("old test from bug")
    {
        Line L1(Point(1, 0), Point(10, 0));
        for(int i = 1; i <= 10; ++i) {
            REQUIRE(L1.IsInLineSegment(Point(i, 0)));
        }

        for(int i = 0; i < 20; ++i) {
            REQUIRE_FALSE(L1.IsInLineSegment(Point(i, i)));
        }

        Point P1(30.1379, 124.485);
        Point P2(41.4647, 124.485);
        Point P3(38.4046, 104.715);
        Point P4(33.7146, 104.715);
        Line L2(P1, P2);
        Line L3(P3, P4);
        REQUIRE_FALSE(L2.IsInLineSegment(P3));
        REQUIRE_FALSE(L2.IsInLineSegment(P4));
        REQUIRE_FALSE(L3.IsInLineSegment(P1));
        REQUIRE_FALSE(L3.IsInLineSegment(P2));
    }
}
