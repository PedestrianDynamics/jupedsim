#define BOOST_TEST_MODULE PointTest
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include "../geometry/Point.h"
#include <cmath>

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

BOOST_AUTO_TEST_CASE(POINT_TO_STRING_Test)
{
    BOOST_TEST_MESSAGE("starting string conv test");
    std::string xpt[] = {"0.25", "1.25", "2.25", "3.25"};
    std::string ypt[] = {"10.25", "11.25", "12.25", "13.25"};
    Point p1;
    for (double i = 0.25; i < 4; ++i)
    {
        p1.SetX(i);
        p1.SetY(i+10);
        BOOST_CHECK( p1.toString() == "( " + xpt[int(i)] + " : " + ypt[int(i)] + " )" );
    }
    
    Point p2(-2,-0.5);
    BOOST_CHECK( p2.toString() == "( -2 : -0.5 )" );
    BOOST_TEST_MESSAGE("Leaving string conv test");
}

BOOST_AUTO_TEST_CASE(POINT_NORM_MOLIFIED_TEST)
{
    BOOST_TEST_MESSAGE("starting NormMolified test");
    Point p;
    for (double i = 0, j = 0.5; i < 10; ++i, ++j)
    {
        p.SetX(i);
        p.SetY(j);
        BOOST_CHECK( p.NormMolified() == sqrt(pow(i,2) + pow(j,2) + 0.1) );
    }
    BOOST_TEST_MESSAGE("Leaving NormMolified test");
}

BOOST_AUTO_TEST_CASE(POINT_NORM_SQUARE_TEST)
{
    BOOST_TEST_MESSAGE("starting NormSquare test");
    Point p;
    for (double i = -5, j = 0.6; i < 5; ++i, ++j)
    {
        p.SetX(i);
        p.SetY(j);
        BOOST_CHECK( p.NormSquare() == pow(i,2) + pow(j,2) );
    }
    BOOST_TEST_MESSAGE("Leaving NormMSquare test");
}

BOOST_AUTO_TEST_CASE(POINT_NORMALIZE_TEST)
{
    BOOST_MESSAGE("starting Normalize test");
    Point p1(0.0001,0.0001);
    Point p2(10,10);
    p2 = p1.Normalized();
    BOOST_REQUIRE( p2.GetX() == 0.0 && p2.GetY() == 0.0 );
    
    for (double i = 0, j = -10; i < 5; ++i, ++j)
    {
        p1.SetX(i);
        p1.SetY(j);
        p2 = p1.Normalized();
        BOOST_REQUIRE( p2.GetX() == i / p1.Norm() && p2.GetY() == j / p1.Norm() );
        
        p2 = p1.NormalizedMolified();
        BOOST_REQUIRE( p2.GetX() == i / p1.NormMolified() && 
                       p2.GetY() == j / p1.NormMolified() );
        
    }
    
    
}
BOOST_AUTO_TEST_SUITE_END()