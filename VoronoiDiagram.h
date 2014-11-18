/**
 * \file        VoronoiDiagram.h
 * \date        Oct 10, 2014
 * \version     v0.6
 * \copyright   <2009-2014> Forschungszentrum J¨¹lich GmbH. All rights reserved.
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
 * The VoronoiDiagram class define functions used to calculate Voronoi diagrams
 * from trajectories.
 *
 *
 **/

#ifndef VORONOIDIAGRAM_H_
#define VORONOIDIAGRAM_H_

#include <vector>
#include <iostream>
#include <math.h>

#define PI 3.14159265

#include <boost/polygon/voronoi.hpp>
using boost::polygon::voronoi_builder;
using boost::polygon::voronoi_diagram;

#include <boost/geometry.hpp>
#include <boost/geometry/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/geometries/adapted/c_array.hpp>
#include <boost/geometry/geometries/ring.hpp>
#include <boost/geometry/algorithms/intersection.hpp>
#include <boost/geometry/algorithms/within.hpp>
#include <boost/foreach.hpp>

typedef boost::geometry::model::d2::point_xy<double, boost::geometry::cs::cartesian> point_2d;
typedef boost::geometry::model::polygon<point_2d> polygon_2d;

typedef boost::polygon::point_data<double> point_type2;
typedef double coordinate_type;
typedef boost::polygon::voronoi_diagram<double> VD;
typedef VD::edge_type edge_type;
typedef VD::cell_type cell_type;
typedef VD::cell_type::source_index_type source_index_type;


class VoronoiDiagram {
private:
    std::vector<point_type2> points;
    point_type2 retrieve_point(const cell_type& cell);
    point_type2 clip_infinite_edge( const edge_type& edge, double minX, double minY, double maxX, double maxY);
    double area_triangle(const point_type2& tri_p1, const point_type2& tri_p2, const point_type2& tri_p3);
    bool point_inside_triangle(const point_type2& pt, const point_type2& tri_p1, const point_type2& tri_p2, const point_type2& tri_p3);
    std::vector<point_type2> add_bounding_points(point_type2 pt1, point_type2 pt2, point_type2 pt, double minX, double minY, double maxX, double maxY);

public:
    VoronoiDiagram();
    virtual ~VoronoiDiagram();
    std::vector<polygon_2d> getVoronoiPolygons(double *XInFrame, double *YInFrame, double *VInFrame,int *IdInFrame, int numPedsInFrame);
    std::vector<polygon_2d> cutPolygonsWithGeometry(std::vector<polygon_2d> polygon, polygon_2d Geometry, double* xs, double* ys);
    std::vector<polygon_2d> cutPolygonsWithCircle(std::vector<polygon_2d> polygon, double* xs, double* ys, double radius, int edges);
};

#endif /* VORONOIDIAGRAM_H_ */
