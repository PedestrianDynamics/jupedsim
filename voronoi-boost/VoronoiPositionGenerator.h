/*
 * VoronoiPositionGenerator.h
 *
 *  Created on: Sep 2, 2015
 *      Author: gsp1502
 */

#ifndef VORONOI_BOOST_VORONOIPOSITIONGENERATOR_H_
#define VORONOI_BOOST_VORONOIPOSITIONGENERATOR_H_


#include <vector>
#include <memory>
#include <atomic>

#include "boost/polygon/voronoi.hpp"
using boost::polygon::voronoi_builder;
using boost::polygon::voronoi_diagram;
using boost::polygon::x;
using boost::polygon::y;
using boost::polygon::low;
using boost::polygon::high;
#include "../geometry/SubRoom.h"


class AgentsSource;
class Building;
class Pedestrian;
class Point;

	/**
	 * Calculates if a point (which is inside the subroom) is far enough from the walls ( > radius of a person)
	 * @param subroom
	 * @param pt
	 */
	bool IsEnoughInSubroom( SubRoom* subroom, Point& pt );

	/**
	* Position incoming pedestrian using voronoi from boost library
	* @param src
	* @param peds
	*/
	bool ComputeBestPositionVoronoiBoost(AgentsSource* src, std::vector<Pedestrian*>& peds, Building* _building);


	/**
	 * Position incoming pedestrian on the vertex with the greatest distance
	 * @param discrete_positions
	 * @param vd
	 * @param subroom
	 * @param max_it
	 * @param max_dis
	 */
	void VoronoiBestVertexMax (const std::vector<Point>& discrete_positions, const voronoi_diagram<double>& vd, SubRoom* subroom, double factor,
		voronoi_diagram<double>::const_vertex_iterator& max_it, double& max_dis	);

	/**
	 * Position incoming pedestrian on the vertex with greater probability for greater distances
	 * @param discrete_positions
	 * @param vd
	 * @param subroom
	 * @param max_it
	 * @param max_dis
	 */
	void VoronoiBestVertexRandMax (const std::vector<Point>& discrete_positions, const voronoi_diagram<double>& vd, SubRoom* subroom, double factor,
				voronoi_diagram<double>::const_vertex_iterator& max_it, double& max_dis	);

	/**
	 * Position incoming pedestrian on a random vertex
	 * @param discrete_positions
	 * @param vd
	 * @param subroom
	 * @param max_it
	 * @param max_dis
	 */
	void VoronoiBestVertexRand (const std::vector<Point>& discrete_positions, const voronoi_diagram<double>& vd, SubRoom* subroom, double factor,
				voronoi_diagram<double>::const_vertex_iterator& max_it, double& max_dis	);

	/**
	 * Gives a person the mean velocity of his voronoi-neighbours
	 * @param vd
	 * @param chosen_it
	 * @param velocities_vector
	 */
	void VoronoiAdjustVelocityNeighbour( const voronoi_diagram<double>& vd, voronoi_diagram<double>::const_vertex_iterator& chosen_it,
			Pedestrian* ped, const std::vector<Point>& velocities_vector ) ;



#endif /* VORONOI_BOOST_VORONOIPOSITIONGENERATOR_H_ */
