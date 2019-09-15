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

#include "pedestrian/Ellipse.h"

#include "math/Mathematics.h"

#include <catch2/catch.hpp>
#include <cmath>
#include <cstdlib>
#include <stdio.h>
#include <time.h>

TEST_CASE("pedestrian/Ellipse", "[pedestrian][Ellipse]")
{
    SECTION("Distance and Effective Distance between two Ellipses")
    {
        double dist, effdist;
        double a = 2.0, // semi-axis
            b    = 1.5; // orthogonal semi-axis
        JEllipse E1, E2;
        double   x2, y2; // E2 center

        E1.SetCenter(Point(0, 0));
        E1.SetV0(1);
        E1.SetV(Point(0, 0));
        E1.SetAmin(a);
        E1.SetBmax(b);

        SECTION("case 1")
        {
            x2 = 10;
            y2 = 0;
            E2.SetCenter(Point(x2, y2));
            E2.SetV0(1);
            E2.SetV(Point(0, 0));
            E2.SetAmin(a);
            E2.SetBmax(b);
            effdist = E1.EffectiveDistanceToEllipse(E2, &dist);

            REQUIRE(effdist == (fabs(x2) - 2 * a));
            REQUIRE(dist == fabs(x2));
        }

        SECTION("case 2")
        {
            x2 = -10;
            y2 = 0;
            E2.SetCenter(Point(x2, y2));
            E2.SetV0(1);
            E2.SetV(Point(0, 0));
            E2.SetAmin(a);
            E2.SetBmax(b);
            effdist = E1.EffectiveDistanceToEllipse(E2, &dist);

            REQUIRE(effdist == (fabs(x2) - 2 * a));
            REQUIRE(dist == fabs(x2));
        }

        SECTION("contact")
        {
            x2 = 2 * a;
            y2 = 0;
            E2.SetCenter(Point(x2, y2));
            E2.SetV0(1);
            E2.SetV(Point(0, 0));
            E2.SetAmin(a);
            E2.SetBmax(b);
            effdist = E1.EffectiveDistanceToEllipse(E2, &dist);

            REQUIRE(effdist == (fabs(x2) - 2 * a));
            REQUIRE(dist == fabs(x2));
        }

        SECTION("overlap")
        {
            x2 = 2 * a - 1.;
            y2 = 0;
            E2.SetCenter(Point(x2, y2));
            E2.SetV0(1);
            E2.SetV(Point(0, 0));
            E2.SetAmin(a);
            E2.SetBmax(b);
            effdist = E1.EffectiveDistanceToEllipse(E2, &dist);

            REQUIRE(effdist == (fabs(x2) - 2 * a));
            REQUIRE(dist == fabs(x2));
        }

        SECTION("y-axis")
        {
            x2 = 0.;
            y2 = 5;
            E2.SetCenter(Point(x2, y2));
            E2.SetV0(1);
            E2.SetV(Point(0, 0));
            E2.SetAmin(a);
            E2.SetBmax(b);
            effdist = E1.EffectiveDistanceToEllipse(E2, &dist);

            REQUIRE(effdist == (fabs(y2) - 2 * b));
            REQUIRE(dist == fabs(y2));
        }

        SECTION("y-axis 2")
        {
            x2 = 0.;
            y2 = -5;
            E2.SetCenter(Point(x2, y2));
            E2.SetV0(1);
            E2.SetV(Point(0, 0));
            E2.SetAmin(a);
            E2.SetBmax(b);
            effdist = E1.EffectiveDistanceToEllipse(E2, &dist);

            REQUIRE(effdist == (fabs(y2) - 2 * b));
            REQUIRE(dist == fabs(y2));
        }

        SECTION("y-axis 3")
        {
            x2 = 0.;
            y2 = 2 * b;
            E2.SetCenter(Point(x2, y2));
            E2.SetV0(1);
            E2.SetV(Point(0, 0));
            E2.SetAmin(a);
            E2.SetBmax(b);
            effdist = E1.EffectiveDistanceToEllipse(E2, &dist);

            REQUIRE(effdist == (fabs(y2) - 2 * b));
            REQUIRE(dist == fabs(y2));
        }

        SECTION("y-axis 4")
        {
            x2 = 0.;
            y2 = -2 * b;
            E2.SetCenter(Point(x2, y2));
            E2.SetV0(1);
            E2.SetV(Point(0, 0));
            E2.SetAmin(a);
            E2.SetBmax(b);
            effdist = E1.EffectiveDistanceToEllipse(E2, &dist);

            REQUIRE(effdist == (fabs(y2) - 2 * b));
            REQUIRE(dist == fabs(y2));
        }

        SECTION("y-axis overlap")
        {
            x2 = 0.;
            y2 = -2 * b + b;
            E2.SetCenter(Point(x2, y2));
            E2.SetV0(1);
            E2.SetV(Point(0, 0));
            E2.SetAmin(a);
            E2.SetBmax(b);
            effdist = E1.EffectiveDistanceToEllipse(E2, &dist);

            REQUIRE(effdist == (fabs(y2) - 2 * b));
            REQUIRE(dist == fabs(y2));
        }

        SECTION("y-axis overlap 2")
        {
            x2 = 0.;
            y2 = 2 * b - b;
            E2.SetCenter(Point(x2, y2));
            E2.SetV0(1);
            E2.SetV(Point(0, 0));
            E2.SetAmin(a);
            E2.SetBmax(b);
            effdist = E1.EffectiveDistanceToEllipse(E2, &dist);

            REQUIRE(effdist == (fabs(y2) - 2 * b));
            REQUIRE(dist == fabs(y2));
        }

        SECTION("total overlap")
        {
            x2 = 0.;
            y2 = 0;
            E2.SetCenter(Point(x2, y2));
            E2.SetV0(1);
            E2.SetV(Point(0, 0));
            E2.SetAmin(a);
            E2.SetBmax(b);
            effdist = E1.EffectiveDistanceToEllipse(E2, &dist);

            REQUIRE(effdist == (fabs(x2) - 2 * a));
            REQUIRE(dist == fabs(x2));
        }


        SECTION("total overlap 2")
        {
            x2 = 0.001;
            y2 = 0;
            E2.SetCenter(Point(x2, y2));
            E2.SetV0(1);
            E2.SetV(Point(0, 0));
            E2.SetAmin(a);
            E2.SetBmax(b);
            effdist = E1.EffectiveDistanceToEllipse(E2, &dist);

            REQUIRE(effdist == (fabs(x2) - 2 * a));
            REQUIRE(dist == fabs(x2));
        }
    }


    SECTION("Effective Distance between Ellipse and Line")
    {
        double a = 2.0, // semi-axis
            b    = 1.5; // orthogonal semi-axis

        JEllipse E;
        E.SetCenter(Point(0, 0));
        E.SetV0(1);
        E.SetV(Point(0, 0));
        E.SetAmin(a);
        E.SetBmax(b);

        SECTION("parallel y")
        {
            Point P1(2 * a, 0);
            Point P2(2 * a, 3);
            Line  L(P1, P2);

            REQUIRE(E.EffectiveDistanceToLine(L) == a);
        }

        SECTION("parallel y")
        {
            Point P1(-3 * a, 0);
            Point P2(-3 * a, 3);
            Line  L(P1, P2);

            REQUIRE(E.EffectiveDistanceToLine(L) == 2 * a);
        }

        SECTION("parallel x")
        {
            Point P1(0, 2 * b);
            Point P2(3, 2 * b);
            Line  L(P1, P2);

            REQUIRE(E.EffectiveDistanceToLine(L) == b);
        }

        SECTION("parallel x")
        {
            Point P1(0, -2 * b);
            Point P2(3, -2 * b);
            Line  L(P1, P2);

            REQUIRE(E.EffectiveDistanceToLine(L) == b);
        }
    }

    SECTION("GetArea of Ellipse")
    {
        const double pi = std::acos(-1);
        JEllipse     E;
        double       a = 2.0;
        double       b = 4.0;

        E.SetCenter(Point(0, 0));
        E.SetV0(1);
        E.SetV(Point(0, 0));
        E.SetAmin(a);
        E.SetBmax(b); // if v=0 then getEb is Bmax

        // TestCase 1
        REQUIRE(E.GetArea() == Approx(a * b * pi));

        // TestCase 2
        E.SetCenter(Point(10, -4));
        REQUIRE(E.GetArea() == Approx(a * b * pi));

        // TestCase 3
        E.SetCenter(Point(10, -4));
        E.SetAmin(b);
        E.SetBmax(a); // if v=0 then getEb is Bmax
        REQUIRE(E.GetArea() == Approx(a * b * pi));

        // TestCase 4
        E.SetCenter(Point(-12.2, 5.1));
        a *= 0.6;
        b *= 1.3;
        E.SetAmin(a);
        E.SetBmax(b); // if v=0 then getEb is Bmax
        REQUIRE(E.GetArea() == Approx(a * b * pi));
    }

    SECTION("Test if a Point is inside an Ellipse")
    {
        float    a = 2.0, b = 10.0;
        JEllipse E;
        // double px, py;
        E.SetCenter(Point(0, 0));
        E.SetV0(1);
        E.SetV(Point(0, 0));
        E.SetAmin(a);
        E.SetBmax(b);

        SECTION("Point Inside")
        {
            // P == E.center
            REQUIRE(E.IsInside(Point(0, 0)));

            // P ~ E.center
            REQUIRE(E.IsInside(Point(0, -0.3)));

            // P ~ E.center
            REQUIRE(E.IsInside(Point(-0.73, -0.3)));
        }

        SECTION("Point Outside")
        {
            // P  semi-axis
            REQUIRE_FALSE(E.IsInside(Point(a, 0)));

            // P  semi-axis
            REQUIRE_FALSE(E.IsInside(Point(-a, 0)));

            // P ~ semi-axis
            REQUIRE_FALSE(E.IsInside(Point(0, b)));

            // P ~ semi-axis
            REQUIRE_FALSE(E.IsInside(Point(0, -b)));

            // P outside
            REQUIRE_FALSE(E.IsInside(Point(a, -b)));

            // P outside
            REQUIRE_FALSE(E.IsInside(Point(2 * a, 3.1 * b)));

            // P outside
            REQUIRE_FALSE(E.IsInside(Point(3 * a, -3.5 * b)));

            // P outside
            REQUIRE_FALSE(E.IsInside(Point(-5 * a, -2 * b)));

            // P outside
            REQUIRE_FALSE(E.IsInside(Point(-1.1 * a, -1.1 * b)));
        }
    }

    SECTION("Test if a Point is on an Ellipse")
    {
        float    a = 2.0, b = 10.0;
        JEllipse E;

        E.SetCenter(Point(0, 0));
        E.SetV0(1);
        E.SetV(Point(0, 0));
        E.SetAmin(a);
        E.SetBmax(b);


        SECTION("Point Inside")
        {
            // P == E.center
            REQUIRE_FALSE(E.IsOn(Point(0, 0)));
            // P ~ E.center
            REQUIRE_FALSE(E.IsOn(Point(0, -0.3)));
            // P ~ E.center
            REQUIRE_FALSE(E.IsOn(Point(-0.73, -0.3)));
        }

        SECTION("Point On Ellipse")
        {
            // P  semi-axis
            REQUIRE(E.IsOn(Point(a, 0)));
            // P  semi-axis
            REQUIRE(E.IsOn(Point(-a, 0)));
            // P ~ semi-axis
            REQUIRE(E.IsOn(Point(0, b)));
            // P ~ semi-axis
            REQUIRE(E.IsOn(Point(0, -b)));
        }

        SECTION("Point Outside")
        {
            // P outside
            REQUIRE_FALSE(E.IsOn(Point(a, -b)));
            // P outside
            REQUIRE_FALSE(E.IsOn(Point(2 * a, 3.1 * b)));
            // P outside
            REQUIRE_FALSE(E.IsOn(Point(3 * a, -3.5 * b)));
            // P outside
            REQUIRE_FALSE(E.IsOn(Point(-5 * a, -2 * b)));
            // P outside
            REQUIRE_FALSE(E.IsOn(Point(-1.1 * a, -1.1 * b)));
        }
    }

    SECTION("Test If a Point is Outside of an Ellipse")
    {
        float    a = 2.0, b = 10.0;
        JEllipse E;
        Point    P;
        // double px, py;
        E.SetCenter(Point(0, 0));
        E.SetV0(1);
        E.SetV(Point(0, 0));
        E.SetAmin(a);
        E.SetBmax(b);

        SECTION("Point Inside")
        {
            // P == E.center
            REQUIRE_FALSE(E.IsOutside(Point(0, 0)));
            // P ~ E.center
            REQUIRE_FALSE(E.IsOutside(Point(0, -0.3)));
            // P ~ E.center
            REQUIRE_FALSE(E.IsOutside(Point(-0.73, -0.3)));
            // P  semi-axis
            REQUIRE_FALSE(E.IsOutside(Point(a, 0)));
            // P  semi-axis
            REQUIRE_FALSE(E.IsOutside(Point(-a, 0)));
            // P ~ semi-axis
            REQUIRE_FALSE(E.IsOutside(Point(0, b)));
            // P ~ semi-axis
            REQUIRE_FALSE(E.IsOutside(Point(0, -b)));
        }

        SECTION("Point Outside")
        {
            // P outside
            REQUIRE(E.IsOutside(Point(a, -b)));
            // P outside
            REQUIRE(E.IsOutside(Point(2 * a, 3.1 * b)));
            // P outside
            REQUIRE(E.IsOutside(Point(3 * a, -3.5 * b)));
            // P outside
            REQUIRE(E.IsOutside(Point(-5 * a, -2 * b)));
            // P outside
            REQUIRE(E.IsOutside(Point(-1.1 * a, -1.1 * b)));
        }
    }

    SECTION("Get Point on Ellipse")
    {
        double   r = 2.0;
        JEllipse E;
        E.SetCenter(Point(0, 0));
        E.SetV0(1);
        E.SetV(Point(0, 0));
        E.SetAmin(r);
        E.SetBmax(r);

        SECTION("Test #1")
        {
            for(unsigned int i = 0; i < 2; i++) {
                for(unsigned int j = 0; j < 2; j++) {
                    if(i == 0 && j == 0)
                        continue;

                    double sx = i * r;
                    double sy = j * r;

                    Point  S{sx, sy};
                    double l = S.Norm();

                    REQUIRE(E.PointOnEllipse(S) == Point(r * sx / l, r * sy / l));
                }
            }
        }

        SECTION("Test #2")
        {
            // test point in the ellipse
            double sx = 0.5 * r;
            double sy = 0.5 * r;

            Point S{sx, sy};
            REQUIRE(E.PointOnEllipse(Point(0.5 * r, 0.5 * r)) ==
                    Point(r * sx / S.Norm(), r * sy / S.Norm()));
        }

        SECTION("Test #3")
        {
            // point in the center
            E.SetCenter(Point(0.4, 1.78));
            Point T{E.GetCenter() + Point(r, 0)};

            REQUIRE(E.PointOnEllipse(Point(0, 0)) == T);
        }
    }
}
