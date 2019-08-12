/**
 * \file        testClassLine.cpp
 * \date        April 18, 2015
 * \version     v0.7
 * \copyright   <2009-2015> Forschungszentrum Jülich GmbH. All rights reserved.
 *
 * \section License
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
 * \section Description
 *
 *
 **/
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_NO_MAIN
#include <boost/test/unit_test.hpp>
#include "../../geometry/Point.h"
#include <cmath>

BOOST_AUTO_TEST_SUITE(PointTest)

BOOST_AUTO_TEST_CASE(Point_Constructor_Test)
{
    BOOST_TEST_MESSAGE("Starting Point ctor test");
    Point p;
    BOOST_REQUIRE(p._x == 0 && p._y == 0);

    Point p2(1, 1);
    BOOST_REQUIRE(p2._x == 1 && p2._y == 1);

    Point p3(p2);
    BOOST_REQUIRE(p3._x == 1 && p3._y == 1);
    BOOST_TEST_MESSAGE("Leaving ctor test");
    BOOST_TEST_MESSAGE("Starting setter test");
    p2._x = (0.5);
    p2._y = (-232.2);
    BOOST_REQUIRE(p2._x == 0.5 && p2._y == -232.2);
    BOOST_TEST_MESSAGE("Leaving setter test");
}

BOOST_AUTO_TEST_CASE(Point_Norm_Tests)
{
    BOOST_TEST_MESSAGE("Starting norm test");
    Point p1(0,0);
    BOOST_CHECK(p1.Norm() == 0);
    p1._x = (1);
    BOOST_CHECK(p1.Norm() == 1);
    p1._x = (-1);
    BOOST_CHECK(p1.Norm() == 1);
    p1._x = (3);
    BOOST_CHECK(p1.Norm() == 3);
    p1._x = (0);
    p1._y = (1);
    BOOST_CHECK(p1.Norm() == 1);
    p1._y = (-1);
    BOOST_CHECK(p1.Norm() == 1);
    p1._y = (3);
    BOOST_CHECK(p1.Norm() == 3);

    p1._x = (3);
    p1._y = (4);
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
        p1._x = (i);
        p1._y = (i+10);
        BOOST_CHECK( p1.toString() == "( " + xpt[int(i)] +
                                     " : " + ypt[int(i)] + " )" );
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
        p._x = (i);
        p._y = (j);
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
        p._x = (i);
        p._y = (j);
        BOOST_CHECK( p.NormSquare() == pow(i,2) + pow(j,2) );
    }
    BOOST_TEST_MESSAGE("Leaving NormMSquare test");
}

BOOST_AUTO_TEST_CASE(POINT_NORMALIZE_TEST)
{
    BOOST_TEST_MESSAGE("starting Normalize test");
    Point p1(0.0001,0.0001);
    Point p2(10,10);
    p2 = p1.Normalized();
    BOOST_TEST_MESSAGE(" check for norm < J_EPS ");
    BOOST_REQUIRE( p2._x == 0.0 && p2._y == 0.0 );

    for (double i = 0, j = -10; i < 5; ++i, ++j)
    {
        p1._x = (i);
        p1._y = (j);
        p2 = p1.Normalized();
        BOOST_TEST_MESSAGE(" check for norm > J_EPS ");
        BOOST_REQUIRE( p2._x == i / p1.Norm() &&
                       p2._y == j / p1.Norm() );

        p2 = p1.NormalizedMolified();
        BOOST_TEST_MESSAGE(" check for norm > J_EPS_GOAL ");
        BOOST_REQUIRE( p2._x == i / p1.NormMolified() &&
                       p2._y == j / p1.NormMolified() );

    }
    BOOST_TEST_MESSAGE("Leaving normalize test");
}

BOOST_AUTO_TEST_CASE(POINT_DET_TEST)
{
    BOOST_TEST_MESSAGE("starting determinant test");
    const double PI = 3.14159265358979323846;
    Point p1(10,5);
    Point p2;
    for (int i = 1; i < 5; ++i)
    {
        p2._x = ( cos(PI / -i) );
        p2._y = ( sin(PI / i) );
        BOOST_CHECK(p1.Determinant(p2) == 10 * sin(PI/i) - 5 * cos(PI/i));
        //BOOST_CHECK( p1.Det(p2) == p1.CrossProduct(p2));
    }
    BOOST_TEST_MESSAGE("Leaving determinant test");
}

BOOST_AUTO_TEST_CASE(POINT_SCALARPRODUCT_TEST)
{
    BOOST_TEST_MESSAGE("starting scalar product test");
    const double PI = 3.14159265358979323846;
    Point p1(10,5);
    Point p2;
    for (int i = 1; i < 5; ++i)
    {
        p2._x = ( cos(PI /  i) );
        p2._y = ( sin(PI / -i) );
        BOOST_CHECK(p1.ScalarProduct(p2) == 10 * p2._x + 5 * p2._y );
    }
    BOOST_TEST_MESSAGE("Leaving scalar product test");
}

BOOST_AUTO_TEST_CASE(POINT_OPEROVERLOADING_TEST)
{
    BOOST_TEST_MESSAGE("starting operator overload test");
    const double PI = 3.14159265358979323846;
    for (int i = 1; i < 10; ++i)
    {
        Point p1(i, -i*10);
        Point p2( cos(PI/i), sin(PI/i) );

        Point sum = p1 + p2;
        BOOST_REQUIRE( sum._x == i + cos(PI/i) &&
                       sum._y == -i*10 + sin(PI/i) );
        Point sub = p1 - p2;
        BOOST_REQUIRE( sub._x == i - cos(PI/i) &&
                       sub._y == -i*10 - sin(PI/i) );
        Point mul = p2 * i;
        BOOST_REQUIRE( mul._x == i * cos(PI/i) &&
                       mul._y == i * sin(PI/i) );
        Point pluseq(i, i);
        pluseq += p1;
        BOOST_REQUIRE( pluseq._x == i + i &&
                       pluseq._y == -i*10 + i );
        Point div = p2 / 1E-7;
        BOOST_REQUIRE( div._x == p2._x &&
                       div._y == p2._y );
        div = p2 / 2;
        BOOST_REQUIRE( div._x == p2._x / 2 &&
                       div._y == p2._y / 2 );

        BOOST_CHECK( p1 != p2 );
        p1._x = ( p2._x );
        p1._y = ( p2._y );
        BOOST_CHECK( p1 == p2 );
    }
    BOOST_TEST_MESSAGE("Leaving operator overload test");
}

BOOST_AUTO_TEST_CASE(POINT_COORDTRANS_TO_ELLIPSE_TEST)
{
    BOOST_TEST_MESSAGE("starting coord transform to ellipse");
    const double PI = 3.14159265358979323846;
    for (int i = 1; i < 5; ++i)
    {
        Point p1(i*10, i/10);
        Point center(i, -i);
        Point check = (p1 - center).Rotate( cos(PI/i), -sin(PI/i) );
        Point transform = p1.TransformToEllipseCoordinates(center, cos(PI / i), sin(PI / i));
        BOOST_CHECK( transform == check );
    }
    BOOST_TEST_MESSAGE("Leaving coord transform to ellipse");
}

BOOST_AUTO_TEST_CASE(POINT_COORDTRANS_TO_CART_TEST)
{
    BOOST_TEST_MESSAGE("starting coord transform to cartesian");
    const double PI = 3.14159265358979323846;
    for (int i = 1; i < 5; ++i)
    {
        Point p1(i*10, i/10);
        Point center(-i, i);
        Point transform = p1.TransformToCartesianCoordinates(center, cos(PI / i), sin(PI / i));
        Point check = p1.Rotate( cos(PI/i), sin(PI/i) );
        BOOST_REQUIRE( transform._x == check._x - i &&
                       transform._y == check._y + i );
    }
    BOOST_TEST_MESSAGE("starting coord transform to cartesian");
}

BOOST_AUTO_TEST_SUITE_END()
