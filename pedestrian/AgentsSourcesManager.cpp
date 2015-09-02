/**
 * \file        AgentsSourcesManager.cpp
 * \date        Apr 14, 2015
 * \version     v0.7
 * \copyright   <2009-2015> Forschungszentrum J��lich GmbH. All rights reserved.
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
 * This class is responsible for materialising agent in a given location at a given frequency up to a maximum number.
 * The optimal position where to put the agents is given by various algorithms, for instance
 * the Voronoi algorithm or the Mitchell Best candidate algorithm.
 *
 **/

#include "AgentsSourcesManager.h"
#include "Pedestrian.h"
#include "StartDistribution.h"
#include "PedDistributor.h"
#include "AgentsSource.h"
//#include "../voronoi/VoronoiDiagramGenerator.h"
#include "../geometry/Building.h"
#include "../geometry/Point.h"

#include "../mpi/LCGrid.h"
#include <iostream>
#include <thread>
#include <chrono>
#include "AgentsQueue.h"


//a.brkic begin
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

//a.brkic end

using namespace std;

bool AgentsSourcesManager::_isCompleted=true;

AgentsSourcesManager::AgentsSourcesManager()
{
}

AgentsSourcesManager::~AgentsSourcesManager()
{
}

void AgentsSourcesManager::operator()()
{
     Run();
}

void AgentsSourcesManager::Run()
{
     Log->Write("INFO:\tStarting agent manager thread");

     //Generate all agents required for the complete simulation
     //It might be more efficient to generate at each frequency step
     for (const auto& src : _sources)
     {
          src->GenerateAgentsAndAddToPool(src->GetMaxAgents(), _building);
     }

     //first call ignoring the return value
     ProcessAllSources();

     //the loop is updated each x second.
     //it might be better to use a timer
     _isCompleted = false;
     bool finished = false;
     long updateFrequency = 4;     // 1 = second
     do
     {
          int current_time = Pedestrian::GetGlobalTime();

          if ((current_time != _lastUpdateTime)
                    && ((current_time % updateFrequency) == 0))
          {
               finished=ProcessAllSources();
               _lastUpdateTime = current_time;
          }
          //wait some time
         // std::this_thread::sleep_for(std::chrono::milliseconds(1));
     } while (!finished);
     Log->Write("INFO:\tTerminating agent manager thread");
     _isCompleted = true;
}

bool AgentsSourcesManager::ProcessAllSources() const
{
     bool empty=true;
     for (const auto& src : _sources)
     {
          if (src->GetPoolSize())
          {
        	  auto dist = src->GetStartDistribution();
        	  int roomID = dist->GetRoomId();
        	  int subroomID = dist->GetSubroomID();
        	  SubRoom* subroom=(_building->GetRoom( roomID ))->GetSubRoom(subroomID);
        	  subroom->SetHelpVariables();


        	   vector<Pedestrian*> peds;
               src->RemoveAgentsFromPool(peds,src->GetFrequency());
               Log->Write("INFO:\tSource %d generating %d agents (%d remaining)",src->GetId(),peds.size(),src->GetPoolSize());

               //ComputeBestPositionRandom(src.get(), peds);
               //todo: compute the optimal position for insertion using voronoi
               if( !ComputeBestPositionVoronoiBoost(src.get(), peds) )
            	   Log->Write("INFO:\t there was no place for some pedestrians");
               //ComputeBestPositionDummy( src.get(), peds );
               /*for (auto&& ped : peds)
               {
               ComputeBestPositionVoronoiBoost(src.get(), ped);
               //ped->Dump(ped->GetID(),0);
               }*/
               AgentsQueueIn::Add(peds);
               empty = false;
          }
          //src->Dump();//exit(0);
     }
     return empty;
}

//4 agents frequency, just for an example
void AgentsSourcesManager::ComputeBestPositionDummy(AgentsSource* src,
          vector<Pedestrian*>& peds)const
{
	peds[0]->SetPos( Point(10,5.5) );
	peds[1]->SetPos( Point(10,4.9) );
	peds[2]->SetPos( Point(10,4.3) );
	peds[3]->SetPos( Point(10,3.7) );

	/*peds[0]->SetPos( Point(10,5.4) );
	peds[1]->SetPos( Point(10,4.6) );
	peds[2]->SetPos( Point(10,3.8) );*/

	for(auto&& ped : peds)
	{
		Point v = (ped->GetExitLine()->ShortestPoint(ped->GetPos())- ped->GetPos()).Normalized();
		double speed=ped->GetV0Norm();
		v=v*speed;
		ped->SetV(v);
	}
}
/*
void AgentsSourcesManager::ComputeBestPositionVoronoi(AgentsSource* src,
          Pedestrian* agent) const
{
     auto dist = src->GetStartDistribution();
     double bounds[4];
     dist->Getbounds(bounds);
     int roomID = dist->GetRoomId();
     int subroomID = dist->GetSubroomID();

     //Get all pedestrians in that location
     vector<Pedestrian*> peds;
     _building->GetPedestrians(roomID, subroomID, peds);

     //filter the points that are not within the boundaries
     for (auto&& iter = peds.begin(); iter != peds.end();)
     {
          const Point& pos = (*iter)->GetPos();
          if ((bounds[0] <= pos._x && pos._x <= bounds[1])
                    && (bounds[1] <= pos._y && pos._y <= bounds[2]))
          {
               iter = peds.erase(iter);
               cout << "removing (testing only)..." << endl;
               exit(0);
          } else
          {
               ++iter;
          }
     }

     //special case with 1, 2 or only three pedestrians in the area
     if (peds.size() < 3)
     {
          //TODO/random position in the area
          return;

     }
     // compute the cells and cut with the bounds
     const int count = peds.size();
     float* xValues = new float[count];
     float* yValues = new float[count];
     //float xValues[count];
     //float yValues[count];

     for (int i = 0; i < count; i++)
     {
          xValues[i] = peds[i]->GetPos()._x;
          yValues[i] = peds[i]->GetPos()._y;
     }

     VoronoiDiagramGenerator vdg;
     vdg.generateVoronoi(xValues, yValues, count, bounds[0], bounds[1],
               bounds[2], bounds[3], 3);
     vdg.resetIterator();
     vdg.resetVerticesIterator();

     printf("\n------vertices---------\n");
     //collect the positions
     vector<Point> positions;
     float x1, y1;
     while (vdg.getNextVertex(x1, y1))
     {
          printf("GOT Point (%f,%f)\n", x1, y1);
          positions.push_back(Point(x1, y1));
     }

     //look for the biggest spot
     map<double, Point> map_dist_to_position;

     for (auto&& pos : positions)
     {
          double min_dist = FLT_MAX;

          for (auto&& ped : peds)
          {
               double dist = (pos - ped->GetPos()).NormSquare();
               if (dist < min_dist)
               {
                    min_dist = dist;
               }
          }
          map_dist_to_position[min_dist] = pos;
     }

     //list the result
     for (auto&& mp : map_dist_to_position)
     {
          cout << "dist: " << mp.first << " pos: " << mp.second.toString()
                                        << endl;
          //agent->SetPos(mp.second, true);
     }

     //the elements are ordered.
     // so the last one has the largest distance
     if (!map_dist_to_position.empty())
     {
          agent->SetPos(map_dist_to_position.rbegin()->second, true);
          cout << "position:" << agent->GetPos().toString() << endl;
          //exit(0);

     } else
     {
          cout << "position not set:" << endl;
          cout << "size: " << map_dist_to_position.size() << endl;
          cout << " for " << peds.size() << " pedestrians" << endl;
          exit(0);
     }
     //exit(0);
     // float x1,y1,x2,y2;
     //while(vdg.getNext(x1,y1,x2,y2))
     //{
     //     printf("GOT Line (%f,%f)->(%f,%f)\n",x1,y1,x2, y2);
     //
     //}
     //compute the best position
     //exit(0);
}
*/

bool AgentsSourcesManager::IsEnoughInSubroom( SubRoom* subroom, Point& pt ) const
{
	vector<Wall> walls = subroom->GetAllWalls();
	bool return_value = true;
	double radius = 0.4; //radius of a person

	for(unsigned int i=0; i<walls.size(); i++)
		if ( walls[i].DistTo(pt) < radius )
		{
			return_value = false;
			break;
		}

	return return_value;
}

bool AgentsSourcesManager::ComputeBestPositionVoronoiBoost(AgentsSource* src,
		std::vector<Pedestrian*>& peds) const
{
	bool return_value = true;
	auto dist = src->GetStartDistribution();
	int roomID = dist->GetRoomId();
	int subroomID = dist->GetSubroomID();
	std::string caption = (_building->GetRoom( roomID ))->GetCaption();
	double radius = 0.4; //DO: change! radius of a person

	std::vector<Pedestrian*> existing_peds;
	_building->GetPedestrians(roomID, subroomID, existing_peds);

	SubRoom* subroom = (_building->GetRoom( roomID ))->GetSubRoom(subroomID);
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
			else //try with the maximum distance, don't need this if already using the VoronoiBestVertexMax function
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
			}
		}// >0

		existing_peds.push_back(ped);
	}//for loop

	return return_value;
}

//gives an agent the mean velocity of his voronoi-neighbors
void AgentsSourcesManager::VoronoiAdjustVelocityNeighbour( const voronoi_diagram<double>& vd, voronoi_diagram<double>::const_vertex_iterator& chosen_it,
			Pedestrian* ped, const std::vector<Point>& velocities_vector ) const
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
void AgentsSourcesManager::VoronoiBestVertexMax (const std::vector<Point>& discrete_positions, const voronoi_diagram<double>& vd, SubRoom* subroom,
		double factor, voronoi_diagram<double>::const_vertex_iterator& max_it, double& max_dis	) const
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
void AgentsSourcesManager::VoronoiBestVertexRandMax (const std::vector<Point>& discrete_positions, const voronoi_diagram<double>& vd, SubRoom* subroom,
		double factor, voronoi_diagram<double>::const_vertex_iterator& chosen_it, double& dis	) const
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
void AgentsSourcesManager::VoronoiBestVertexRand (const std::vector<Point>& discrete_positions, const voronoi_diagram<double>& vd, SubRoom* subroom,
		double factor, voronoi_diagram<double>::const_vertex_iterator& chosen_it, double& dis	) const
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


void AgentsSourcesManager::ComputeBestPositionRandom(AgentsSource* src,
          std::vector<Pedestrian*>& peds) const
{

     //generate the agents with default positions
     auto dist = src->GetStartDistribution();
     auto subroom = _building->GetRoom(dist->GetRoomId())->GetSubRoom(
               dist->GetSubroomID());
     vector<Point> positions = PedDistributor::PossiblePositions(*subroom);
     double bounds[4] = { 0, 0, 0, 0 };
     dist->Getbounds(bounds);

     vector<Point> extra_positions;

     for (auto& ped : peds)
     {
          //need to be called at each iteration
          SortPositionByDensity(positions, extra_positions);

          int index = -1;

          //in the case a range was specified
          //just take the first element
          for (unsigned int a = 0; a < positions.size(); a++)
          {
               Point pos = positions[a];
               //cout<<"checking: "<<pos.toString()<<endl;
               if ((bounds[0] <= pos._x) && (pos._x <= bounds[1])
                         && (bounds[2] <= pos._y) && (pos._y < bounds[3]))
               {
                    index = a;
                    break;
               }
          }
          if (index == -1)
          {
               if (positions.size())
               {
                    Log->Write(
                              "ERROR:\t AgentSourceManager Cannot distribute pedestrians in the mentioned area [%0.2f,%0.2f,%0.2f,%0.2f]",
                              bounds[0], bounds[1], bounds[2], bounds[3]);
                    Log->Write("     \t Specifying a subroom_id might help");
                    Log->Write("     \t %d positions were available",positions.size());
                    exit(EXIT_FAILURE);
               }
          }
          else
          {
               const Point& pos = positions[index];
               extra_positions.push_back(pos);
               ped->SetPos(pos, true); //true for the initial position
               positions.erase(positions.begin() + index);

               //at this point we have a position
               //so we can adjust the velocity
               //AdjustVelocityUsingWeidmann(ped);
               AdjustVelocityByNeighbour(ped);
          }
     }
}

void AgentsSourcesManager::AdjustVelocityByNeighbour(Pedestrian* ped) const
{
     //get the density
     vector<Pedestrian*> neighbours;
     _building->GetGrid()->GetNeighbourhood(ped,neighbours);

     double speed=0.0;
     double radius_square=0.56*0.56;//corresponding to an area of 1m3
     int count=0;

     for(const auto& p: neighbours)
     {
          //only pedes in a sepcific rance
          if( (ped->GetPos()-p->GetPos()).NormSquare()<=radius_square)
          {
               //only peds with the same destination
               if(ped->GetExitIndex()==p->GetExitIndex())
               {
                    double dist1=ped->GetDistanceToNextTarget();
                    double dist2=p->GetDistanceToNextTarget();
                    //only peds in front of me
                    if(dist2<dist1)
                    {
                         speed+=p->GetV().Norm();
                         count++;
                    }
               }
          }

     }
     //mean speed
     if(count==0)
     {
          speed=ped->GetV0Norm();
     }
     else
     {
          speed=speed/count;
     }

     if(ped->FindRoute()!=-1)
     {
          //get the next destination point
          Point v =(ped->GetExitLine()->ShortestPoint(ped->GetPos())- ped->GetPos()).Normalized();
          v=v*speed;
          ped->SetV(v);
     }
     else
     {
          Log->Write("ERROR:\t no route could be found for agent [%d] going to [%d]",ped->GetID(),ped->GetFinalDestination());
          //that will be most probably be fixed in the next computation step.
          // so do not abort
     }

}

void AgentsSourcesManager::AdjustVelocityUsingWeidmann(Pedestrian* ped) const
{
     //get the density
     vector<Pedestrian*> neighbours;
     _building->GetGrid()->GetNeighbourhood(ped,neighbours);

     //density in pers per m2
     double density = 1.0;
     //radius corresponding to a surface of 1m2
     //double radius_square=0.564*0.564;
     double radius_square=1.0;

     for(const auto& p: neighbours)
     {
          if( (ped->GetPos()-p->GetPos()).NormSquare()<=radius_square)
               density+=1.0;
     }
     density=density/(radius_square*M_PI);

     //get the velocity
     double density_max=5.4;

     //speed from taken from weidmann FD
     double speed=1.34*(1-exp(-1.913*(1.0/density-1.0/density_max)));
     if(speed>=ped->GetV0Norm())
     {
          speed=ped->GetV0Norm();
     }

     //set the velocity vector
     if(ped->FindRoute()!=-1)
     {
          //get the next destination point
          Point v =(ped->GetExitLine()->ShortestPoint(ped->GetPos())- ped->GetPos()).Normalized();
          v=v*speed;
          ped->SetV(v);
          //cout<<"density: "<<density<<endl;
     }
     else
     {
          Log->Write("ERROR:\t no route could be found for agent [%d] going to [%d]",ped->GetID(),ped->GetFinalDestination());
          //that will be most probably be fixed in the next computation step.
          // so do not abort
     }

}

void AgentsSourcesManager::SortPositionByDensity(std::vector<Point>& positions, std::vector<Point>& extra_positions) const
{
     std::multimap<double,Point> density2pt;
     //std::map<double,Point> density2pt;

     for(auto&& pt:positions)
     {
          vector<Pedestrian*> neighbours;
          _building->GetGrid()->GetNeighbourhood(pt,neighbours);
          //density in pers per m2
          double density = 0.0;
          double radius_square=0.40*0.40;

          for(const auto& p: neighbours)
          {
               if( (pt-p->GetPos()).NormSquare()<=radius_square)
                    density+=1.0;
          }

          //consider the extra positions
          for(const auto& ptx: extra_positions)
          {
               if( (ptx-pt).NormSquare()<=radius_square)
                    density+=1.0;
          }
          density=density/(radius_square*M_PI);

          density2pt.insert(std::pair<double,Point>(density,pt));

     }

     //cout<<"------------------"<<positions.size()<<"-------"<<endl;
     positions.clear();
     for(auto&& d: density2pt)
     {
          positions.push_back(d.second);
          //     printf("density [%lf, %s]\n",d.first, d.second.toString().c_str());
     }

}


void AgentsSourcesManager::GenerateAgents()
{

     for (const auto& src : _sources)
     {
          src->GenerateAgentsAndAddToPool(src->GetMaxAgents(), _building);
     }
}

void AgentsSourcesManager::AddSource(std::shared_ptr<AgentsSource> src)
{
     _sources.push_back(src);
     _isCompleted=false;//at least one source was provided
}

const std::vector<std::shared_ptr<AgentsSource> >& AgentsSourcesManager::GetSources() const
{
     return _sources;
}

void AgentsSourcesManager::SetBuilding(Building* building)
{
     _building = building;
}

bool AgentsSourcesManager::IsCompleted() const
{
     return _isCompleted;
}


Building* AgentsSourcesManager::GetBuilding() const
{
     return _building;
}

long AgentsSourcesManager::GetMaxAgentNumber() const
{
     long pop=0;
     for (const auto& src : _sources)
     {
          pop+=src->GetMaxAgents();
     }
     return pop;
}
