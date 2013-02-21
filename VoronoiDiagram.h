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
#include "boost/polygon/voronoi.hpp"
using boost::polygon::voronoi_builder;
using boost::polygon::voronoi_diagram;
using namespace boost::polygon;

typedef double coordinate_type;
typedef point_data<coordinate_type> point_type;
typedef voronoi_diagram<coordinate_type> VD;
typedef VD::edge_type edge_type;
typedef VD::cell_type cell_type;
typedef VD::cell_type::source_index_type source_index_type;

#include <boost/geometry.hpp>
#include <boost/geometry/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/geometries/adapted/c_array.hpp>
#include <boost/geometry/geometries/ring.hpp>

typedef boost::geometry::model::d2::point_xy<double, boost::geometry::cs::cartesian> point_2d;
typedef boost::geometry::model::polygon<point_2d> polygon_2d;



class VoronoiDiagram {
private:

	std::vector<point_type> points;
	void clip_infinite_edge( const edge_type& edge);
	void Inter_Ray_Box(double ray[2][2], double minX, double minY, double maxX, double maxY);
	//void getVoronoiPolygons(float *XInFrame, float *YInFrame,int numPedsInFrame, double minX, double minY, double maxX, double maxY) ;
	point_type retrieve_point(const cell_type& cell);
	point_type clip_infinite_edge1( const edge_type& edge, double minX, double minY, double maxX, double maxY);
	double area_triangle(point_type tri_p1, point_type tri_p2, point_type tri_p3);
	bool point_inside_triangle(point_type pt, point_type tri_p1, point_type tri_p2, point_type tri_p3);
	std::vector<point_type> add_bounding_points(point_type pt1, point_type pt2, point_type pt, double minX, double minY, double maxX, double maxY);
	std::vector<polygon_2d> getVoronoiPolygons(double *XInFrame, double *YInFrame, int numPedsInFrame, double minX, double minY, double maxX, double maxY);
public:
	VoronoiDiagram();
	virtual ~VoronoiDiagram();

};

#endif /* VORONOIDIAGRAM_H_ */
