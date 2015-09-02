/*
 * VoronoiPositionGenerator.cpp
 *
 *  Created on: Sep 2, 2015
 *      Author: gsp1502
 */

#include "VoronoiPositionGenerator.h"
//check if all includes are necessary
#include "../pedestrian/AgentsSourcesManager.h"
#include "../pedestrian/Pedestrian.h"
#include "../pedestrian/StartDistribution.h"
#include "../pedestrian/PedDistributor.h"
#include "../pedestrian/AgentsSource.h"
#include "../geometry/Building.h"
#include "../geometry/Point.h"

//#include "../mpi/LCGrid.h"
#include <iostream>
#include <thread>
#include <chrono>


#include "../geometry/SubRoom.h"
#include <stdlib.h>
#include <time.h>
#include <string>
#include <random>

#include "boost/polygon/voronoi.hpp"
using boost::polygon::voronoi_builder;
using boost::polygon::voronoi_diagram;
using boost::polygon::x;
using boost::polygon::y;
using boost::polygon::low;
using boost::polygon::high;


//wrapping the boost objects
namespace boost {
namespace polygon {
	template <>
	struct geometry_concept<Point> {
	  typedef point_concept type;
	};

	template <>
	struct point_traits<Point> {
	  typedef int coordinate_type;

	  static inline coordinate_type get(
		  const Point& point, orientation_2d orient) {
		return (orient == HORIZONTAL) ? point._x : point._y;
	  }
	};
}  // polygon
}  // boost


using namespace std;


//functions

bool IsEnoughInSubroom( SubRoom* subroom, Point& pt )
{
	std::vector<Wall> walls = subroom->GetAllWalls();
	std::vector<Transition*> transitions= subroom->GetAllTransitions();
	std::vector<Crossing*> crossings = subroom->GetAllCrossings();

	double radius = 0.4; //radius of a person

	//checking distance to walls
	for(unsigned int i=0; i<walls.size(); i++)
		if ( walls[i].DistTo(pt) < radius )
			return false;
	//checking distance to transitions
	for(unsigned int i=0; i<transitions.size(); i++)
		if ( transitions[i]->DistTo(pt) < radius )
			return false;
	//checking distance to crossings
	for(unsigned int i=0; i<crossings.size(); i++)
		if ( crossings[i]->DistTo(pt) < radius )
			return false;

	return true;
}

bool ComputeBestPositionVoronoiBoost(AgentsSource* src,
		std::vector<Pedestrian*>& peds, Building* building)
{
	bool return_value = true;
	auto dist = src->GetStartDistribution();
	int roomID = dist->GetRoomId();
	int subroomID = dist->GetSubroomID();
	std::string caption = (building->GetRoom( roomID ))->GetCaption();
	double radius = 0.4; //DO: change! radius of a person

	std::vector<Pedestrian*> existing_peds;
	building->GetPedestrians(roomID, subroomID, existing_peds);

	SubRoom* subroom = (building->GetRoom( roomID ))->GetSubRoom(subroomID);
	vector<Point> room_vertices = subroom->GetPolygon();

	double factor = 100;  //factor for conversion to integer for the boost voronoi

	vector<Point> fake_peds;  //doing this now so I don't have to do it in every loop
	Point temp(0,0);
	for (unsigned int i=0; i<room_vertices.size(); i++ )
	{
		Point center_pos = subroom->GetCentroid();
		temp.SetX( center_pos.GetX()-room_vertices[i].GetX( ) );
		temp.SetY( center_pos.GetY()-room_vertices[i].GetY( ) );
		temp = temp/sqrt(temp.NormSquare());
		temp = temp*(radius*1.4);  //now the norm of the vector is ~r*sqrt(2), pointing to the center
		temp = temp + room_vertices[i];
		temp.SetX( (int)(temp.GetX()*factor) );
		temp.SetY( (int)(temp.GetY()*factor) );
		fake_peds.push_back( temp );
	}

	for(auto&& ped : peds)
	{
		if(existing_peds.size() == 0 )
		{

			Point center_pos = subroom->GetCentroid();

			srand (time(NULL));
			double x_coor = 3 * ( (double)rand() / (double)RAND_MAX ) - 1.5;
			double y_coor = 3 * ( (double)rand() / (double)RAND_MAX ) - 1.5;
			Point random_pos(x_coor, y_coor);
			Point new_pos = center_pos + random_pos;

			if ( subroom->IsInSubRoom( new_pos ) )
			{
				if( IsEnoughInSubroom(subroom, new_pos ) )
				{
					ped->SetPos(center_pos + random_pos, true);
					Point v = (ped->GetExitLine()->ShortestPoint(ped->GetPos())- ped->GetPos()).Normalized();
					double speed=ped->GetV0Norm();
					v=v*speed;
					ped->SetV(v);
				}
				else
				{
					ped->SetPos(center_pos, true);
					Point v = (ped->GetExitLine()->ShortestPoint(ped->GetPos())- ped->GetPos()).Normalized();
					double speed=ped->GetV0Norm();
					v=v*speed;
					ped->SetV(v);
				}
			}
			else
			{
				ped->SetPos(center_pos, true);
				Point v = (ped->GetExitLine()->ShortestPoint(ped->GetPos())- ped->GetPos()).Normalized();
				double speed=ped->GetV0Norm();
				v=v*speed;
				ped->SetV(v);
			}


		}//0
		else
		{
			std::vector<Point> discrete_positions;
			std::vector<Point> velocities_vector;

			Point temp(0,0);
			Point v(0,0);
			double no = 0;

			//points from double to integer
			for (auto&& iter = existing_peds.begin(); iter != existing_peds.end(); ++iter)
			{
				const Point& pos = (*iter)->GetPos();
				temp.SetX( (int)( pos.GetX()*factor ) );
				temp.SetY( (int)( pos.GetY()*factor ) );
				discrete_positions.push_back( temp );
				velocities_vector.push_back( (*iter)->GetV() );

				//calculating the mean, using it for the fake pedestrians
				v = v + (*iter)->GetV();
				no++;
			}

			v = v/no; //this is the mean of all the velocities

			//adding fake people to the voronoi diagram
			for (unsigned int i=0; i<room_vertices.size(); i++ )
			{
				discrete_positions.push_back( fake_peds[i] );
				velocities_vector.push_back( v ); //DO: what speed?
			}

			//constructing the diagram
			voronoi_diagram<double> vd;
			construct_voronoi(discrete_positions.begin(), discrete_positions.end(), &vd);

			voronoi_diagram<double>::const_vertex_iterator chosen_it = vd.vertices().begin();
			double dis = 0;
			VoronoiBestVertexMax(discrete_positions, vd, subroom, factor, chosen_it, dis);

			if( dis > radius*factor*radius*factor)// be careful with the factor!! radius*factor
			{
				Point pos( chosen_it->x()/factor, chosen_it->y()/factor ); //check!
				ped->SetPos(pos , true);
				VoronoiAdjustVelocityNeighbour( vd, chosen_it, ped, velocities_vector );
			}
			else
			{
				return_value = false;
				//reject the pedestrian
			}

			/*else //try with the maximum distance, don't need this if already using the VoronoiBestVertexMax function
			{
				VoronoiBestVertexMax(discrete_positions, vd, subroom, factor, chosen_it, dis );
				if( dis > radius*factor*radius*factor)// be careful with the factor!! radius*factor
				{
					Point pos( chosen_it->x()/factor, chosen_it->y()/factor ); //check!
					ped->SetPos(pos , true);
					VoronoiAdjustVelocityNeighbour( vd, chosen_it, ped, velocities_vector );
				}
				else
				{
					return_value = false;
					//reject the pedestrian
				}
			}*/
		}// >0

		existing_peds.push_back(ped);
	}//for loop

	return return_value;
}

//gives an agent the mean velocity of his voronoi-neighbors
void VoronoiAdjustVelocityNeighbour( const voronoi_diagram<double>& vd, voronoi_diagram<double>::const_vertex_iterator& chosen_it,
			Pedestrian* ped, const std::vector<Point>& velocities_vector )
{
	//finding the neighbors (nearest pedestrians) of the chosen vertex
	const voronoi_diagram<double>::vertex_type &vertex = *chosen_it;
	const voronoi_diagram<double>::edge_type *edge = vertex.incident_edge();
	double sum_x=0, sum_y=0;
	double no=0;
	std::size_t index;

	do
	{
		no++;
		index = ( edge->cell() )->source_index();
		const Point& v = velocities_vector[index];
		sum_x += v.GetX();
		sum_y += v.GetY();
		edge = edge->rot_next();
	} while (edge != vertex.incident_edge());

	Point v(sum_x/no, sum_y/no);
	ped->SetV(v);

}

//gives the voronoi vertex with max distance
void VoronoiBestVertexMax (const std::vector<Point>& discrete_positions, const voronoi_diagram<double>& vd, SubRoom* subroom,
		double factor, voronoi_diagram<double>::const_vertex_iterator& max_it, double& max_dis	)
{
	double dis = 0;
	for (voronoi_diagram<double>::const_vertex_iterator it = vd.vertices().begin(); it != vd.vertices().end(); ++it)
	{
		Point vert_pos( it->x()/factor, it->y()/factor );
		if( subroom->IsInSubRoom(vert_pos) )
			if( IsEnoughInSubroom( subroom, vert_pos ) )
			{
				const voronoi_diagram<double>::vertex_type &vertex = *it;
				const voronoi_diagram<double>::edge_type *edge = vertex.incident_edge();

				std::size_t index = ( edge->cell() )->source_index();
				Point p = discrete_positions[index];

				dis = ( p.GetX() - it->x() )*( p.GetX() - it->x() )  + ( p.GetY() - it->y() )*( p.GetY() - it->y() )  ;
				if(dis > max_dis)
				{
					max_dis = dis;
					max_it = it;
				}
			}
	}
	//at the end, max_it is the choosen vertex, or the first vertex - max_dis=0 assures that this position will not be taken
}

//gives random voronoi vertex but with weights proportional to squared distances
void VoronoiBestVertexRandMax (const std::vector<Point>& discrete_positions, const voronoi_diagram<double>& vd, SubRoom* subroom,
		double factor, voronoi_diagram<double>::const_vertex_iterator& chosen_it, double& dis	)
{
	std::vector< voronoi_diagram<double>::const_vertex_iterator > possible_vertices;
	vector<double> partial_sums;
	unsigned int size;

	for (voronoi_diagram<double>::const_vertex_iterator it = vd.vertices().begin(); it != vd.vertices().end(); ++it)
	{
		Point vert_pos = Point( it->x()/factor, it->y()/factor );
		if( subroom->IsInSubRoom( vert_pos ) )
			if( IsEnoughInSubroom(subroom, vert_pos) )
			{
				const voronoi_diagram<double>::vertex_type &vertex = *it;
				const voronoi_diagram<double>::edge_type *edge = vertex.incident_edge();

				std::size_t index = ( edge->cell() )->source_index();
				Point p = discrete_positions[index];

				dis = ( p.GetX() - it->x() )*( p.GetX() - it->x() )   + ( p.GetY() - it->y() )*( p.GetY() - it->y() )  ;

				possible_vertices.push_back( it );
				partial_sums.push_back( dis );

				size = partial_sums.size();
				if( size > 1 )
				{
					partial_sums[ size - 1 ] += partial_sums[ size - 2 ];
				}
			}
	}
	//now we have the vector of possible vertices and weights and we can choose one randomly

	double lower_bound = 0;
	double upper_bound = partial_sums[size-1];
	std::uniform_real_distribution<double> unif(lower_bound,upper_bound);
	std::default_random_engine re;
	double a_random_double = unif(re);

	for (unsigned int i=0; i<size; i++)
	{
		if ( partial_sums[i] >= a_random_double )
		{
			//this is the chosen index
			chosen_it = possible_vertices[i];
			dis = partial_sums[i];
			if( i > 1 )
				dis -= partial_sums[i-1];
			break;
		}
	}

}

//gives a random voronoi vertex
void VoronoiBestVertexRand (const std::vector<Point>& discrete_positions, const voronoi_diagram<double>& vd, SubRoom* subroom,
		double factor, voronoi_diagram<double>::const_vertex_iterator& chosen_it, double& dis	)
{
	std::vector< voronoi_diagram<double>::const_vertex_iterator > possible_vertices;
	std::vector<double> distances;

	for (voronoi_diagram<double>::const_vertex_iterator it = vd.vertices().begin(); it != vd.vertices().end(); ++it)
	{
		Point vert_pos = Point( it->x()/factor, it->y()/factor );
		if( subroom->IsInSubRoom(vert_pos) )
			if( IsEnoughInSubroom(subroom, vert_pos) )
			{
				const voronoi_diagram<double>::vertex_type &vertex = *it;
				const voronoi_diagram<double>::edge_type *edge = vertex.incident_edge();

				std::size_t index = ( edge->cell() )->source_index();
				Point p = discrete_positions[index];

				dis = ( p.GetX() - it->x() )*( p.GetX() - it->x() )   + ( p.GetY() - it->y() )*( p.GetY() - it->y() )  ;

				possible_vertices.push_back( it );
				distances.push_back( dis );
			}
	}
	//now we have all the possible vertices and their distances and we can choose one randomly

	srand (time(NULL));
	unsigned int i = rand() % possible_vertices.size();
	chosen_it = possible_vertices[i];
	dis = distances[i];

}


