#define BOOST_TEST_MODULE PointTest
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include "../geometry/Point.h"

BOOST_AUTO_TEST_SUITE(PointTest)

BOOST_AUTO_TEST_CASE(Point_Constructor_Test)
{
    BOOST_TEST_MESSAGE("Starting Point ctor test");
    Point p;
    BOOST_REQUIRE(p.GetX() == 0 && p.GetY() == 0);

    Point p2(1, 1);
    BOOST_REQUIRE(p2.GetX() == 1 && p2.GetY() == 1);

    Point p3(p2);
    BOOST_REQUIRE(p3.GetX() == 1 && p3.GetY() == 1);
    BOOST_TEST_MESSAGE("Leaving ctor test");
    BOOST_MESSAGE("Starting setter test");
    p2.SetX(0.5);
    p2.SetY(-232.2);
    BOOST_REQUIRE(p2.GetX() == 0.5 && p2.GetY() == -232.2);
    BOOST_TEST_MESSAGE("Leaving setter test");
}

BOOST_AUTO_TEST_CASE(Point_Norm_Tests)
{
    BOOST_TEST_MESSAGE("Starting norm test");
    Point p1(0,0);
    BOOST_CHECK(p1.Norm() == 0);
    p1.SetX(1);
    BOOST_CHECK(p1.Norm() == 1);
    p1.SetX(-1);
    BOOST_CHECK(p1.Norm() == 1);
    p1.SetX(3);
    BOOST_CHECK(p1.Norm() == 3);
    p1.SetX(0);
    p1.SetY(1);
    BOOST_CHECK(p1.Norm() == 1);
    p1.SetY(-1);
    BOOST_CHECK(p1.Norm() == 1);
    p1.SetY(3);
    BOOST_CHECK(p1.Norm() == 3);

    p1.SetX(3);
    p1.SetY(4);
    BOOST_CHECK(p1.Norm() == 5);
    BOOST_TEST_MESSAGE("Leaving norm test");
}

BOOST_AUTO_TEST_SUITE_END()