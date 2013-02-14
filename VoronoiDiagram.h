/*
 * VoronoiDiagram.h
 *
 *  Created on: 06.02.2013
 *      Author: JUNZHANG
 */

#ifndef VORONOIDIAGRAM_H_
#define VORONOIDIAGRAM_H_

#include <iostream>
using namespace std;

#include <iostream>
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/adapted/boost_polygon.hpp>
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

class VoronoiDiagram {
private:
	vector<point_type> points;
	void clip_infinite_edge( const edge_type& edge);
	int iterate_primary_edges(const voronoi_diagram<double> &vd, double minX, double minY, double maxX, double maxY);
	void Inter_Ray_Box(double ray[2][2], double minX, double minY, double maxX, double maxY);
	void getVoronoiPolygons(float *XInFrame, float *YInFrame,int numPedsInFrame, double minX, double minY, double maxX, double maxY) ;
	point_type retrieve_point(const cell_type& cell);

public:
	VoronoiDiagram();
	virtual ~VoronoiDiagram();

};

#endif /* VORONOIDIAGRAM_H_ */
