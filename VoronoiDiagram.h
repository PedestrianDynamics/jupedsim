/*
 * VoronoiDiagram.h
 *
 *  Created on: 06.02.2013
 *      Author: JUNZHANG
 */

#ifndef VORONOIDIAGRAM_H_
#define VORONOIDIAGRAM_H_

#include <vector>
#include <iostream>

//#include <boost/geometry/geometries/adapted/boost_polygon.hpp>
#include <boost/polygon/voronoi.hpp>
using boost::polygon::voronoi_builder;
using boost::polygon::voronoi_diagram;
//using namespace boost::polygon;

#include <boost/geometry.hpp>
#include <boost/geometry/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/geometries/adapted/c_array.hpp>
#include <boost/geometry/geometries/ring.hpp>
#include <boost/geometry/algorithms/intersection.hpp>
#include <boost/geometry/algorithms/within.hpp>
#include <boost/foreach.hpp>
//#include <boost/geometry/core/cs.hpp>

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
	double area_triangle(point_type2 tri_p1, point_type2 tri_p2, point_type2 tri_p3);
	bool point_inside_triangle(point_type2 pt, point_type2 tri_p1, point_type2 tri_p2, point_type2 tri_p3);
	std::vector<point_type2> add_bounding_points(point_type2 pt1, point_type2 pt2, point_type2 pt, double minX, double minY, double maxX, double maxY);
public:
	VoronoiDiagram();
	virtual ~VoronoiDiagram();
	std::vector<polygon_2d> getVoronoiPolygons(double *XInFrame, double *YInFrame, int numPedsInFrame);
	std::vector<polygon_2d> cutPolygonsWithGeometry(std::vector<polygon_2d> polygon, polygon_2d Geometry, double* xs, double* ys);
};

#endif /* VORONOIDIAGRAM_H_ */
