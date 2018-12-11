#define BOOST_TEST_MODULE LineTest
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include "../../geometry/Line.h"

#include <cmath>

BOOST_AUTO_TEST_SUITE(LineTest)

BOOST_AUTO_TEST_CASE(LINE_CONSTRUCTOR_TEST)
{
     BOOST_TEST_MESSAGE("starting Default constr test");
     Line L1;
     Point P1 = L1.GetPoint1();
     Point P2 = L1.GetPoint2();
     BOOST_REQUIRE(P1._x == 0 && P1._y == 0);
     BOOST_REQUIRE(P2._x == 0 && P2._y == 0);
     Line L2;
     BOOST_REQUIRE(L1.GetUniqueID() == 0 && L2.GetUniqueID() == 1);
     BOOST_TEST_MESSAGE("Leaving Default constr test");

     BOOST_TEST_MESSAGE("starting constr test");
     const double PI = 3.14159265358979323846;
     for (int i = -5, j = 2; i < 5; ++i, ++j)
     {
          i = (i == 0)? 1 : i;
          P1._x = (cos(PI / i));
          P1._y = (sin(PI * i));
          P2._x = (i);
          P2._y = (i * i);
          Line L3(P1, P2);
          Line L4(L3);
          Point P3 = L4.GetPoint1();
          Point P4 = L4.GetPoint2();
          BOOST_REQUIRE(P1._x == P3._x && P2._x == P4._x);
          BOOST_REQUIRE(P1._y == P3._y && P2._y == P4._y);
          BOOST_CHECK(j == L4.GetUniqueID());
     }
     BOOST_TEST_MESSAGE("Leaving constr test");
}

BOOST_AUTO_TEST_CASE(LINE_NORMAL_VEC_TEST)
{
     BOOST_TEST_MESSAGE("starting normal vector test");
     const double PI = 3.14159265358979323846;
     for (int i = -5; i < 5; ++i)
     {
          i = (i == 0)? 1 : i;
          Point P1 (PI / i, PI * i);
          Point P2 (i, sin(PI / i));
          Line L1 (P1, P2);
          Point normal = L1.NormalVec();
          Point diff = P2 - P1;
          BOOST_CHECK_MESSAGE(normal.ScalarProduct(diff) < 1E-12,
                              "normal.ScalarProduct(diff) = " << normal.ScalarProduct(diff) );
     }
     BOOST_TEST_MESSAGE("Leaving normal vector test");
}

BOOST_AUTO_TEST_CASE(LINE_SHORTEST_POINT_TEST)
{
     BOOST_TEST_MESSAGE("starting shortest point test");
     const double PI = 3.14159265358979323846;
     Point PA (-2, 4);
     Point PB (14, 9);
     Line L1 (PA, PB);
     const Point& DPAB = PA - PB;
     double lambda;
     for (float i = -20; i < 20; ++i)
     {
          i = (i == 0)? 0.5 : i;
          Point P1 (i, sin(PI / i));
          Point P2 = L1.ShortestPoint(P1);
          lambda = (P1 - PB).ScalarProduct(DPAB) / DPAB.ScalarProduct(DPAB);
          if (lambda > 1)
               BOOST_CHECK_MESSAGE(P2 == PA, " P2 = ( " << P2._x << ", " << P2._y << ")");
          else if(lambda < 0)
               BOOST_CHECK_MESSAGE(P2 == PB, " P2 = ( " << P2._x << ", " << P2._y << ")");
          else
               BOOST_CHECK_MESSAGE((P2 - P1).ScalarProduct(DPAB) < 1E-12,
                                   " P2 = ( " << P2._x << ", " << P2._y << ")");

     }
     BOOST_TEST_MESSAGE("Leaving shortest point test");
}

BOOST_AUTO_TEST_CASE(LINE_ISINLINESEGMENT_TEST)
{
     BOOST_TEST_MESSAGE("starting is_in_linesegment test");
     Line L1(Point(1,0), Point(10,0));
     for (int i = 1; i <= 10; ++i)
          BOOST_CHECK(L1.IsInLineSegment(Point(i, 0)));

     for (int i = 0; i < 20; ++i)
          BOOST_CHECK(!L1.IsInLineSegment(Point(i, i)));

     Point P1 (30.1379, 124.485);
     Point P2 (41.4647, 124.485);
     Point P3 (38.4046,104.715);
     Point P4 (33.7146,104.715);
     Line L2(P1,P2);
     Line L3(P3,P4);
     BOOST_CHECK(L2.IsInLineSegment(P3)==false);
     BOOST_CHECK(L2.IsInLineSegment(P4)==false);
     BOOST_CHECK(L3.IsInLineSegment(P1)==false);
     BOOST_CHECK(L3.IsInLineSegment(P2)==false);
     BOOST_TEST_MESSAGE("Leaving is_in_linesegment test");
}

BOOST_AUTO_TEST_CASE(LINE_DIST_TO_TEST)
{
     BOOST_TEST_MESSAGE("starting dist to shortestPoint test");
     Line L1(Point(-10, 2), Point(10, 2));
     for (int i = -10; i < 11; ++i)
     {
          BOOST_CHECK_MESSAGE(L1.DistTo(Point(i, i)) == abs(i-2), L1.DistTo(Point(i, i)));
          BOOST_CHECK_MESSAGE(L1.DistToSquare(Point(i, i)) == ((i-2)*(i-2)), L1.DistToSquare(Point(i, i)));
     }
     BOOST_TEST_MESSAGE("Leaving dist to shortestPoint test");

}

BOOST_AUTO_TEST_CASE(LINE_OPERATOR_TEST)
{
     BOOST_TEST_MESSAGE("starting operator overload test");
     const double PI = 3.14159265358979323846;
     for (int i = -5; i < 5; ++i)
     {
          i = (i == 0)? 1 : i;
          Point P1 (PI / i, PI * i);
          Point P2 (i, sin(PI / i));
          Line L1 (P1, P2);
          Line L2 (P1, P2);
          BOOST_CHECK(L1 == L2);
          Point P3 (i, i);
          Line L3 (P2, P3);
          BOOST_CHECK(L1 != L3);
     }
     BOOST_TEST_MESSAGE("Leaving operator overload test");
}

BOOST_AUTO_TEST_CASE(LINE_LENGTH_TEST)
{
     BOOST_TEST_MESSAGE("starting line length and length square test");
     Point P1;
     const double PI = 3.14159265358979323846;
     for (int i = -5; i < 5; ++i)
     {
          i = (i == 0)? 1 : i;
          Point P2 (i, sin(PI / i));
          Line L1 (P1, P2);
          double norm = P2.Norm();
          BOOST_CHECK(L1.Length() == norm);

          double normSq = P2.NormSquare();
          BOOST_CHECK(L1.LengthSquare() == normSq);
     }
     BOOST_TEST_MESSAGE("Leaving line length and length square test");
}

BOOST_AUTO_TEST_CASE(Line_Overlap_test)
{
        BOOST_TEST_MESSAGE("starting line overlap test");
        Point P1(0,0);
        Point P2(10,0);
        Line L1(P1,P2);
        for (int i = 0 ; i < 10; ++i)
        {
                Line L2(Point(i,0), Point(i+4,0));
                BOOST_CHECK(L1.Overlapp(L2) == true);

        }
        Line L3(Point(10,0), Point(15,0));
    BOOST_CHECK(L1.Overlapp(L3) == false);
        Line L4(Point(5,-5), Point(5,5));
        BOOST_CHECK(L1.Overlapp(L4) == false);
        Line L5(Point(-5,0), P1);
        BOOST_CHECK(L1.Overlapp(L5) == false);
        BOOST_TEST_MESSAGE("Leaving line overlap test");
}

BOOST_AUTO_TEST_CASE(Line_Intersection_test)
{
        BOOST_TEST_MESSAGE("starting line intersection test");
        Point P1(-1,0);
        Point P2(1,0);
        Line L1(P1,P2);
        const double Pi = 3.14159265358979323846;
        for (int i = 0; i <= 6 ; ++i)
        {
                Line L2(Point(0, -1), Point(cos(i*Pi/6), sin(i*Pi/6)));
                BOOST_CHECK(L1.IntersectionWith(L2) == true);
        }
        BOOST_CHECK(L1.IntersectionWith(Point(-1,-1), Point(-1,5)) == true);
        BOOST_CHECK(L1.IntersectionWith(Point(1,-5), Point(1, 0)) == true);

        BOOST_CHECK(L1.IntersectionWith(L1) == true);

    BOOST_CHECK(L1.IntersectionWith(Point(-1.0005, -1), Point(-1.0005, 1)) == false);
    BOOST_CHECK(L1.IntersectionWith(Point(-1, 0.00005), Point(1, 0.00005)) == false);

    Point P3;
    BOOST_CHECK(L1.IntersectionWith(Point(0.5, 1), Point(0.5, -1), P3) == true);
    BOOST_CHECK(P3 == Point(0.5, 0));
    BOOST_CHECK(L1.IntersectionWith(Point(-1, 1), Point(-1, 0), P3) == true);
    BOOST_CHECK(P3 == Point(-1, 0));
    BOOST_CHECK(L1.IntersectionWith(Point(-1.04, 1), Point(-1.04, -1), P3) == false);
    BOOST_CHECK(std::isnan(P3._x) && std::isnan(P3._y));

        BOOST_TEST_MESSAGE("Leaving line intersection test");
}

BOOST_AUTO_TEST_CASE(Line_hor_vert_test)
{
        BOOST_TEST_MESSAGE("starting line horizontal / vertical test");
        const double Pi = 3.14159265358979323846;

        for (int i = 0; i <= 12; ++i)
        {
                Line L1(Point(0, 0), Point(cos(i*Pi/12), sin(i*Pi/12)));
                if (i == 0 || i == 12)
                        BOOST_CHECK(L1.IsHorizontal() == true);

                else if(i == 6)
                        BOOST_CHECK(L1.IsVertical() == true);

                else{
                        BOOST_CHECK(L1.IsHorizontal() == false);
                        BOOST_CHECK(L1.IsVertical() == false);
                }
        }
        BOOST_TEST_MESSAGE("Leaving line horizontal / vertical test");
}

BOOST_AUTO_TEST_CASE(Line_whichSide_test)
{
        BOOST_TEST_MESSAGE("starting line which side test");
        Point Pleft(-2, 2);
        Point Pright(2, -2);
        const double Pi = 3.14159265358979323846;

        BOOST_CHECK(Line(Point(0,-2), Point(0,1)).WichSide(Pleft) == 0);
        BOOST_CHECK(Line(Point(0,-2), Point(0,1)).IsLeft(Pleft) == true);
        BOOST_CHECK(Line(Point(0,-2), Point(0,1)).WichSide(Pright) == 1);
        BOOST_CHECK(Line(Point(0,-2), Point(0,1)).IsLeft(Pright) == false);

        for (int i = 0; i <= 3; ++i) {    // including horizontal lines
                Line L1(Point(0, 0), Point(cos(i*Pi/6), sin(i*Pi/6)));

                BOOST_CHECK_MESSAGE(L1.WichSide(Pleft) == 0,"L1.WichSide(Pleft): " << L1.WichSide(Pleft) << " Pt:(-2,2), LINE: (0,0), (" <<
                        cos(i*Pi/6) << "," << sin(i*Pi/6) << ")");
        BOOST_CHECK_MESSAGE(L1.IsLeft(Pleft) == true, "L1.IsLeft(Pleft): " << L1.IsLeft(Pleft) << " Pt:(-2,2), LINE: (0,0), (" <<
                        cos(i*Pi/6) << "," << sin(i*Pi/6) << ")");

        BOOST_CHECK_MESSAGE(L1.WichSide(Pright) == 1, "L1.WichSide(Pright): " << L1.WichSide(Pright)<< " Pt:(2,2), LINE: (0,0), (" <<
                    cos(i*Pi/6) << "," << sin(i*Pi/6) << ")");
        BOOST_CHECK_MESSAGE(L1.IsLeft(Pright) == false, "L1.IsLeft(Pright): " << L1.IsLeft(Pright) << " Pt:(2,2), LINE: (0,0), (" <<
                    cos(i*Pi/6) << "," << sin(i*Pi/6) << ")");
        }

        BOOST_TEST_MESSAGE("starting line which side test");
}

BOOST_AUTO_TEST_CASE(LINE_NEARLYINLINESEGMENT_TEST)
{
     BOOST_TEST_MESSAGE("starting nearly_in_linesegment test");
     Line L1(Point(0, 6.27), Point(3.62, 6.27));
     Line L2(Point(3.62, 6.27), Point(3.62, 8));

     Point P1 (1.5, 6.3);
     Point P2 (2.3, 6.3);
     Point P3 (3.6, 6.6);
     Point P4 (3.6, 7.6);
     Line L3(P1,P2);
     Line L4(P3,P4);
     BOOST_CHECK(L1.NearlyInLineSegment(P1)==true);
     BOOST_CHECK(L1.NearlyInLineSegment(P2)==true);
     BOOST_CHECK(L2.NearlyInLineSegment(P3)==true);
     BOOST_CHECK(L2.NearlyInLineSegment(P4)==true);
     BOOST_TEST_MESSAGE("Leaving nearly_in_linesegment test");
}


BOOST_AUTO_TEST_SUITE_END()
