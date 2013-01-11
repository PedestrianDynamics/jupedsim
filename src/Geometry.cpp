/*
 * Geometry.cpp
 *
 *  Created on: Apr 29, 2010
 *      Author: ZhangJun
 */

#include "Geometry.h"

#include <algorithm> // for reverse, unique
#include <iostream>
#include <string>




Geometry::Geometry() {
	// TODO Auto-generated constructor stub

}

Geometry::~Geometry() {
	// TODO Auto-generated destructor stub
}

// length is in x-direction, width is in y-direction

/* this function is to generate a scenario of bottleneck
 * output: data type is polygon_2d in boost geometry library
 * input: the coordinate of vertex of the scenario
 * note that: the first and last data should be the same to make the polygon closed.
 */
polygon_2d Geometry::bottleneck(float width, float length)
{


    // Define a polygon and fill the outer ring.
    // In most cases you will read it from a file or database
    polygon_2d poly;
    {
        const double coor[][2] = {
            {-350.0,-500.0}, {350,-500}, {350, 0}, {width/2.0, 0}, {width/2.0, length},
            {350, length}, {350, 1000}, {-350, 1000}, {-350, length}, {-1.0*width/2.0, length}, {-1.0*width/2.0,0},
            {-350, 0}, {-350.0,-500.0} // closing point is opening point
            };
        assign_points(poly, coor);
    }
    // Polygons should be closed, and directed clockwise. If you're not sure if that is the case,
    // call the correct algorithm
    correct(poly);
    return poly;





}

// #include <boost/polygon/polygon.hpp>
// #include <cassert>
// namespace gtl = boost::polygon;
// using namespace boost::polygon::operators;

polygon_2d Geometry::Tawaf()
{
  float f=1.0; //100 ;  //cm
  //namespace gtl = boost::polygon;
  polygon_2d poly;
    {
        const double coor[][2] =
	// {
	//   {64.650398*f,  -46.793868*f},
	//   {-43.943486*f,  -55.404077*f},
	//   {-70.240993*f,  -25.253470*f},
	//   {-72.210723*f,  23.220107*f},
	//   {-49.214346*f,  46.046596*f},
	//   {64.446852*f,  54.796826*f},
	//   {89.281770*f,  41.467866*f},
	//   {90.714977*f,  -9.748546*f},
	//   {64.650398*f,  -46.793868*f}
	// };

	{
	  {64.650398*f,  -46.793868*f},
	    {90.714977*f,  -9.748546*f},
	      {89.281770*f,  41.467866*f},
		{64.446852*f,  54.796826*f},
		  {-49.214346*f,  46.046596*f},
		    {-72.210723*f,  23.220107*f},
		      {-70.240993*f,  -25.253470*f},
			{-43.943486*f,  -55.404077*f},
          {64.650398*f,  -46.793868*f}
	};
        assign_points(poly, coor);
    }

    // Polygons should be closed, and directed clockwise. If you're not sure if that is the case,
    // call the correct algorithm
    correct(poly);
    
    //

    // // Polygon  have one inner rings
    // {
    //     poly.inners().resize(1);
    //     linear_ring<point_2d>& inner = poly.inners().back(); //poly 0
    polygon_2d inner;
         const double coorK[][2] = {
    	  {-1.256993*f,  14.919376*f}, {-2.259613*f,  14.578646*f}, {-3.183632*f,  14.025337*f},
    	  {-3.993542*f,  13.280705*f}, {-4.658218*f,  12.373369*f}, {-5.152117*f,  11.338194*f},
    	  {-5.456258*f,  10.214966*f}, {-5.558954*f,  9.046849*f},  {-5.952690*f,  6.845513*f},
    	  {-5.952690*f,  -6.237194*f}, {-5.414931*f,  -6.774953*f}, {5.460947*f,  -6.774953*f},
    	  {5.998705*f,  -6.237194*f},  {5.998705*f,  6.845513*f},   {5.130350*f,  9.046850*f},
    	  {5.027654*f,  10.214968*f},  {4.723513*f,  11.338196*f},  {4.229613*f,  12.373369*f},
    	  {3.564937*f,  13.280704*f},  {2.755027*f,  14.025336*f},  {1.831006*f,  14.578646*f},
    	  {0.828386*f,  14.919376*f},  {-0.214303*f,  15.034424*f}, {-1.256993*f,  14.919376*f}

    	};
        assign_points(inner, coorK);
        correct(inner);
        std::cout << "area Kaaba= "<<(area(inner)) <<"\n";

        polygon_2d raum0, raum1, raum2, raum3;
        
        const double coor0[][2] = {
          {64.650398*f,  -46.793868*f},
	    {90.714977*f,  -9.748546*f},
	      {89.281770*f,  41.467866*f},
		{64.446852*f,  54.796826*f},
          {6.00,    6.85},{ 6.00  ,  -6.24},
          {64.650398*f,  -46.793868*f}
        };
        const double coor1[][2] = {
          {6.00,  6.85},   
          { 64.45,    54.80},
          {-49.21,  46.05},
 {-5.95 ,   6.85},  {-5.56,    9.05}, 
 {-5.46,    10.21}, {-5.15,    11.34},	{-4.66,    12.37}, 
{-3.99 ,   13.28},  {-3.18,    14.03}, {-2.26,    14.58},  
{-1.26,    14.92},  {-0.21 ,   15.03},{ 0.83,    14.92},   
 {1.83,    14.58}, {2.76,    14.03},    {3.56,    13.28},  {4.23,    12.37},  {4.72,    11.34},{5.03,    10.21},{5.13,    9.05},
    {6.00,  6.85}
        };
         const double coor2[][2] = {
          { -43.94,  -55.40}, {-5.95,    -6.24},
          {-5.95,  6.85}, { -49.21,    46.05},
          { -72.21 ,   23.22}, {-70.24 ,   -25.25},
          { -43.94,  -55.40}
          
        };
       const double coor3[][2] = {
          { -43.94,    -55.40}, {   64.65 , -46.79},  
          { 6.00  ,  -6.24},  { 5.46  ,  -6.77},
           { -5.41 ,   -6.77}, {   -5.95 , -6.24},
      
          { -43.94,    -55.40}
        };

        assign_points(raum0, coor0);
        correct(raum0);
        assign_points(raum1, coor1);
        correct(raum1);
        assign_points(raum2, coor2);
        correct(raum2);
        assign_points(raum3, coor3);
        correct(raum3);
/*        std::cout << "area Raum0= "<<area(raum0) <<std::endl;

        std::cout << "area Raum1= "<<area(raum1) <<std::endl;

        std::cout << "area Raum2= "<<area(raum2) <<std::endl;

        std::cout << "area Raum3= "<<area(raum3) <<std::endl;
        std::cout << "Sum rooms= "<<area(raum0)+area(raum1)+area(raum2)+area(raum3) <<std::endl;
        std::cout << "Sum rooms+Kaaba= "<<area(raum0)+area(raum1)+area(raum2)+area(raum3)+area(inner) <<std::endl;
        std::cout << "area Alles= "<<area(poly) << "  ERROR="<< area(poly)-(area(raum0)+area(raum1)+area(raum2)+area(raum3)+area(inner)) <<std::endl;*/

    // }

    // correct(poly);

    //gtl::scale_up(poly, 100);
    
    return poly;

}





//---------------------------------------------------------------------------------
/* this function is to generate a T-shape scenario
 * output: data type is polygon_2d in boost geometry library
 * input: the coordinate of vertex of the scenario
 * note that: the first and last data should be the same to make the polygon closed.
 */
polygon_2d Geometry::Tshape()
{


    // Define a polygon and fill the outer ring.
    // In most cases you will read it from a file or database
	    polygon_2d poly;
    {
        const double coor[][2] = {
            {-500.0,-240.0}, {-500,0}, {-240, 0}, {-240, 450}, {0, 450},
            {0, 0}, {400, 0}, {400, -240}, {-500.0,-240.0} // closing point is opening point
            };
        assign_points(poly, coor);
    }
/*
    polygon_2d poly;
    {
        const double coor[][2] = {
            {-500.0,-300.0}, {-500,0}, {-300, 0}, {-300, 430}, {0, 430},
            {0, 0}, {400, 0}, {400, -300}, {-500.0,-300.0} // closing point is opening point
            };
        assign_points(poly, coor);
    }
*/
    // Polygons should be closed, and directed clockwise. If you're not sure if that is the case,
    // call the correct algorithm
    correct(poly);
    return poly;

}


polygon_2d Geometry::Corner(float b)
{
 polygon_2d poly;
    {
      const double coor[][2] = {
        {600,1000+b},{1000+b,1000+b},{1000+b,600},{1000,600},{1000,1000},{600,1000},{600,1000+b}
      };
      assign_points(poly, coor);
    }
    correct(poly);
    return poly;
}

/* this function is to generate a corridor scenario
 * output: data type is polygon_2d in boost geometry library
 * input: the coordinate of vertex of the scenario
 */
polygon_2d Geometry::Channel(float originX, float originY, float length, float width)
{

     /* |<--------------------length----------------------------->
      *   ========================================================
      *                                      |
      *  (oringinal point)                   |width
      * *@==========================================================
      * the original point is the minimum value left bottom point
      */

	// Define a polygon and fill the outer ring.
    // In most cases you will read it from a file or database
    polygon_2d poly;
    {
        const double coor[][2] = {
            {originX,originY}, {originX,originY+width}, {originX+length, originY+width}, {originX+length, originY},
            {originX,originY} // closing point is opening point
            };
        assign_points(poly, coor);
    }

    // Polygons should be closed, and directed clockwise. If you're not sure if that is the case,
    // call the correct algorithm
    correct(poly);
    return poly;

}


/* this function is to define a rectangle measurement area in front of the bottleneck
 * output: data type is polygon_2d in boost geometry library
 * input: the distance of measurement to the bottleneck; the length and width of the area
 */
polygon_2d Geometry::MeasureAreabottleneck(float disTobottle, float length, float width)
{


    // Define a polygon and fill the outer ring.
    // In most cases you will read it from a file or database
    polygon_2d poly;
    {
        const double coor[][2] = {
            {-1.0*width/2.0,disTobottle}, {width/2.0,disTobottle}, {width/2.0, -1.0*length+disTobottle},
            {-1.0*width/2.0, -1*length+disTobottle}, {-1.0*width/2.0,disTobottle} // closing point is opening point
            };
        assign_points(poly, coor);
    }

    // Polygons should be closed, and directed clockwise. If you're not sure if that is the case,
    // call the correct algorithm
    correct(poly);
    return poly;

}
/* this function is to define a rectangle measurement area
 * output: data type is polygon_2d in boost geometry library
 * input: the left bottom point of the area, the length and width of the area
 */
polygon_2d Geometry::MeasureArea(float originX, float originY, float length, float width)
{

     /* |<--------------------length----------------------------->
      *   ========================================================
      *                                      |
      *  (original point)                   |width
      * *@==========================================================
      *  the original point is the minimum value left bottom point
      */

	// Define a polygon and fill the outer ring.
    // In most cases you will read it from a file or database
    polygon_2d poly;
    {
        const double coor[][2] = {
            {originX,originY}, {originX,originY+width}, {originX+length, originY+width}, {originX+length, originY},
            {originX,originY} // closing point is opening point
            };
        assign_points(poly, coor);
    }
    correct(poly);     // Polygons should be closed, and directed clockwise. If you're not sure if that is the case, call this function
    return poly;

}
