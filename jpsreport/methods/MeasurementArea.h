/**
 * \file        MeasurementArea.cpp
 * \date        Oct 10, 2014
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

#ifndef MEASUREMENTAREA_H_
#define MEASUREMENTAREA_H_

#include <boost/geometry/geometries/adapted/c_array.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/geometries/ring.hpp>
#include <boost/geometry/geometry.hpp>

using namespace boost::geometry;
typedef model::d2::point_xy<double, cs::cartesian> point_2d;
typedef model::polygon<point_2d> polygon_2d;
typedef model::ring<point_2d> ring;

class MeasurementArea
{
public:
    virtual ~MeasurementArea() {}
    int _id;
    std::string _type;
    double _zPos;
};

class MeasurementArea_B : public MeasurementArea
{
public:
    polygon_2d _poly;
    double _length = -1.0;
};

class MeasurementArea_L : public MeasurementArea
{
public:
    double _lineStartX;
    double _lineStartY;
    double _lineEndX;
    double _lineEndY;
};

#endif /* MEASUREMENTAREA_H_ */
