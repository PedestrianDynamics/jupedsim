/*
 * MeasurementArea.h
 *
 *  Created on: Aug 30, 2013
 *      Author: piccolo
 */

#ifndef MEASUREMENTAREA_H_
#define MEASUREMENTAREA_H_

#include <boost/geometry/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/geometries/adapted/c_array.hpp>
#include <boost/geometry/geometries/ring.hpp>

using namespace boost::geometry;
typedef model::d2::point_xy<double, cs::cartesian> point_2d;
typedef model::polygon<point_2d> polygon_2d;
typedef model::ring<point_2d> ring;

class MeasurementArea
{
public:
     virtual ~MeasurementArea()
     {
     }
     int _id;
     std::string _type;
};

class MeasurementArea_B: public MeasurementArea
{
public:
     polygon_2d _poly;
     double _length;
};

class MeasurementArea_L: public MeasurementArea
{
public:
     double _lineStartX;
     double _lineStartY;
     double _lineEndX;
     double _lineEndY;
};

#endif /* MEASUREMENTAREA_H_ */
