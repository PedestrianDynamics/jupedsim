/*
 * Geometry.h
 *
 *  Created on: Apr 29, 2010
 *      Author: ZhangJun
 */

#ifndef GEOMETRY_H_
#define GEOMETRY_H_

#include <boost/geometry/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/geometries/adapted/c_array.hpp>

using namespace boost::geometry;
//using namespace std;

typedef model::d2::point_xy<double,  cs::cartesian> point_2d;
typedef model::polygon<point_2d> polygon_2d;
//BOOST_GEOMETRY_REGISTER_C_ARRAY_CS(cs::cartesian)

class Geometry {
public:
	Geometry();
	virtual ~Geometry();
	polygon_2d bottleneck(float width, float length);
	polygon_2d Tawaf();
	polygon_2d MeasureAreabottleneck(float disTobottle, float length, float width);
	polygon_2d Channel(float originX, float originY, float length, float width);
	polygon_2d MeasureArea(float originX, float originY, float length, float width);
	polygon_2d Tshape();

	polygon_2d Corner(float b);
};

#endif /* GEOMETRY_H_ */
