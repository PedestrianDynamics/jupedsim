
#define BOOST_TEST_MODULE LineTest
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include "../../geometry/Line.h"

#include <cmath>

BOOST_AUTO_TEST_SUITE(LineTest)

BOOST_AUTO_TEST_CASE(LINE_CONSTRUCTOR_TEST)
{
     BOOST_MESSAGE("starting Default constr test");
     Line L1;
     Point P1 = L1.GetPoint1();
     Point P2 = L1.GetPoint2();
     BOOST_REQUIRE(P1.GetX() == 0 && P1.GetY() == 0);
     BOOST_REQUIRE(P2.GetX() == 0 && P2.GetY() == 0);
     Line L2;
     BOOST_REQUIRE(L1.GetUniqueID() == 0 && L2.GetUniqueID() == 1);
     BOOST_MESSAGE("Leaving Default constr test");
     
     BOOST_MESSAGE("starting constr test");
     const double PI = 3.14159265358979323846;
     for (int i = -5, j = 2; i < 5; ++i, ++j)
     {
          i = (i == 0)? 1 : i;
          P1.SetX(cos(PI / i));
          P1.SetY(sin(PI * i));
          P2.SetX(i);
          P2.SetY(i * i);
          Line L3(P1, P2);
          Line L4(L3);
          Point P3 = L4.GetPoint1();
          Point P4 = L4.GetPoint2();
          BOOST_REQUIRE(P1.GetX() == P3.GetX() && P2.GetX() == P4.GetX());
          BOOST_REQUIRE(P1.GetY() == P3.GetY() && P2.GetY() == P4.GetY());
          BOOST_CHECK(j == L4.GetUniqueID());
     }
     BOOST_MESSAGE("Leaving constr test");
}

BOOST_AUTO_TEST_CASE(LINE_NORMAL_VEC_TEST)
{
     BOOST_MESSAGE("starting normal vector test");
     const double PI = 3.14159265358979323846;
     for (int i = -5; i < 5; ++i)
     {
          i = (i == 0)? 1 : i;
          Point P1 (PI / i, PI * i);
          Point P2 (i, sin(PI / i));
          Line L1 (P1, P2);
          Point normal = L1.NormalVec();
          Point diff = P2 - P1;
          BOOST_CHECK_MESSAGE(normal.ScalarP(diff) < 1E-12, 
                              "normal.ScalarP(diff) = " << normal.ScalarP(diff) );
     }
     BOOST_MESSAGE("Leaving normal vector test");
}

BOOST_AUTO_TEST_CASE(LINE_SHORTEST_POINT_TEST)
{
     BOOST_MESSAGE("starting shortest point test");
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
          lambda = (P1 - PB).ScalarP(DPAB) / DPAB.ScalarP(DPAB);
          if (lambda > 1)
               BOOST_CHECK_MESSAGE(P2 == PA, " P2 = ( " << P2.GetX() << ", " << P2.GetY() << ")");
          else if(lambda < 0)
               BOOST_CHECK_MESSAGE(P2 == PB, " P2 = ( " << P2.GetX() << ", " << P2.GetY() << ")");
          else
               BOOST_CHECK_MESSAGE((P2 - P1).ScalarP(DPAB) < 1E-12, 
                                   " P2 = ( " << P2.GetX() << ", " << P2.GetY() << ")");
       
     }
     BOOST_MESSAGE("Leaving shortest point test");
}

BOOST_AUTO_TEST_CASE(LINE_ISINLINESEGMENT_TEST)
{
     BOOST_MESSAGE("starting is_in_linesegment test");
     Line L1(Point(1,0), Point(10,0));
     for (int i = 1; i <= 10; ++i)
          BOOST_CHECK(L1.IsInLineSegment(Point(i, 0)));
     
     for (int i = 0; i < 20; ++i)
          BOOST_CHECK(!L1.IsInLineSegment(Point(i, i)));
  
     BOOST_MESSAGE("Leaving is_in_linesegment test");
}

BOOST_AUTO_TEST_CASE(LINE_DIST_TO_TEST)
{
     BOOST_MESSAGE("starting dist to shortestPoint test");
     Line L1(Point(-10, 2), Point(10, 2));
     for (int i = -10; i < 11; ++i)
     {
          BOOST_CHECK_MESSAGE(L1.DistTo(Point(i, i)) == abs(i-2), L1.DistTo(Point(i, i)));  
          BOOST_CHECK_MESSAGE(L1.DistToSquare(Point(i, i)) == abs(i-2)^2, L1.DistToSquare(Point(i, i)));  
     }
     BOOST_MESSAGE("Leaving dist to shortestPoint test");

}

BOOST_AUTO_TEST_CASE(LINE_OPERATOR_TEST)
{
     BOOST_MESSAGE("starting operator overload test");
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
     BOOST_MESSAGE("Leaving operator overload test");
}

BOOST_AUTO_TEST_CASE(LINE_LENGTH_TEST)
{
     BOOST_MESSAGE("starting line length test");
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
}

BOOST_AUTO_TEST_SUITE_END()