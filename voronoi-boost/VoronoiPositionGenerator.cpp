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


//wrapping the boost objects (just the point object)
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
//TODO: refactor the function
bool IsEnoughInSubroom( SubRoom* subroom, Point& pt, double radius )
{
     //TODO: code refactoring:
     for (const auto& wall: subroom->GetAllWalls())
          if(wall.DistTo(pt)<radius)
               return false;

     for(const auto& trans: subroom->GetAllTransitions() )
    	 if ( trans->DistTo(pt) < radius + 0.2 )
    		 return false;

     for( const auto& cross: subroom->GetAllCrossings() )
    	 if( cross->DistTo(pt) < radius + 0.2 )
    		 return false;

     return true;
}

bool ComputeBestPositionVoronoiBoost(AgentsSource* src, std::vector<Pedestrian*>& peds,
          Building* building)
{
     bool return_value = true;
     auto dist = src->GetStartDistribution();
     int roomID = dist->GetRoomId();
     int subroomID = dist->GetSubroomID();
     std::string caption = (building->GetRoom( roomID ))->GetCaption();

     std::vector<Pedestrian*> existing_peds;
     std::vector<Pedestrian*> peds_without_place;
     building->GetPedestrians(roomID, subroomID, existing_peds);

     double radius = 0.3; //radius of a person, 0.3 is just some number(needed for the fake_peds bellow), will be changed afterwards

     SubRoom* subroom = building->GetRoom( roomID )->GetSubRoom(subroomID);

     double factor = 100;  //factor for conversion to integer for the boost voronoi

     std::vector<Point> fake_peds;
     Point temp(0,0);
     //fake_peds will be the positions of "fake" pedestrians, multiplied by factor and converted to int
     for (auto vert: subroom->GetPolygon() ) //room vertices
     {
    	const Point& center_pos = subroom->GetCentroid();
    	temp.SetX( center_pos.GetX()-vert.GetX( ) );
		temp.SetY( center_pos.GetY()-vert.GetY( ) );
		temp = temp/sqrt(temp.NormSquare());
		temp = temp*(radius*1.4);  //now the norm of the vector is ~r*sqrt(2), pointing to the center
		temp = temp + vert;
		temp.SetX( (int)(temp.GetX()*factor) );
		temp.SetY( (int)(temp.GetY()*factor) );
		fake_peds.push_back( temp );
     }

     std::vector<Pedestrian*>::iterator iter_ped;
     for (iter_ped = peds.begin(); iter_ped != peds.end(); )
     {
          Pedestrian* ped = (*iter_ped);
          radius = ped->GetEllipse().GetBmax(); //max radius of the curren pedestrian

          if(existing_peds.size() == 0 )
          {
        	   const Point& center_pos = subroom->GetCentroid();

               double x_coor = 3 * ( (double)rand() / (double)RAND_MAX ) - 1.5;
               double y_coor = 3 * ( (double)rand() / (double)RAND_MAX ) - 1.5;
               Point random_pos(x_coor, y_coor);
               Point new_pos = center_pos + random_pos;

               if ( subroom->IsInSubRoom( new_pos ) )
               {
                    if( IsEnoughInSubroom(subroom, new_pos, radius ) )
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

               existing_peds.push_back(ped);


          }//0

          else //more than one pedestrian
          {
        	   std::vector<Point> discrete_positions;
               std::vector<Point> velocities_vector;
               Point temp(0,0);
               Point v(0,0);
               double no = 0;

               //points from double to integer
               for (const auto& eped: existing_peds)
               {
                    const Point& pos = eped->GetPos();
                    temp.SetX( (int)( pos.GetX()*factor ) );
                    temp.SetY( (int)( pos.GetY()*factor ) );
                    discrete_positions.push_back( temp );
                    velocities_vector.push_back( eped->GetV() );

                    //calculating the mean, using it for the fake pedestrians
                    v = v + eped->GetV();
                    no++;
               }

               v = v/no; //this is the mean of all velocities

               //adding fake people to the vector for constructing voronoi diagram
               for (unsigned int i=0; i<subroom->GetPolygon().size(); i++ )
               {
                    discrete_positions.push_back( fake_peds[i] );
                    velocities_vector.push_back( v );
               }

               //constructing the diagram
               voronoi_diagram<double> vd;
               construct_voronoi(discrete_positions.begin(), discrete_positions.end(), &vd);

               voronoi_diagram<double>::const_vertex_iterator chosen_it = vd.vertices().begin();
               double dis = 0;
               VoronoiBestVertexMax(discrete_positions, vd, subroom, factor, chosen_it, dis, radius);

               if( dis > radius*factor*radius*factor)// be careful with the factor!! radius*factor
               {
                    Point pos( chosen_it->x()/factor, chosen_it->y()/factor ); //check!
                    ped->SetPos(pos , true);
                    VoronoiAdjustVelocityNeighbour( vd, chosen_it, ped, velocities_vector );

                    // proceed to the next pedestrian
                    existing_peds.push_back(ped);
                    ++iter_ped;

               }
               else
               {
                    //reject the pedestrian:
                    return_value = false;
                    peds_without_place.push_back(*iter_ped); //Put in a different queue, they will be put back in the source.
                    iter_ped=peds.erase(iter_ped); // remove from the initial vector since it should only contain the pedestrians that could find a place
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


     }//for loop


     //maybe not all pedestrians could find a place, requeue them in the source
     if(peds_without_place.size()>0)
          src->AddAgentsToPool(peds_without_place);

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
          double factor, voronoi_diagram<double>::const_vertex_iterator& max_it, double& max_dis, double radius	)
{
     double dis = 0;


     for (auto it = vd.vertices().begin(); it != vd.vertices().end(); ++it)
     {
          Point vert_pos( it->x()/factor, it->y()/factor );
          if( subroom->IsInSubRoom(vert_pos) )
               if( IsEnoughInSubroom( subroom, vert_pos, radius ) )
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
          double factor, voronoi_diagram<double>::const_vertex_iterator& chosen_it, double& dis	, double radius)
{
     std::vector< voronoi_diagram<double>::const_vertex_iterator > possible_vertices;
     vector<double> partial_sums;
     unsigned int size=0;

     for (auto it = vd.vertices().begin(); it != vd.vertices().end(); ++it)
     {
          Point vert_pos = Point( it->x()/factor, it->y()/factor );
          if( subroom->IsInSubRoom( vert_pos ) )
               if( IsEnoughInSubroom(subroom, vert_pos,radius) )
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
          double factor, voronoi_diagram<double>::const_vertex_iterator& chosen_it, double& dis, double radius	)
{
     std::vector< voronoi_diagram<double>::const_vertex_iterator > possible_vertices;
     std::vector<double> distances;

     for (auto it = vd.vertices().begin(); it != vd.vertices().end(); ++it)
     {
          Point vert_pos = Point( it->x()/factor, it->y()/factor );
          if( subroom->IsInSubRoom(vert_pos) )
               if( IsEnoughInSubroom(subroom, vert_pos, radius) )
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


