/*
 * VoronoiPositionGenerator.cpp
 *
 *  Created on: Sep 2, 2015
 *      Author: gsp1502
 */

#define PLOT_VORONOI_DIAGRAM 0
static int global_count = 0;



#include "VoronoiPositionGenerator.h"
//check if all includes are necessary
#include "../pedestrian/AgentsSourcesManager.h"
#include "../pedestrian/Pedestrian.h"
#include "geometry/Wall.h"

//#include "../pedestrian/StartDistribution.h"
//#include "../pedestrian/PedDistributor.h"
//#include "../pedestrian/AgentsSource.h"
//#include "../geometry/Building.h"
//#include "../geometry/Point.h"

//#include "../mpi/LCGrid.h"
//#include <iostream>
#include <thread>
//#include <chrono>


//#include "../geometry/SubRoom.h"
//#include <stdlib.h>
//#include <time.h>
//#include <string>
//#include <random>

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


//functions
//TODO: refactor the function
bool IsEnoughInSubroom( SubRoom* subroom, Point& pt, double radius )
{
     for (const auto& wall: subroom->GetAllWalls())
          if(wall.DistTo(pt)<radius)
               return false;

     for(const auto& trans: subroom->GetAllTransitions() )
         if ( trans->DistTo(pt) < radius + 0.1 )
                 return false;

     for( const auto& cross: subroom->GetAllCrossings() )
         if( cross->DistTo(pt) < radius + 0.1 )
                 return false;

     return true;
}

//Point ComputeBestPositionVoronoiBoost(Goal* goal, std::vector<Pedestrian*>& pedInside)
//{
//
//}

bool ComputeBestPositionVoronoiBoost(AgentsSource* src, std::vector<Pedestrian*>& peds,
                                     Building* building, std::vector<Pedestrian*>& peds_queue)
{
    bool return_value = true;
    auto dist = src->GetStartDistribution();
    int roomID = dist->GetRoomId();
    int subroomID = dist->GetSubroomID();
    // std::string caption = (building->GetRoom( roomID ))->GetCaption();

    std::vector<Pedestrian*> existing_peds;
    std::vector<Pedestrian*> peds_without_place;
    building->GetPedestrians(roomID, subroomID, existing_peds);
    existing_peds.insert(existing_peds.end(), peds_queue.begin(), peds_queue.end());

    double radius = 0.3; //radius of a person, 0.3 is just some number(needed for the fake_peds bellow). should be changed afterwards

    SubRoom* subroom = building->GetRoom( roomID )->GetSubRoom(subroomID);
    double factor = 100;  //factor for conversion to integer for the boost voronoi

    std::vector<Point> fake_peds;
    Point temp(0,0);
    //fake_peds will be the positions of "fake" pedestrians, multiplied by factor and converted to int
    float BBxmin = src->GetBoundaries()[0];
    float BBxmax = src->GetBoundaries()[1];
    float BBymin = src->GetBoundaries()[2];
    float BBymax = src->GetBoundaries()[3];
    bool haveBB = (BBxmin != std::numeric_limits<float>::lowest()) ||
         (BBxmax != std::numeric_limits<float>::max()) ||
         (BBymin != std::numeric_limits<float>::lowest()) ||
         (BBymax != std::numeric_limits<float>::max());

    if(haveBB == false)
         for (auto vert: subroom->GetPolygon() ) //room vertices
         {
              const Point& center_pos = subroom->GetCentroid();
              temp._x = ( center_pos._x-vert._x );
              temp._y = ( center_pos._y-vert._y );
              temp = temp/temp.Norm();
              temp = temp*(radius*1.4);  //now the norm of the vector is ~r*sqrt(2), pointing to the center
              temp = temp + vert;
              temp._x = (int)(temp._x*factor);
              temp._y = (int)(temp._y*factor);
              fake_peds.push_back( temp );
         }
    else
    {
         fake_peds.push_back(Point(BBxmin,BBymin)*factor);
         fake_peds.push_back(Point(BBxmin,BBymax)*factor);
         fake_peds.push_back(Point(BBxmax,BBymin)*factor);
         fake_peds.push_back(Point(BBxmax,BBymax)*factor);
    }
    std::vector<Pedestrian*>::iterator iter_ped;
//    std::srand(0);
    for (iter_ped = peds.begin(); iter_ped != peds.end(); )
    {
         Pedestrian* ped = *iter_ped;
         radius = ped->GetEllipse().GetBmax(); //max radius of the current pedestrian
         if(existing_peds.size() == 0 )
         {
              Point center_pos = subroom->GetCentroid();
              if(haveBB)
                   center_pos = Point(0.5*(BBxmax-BBxmin), 0.5*(BBymax-BBymin));

              double x_coor = 3 * ( (double)rand() / (double)RAND_MAX ) - 1.5;
              double y_coor = 3 * ( (double)rand() / (double)RAND_MAX ) - 1.5;
              Point random_pos(x_coor, y_coor);
              Point new_pos = center_pos + random_pos;
              //this could be better, but needs to work with any polygon - random point inside a polygon?
              bool inBox = (new_pos._x <= BBxmax) && (new_pos._x >= BBxmin) && (new_pos._y <= BBymax) && (new_pos._y >= BBymin);
              if ( subroom->IsInSubRoom( new_pos ) && inBox)
              {
                   if( IsEnoughInSubroom(subroom, new_pos, radius ) )
                   {
                        ped->SetPos(center_pos + random_pos, true);
                   }
              }
              else
              {
                    ped->SetPos(center_pos, true);
              }

              Point v;
              if (ped->GetExitLine()) {
                    v = (ped->GetExitLine()->ShortestPoint(ped->GetPos())- ped->GetPos()).Normalized();
              } else {
                    v = Point(0., 0.);
              }
              //double speed=ped->GetV0Norm();
              double speed = ped->GetEllipse().GetV0(); //@todo: some peds do not have a navline. This should not be accepted.
              v=v*speed;
              ped->SetV(v);

              existing_peds.push_back(ped);

         }//0
         else //more than one pedestrian
         {
           //it would be better to maybe have a mapping between discrete_positions and pointers to the pedestrians
           //then there would be no need to remember the velocities_vector and goal_vector
              std::vector<Point> discrete_positions;
              std::vector<Point> velocities_vector;
              std::vector<int> goal_vector;
              Point tmp(0,0);
              Point v(0,0);
              double no = 0;

              //points from double to integer
              for (const auto& eped: existing_peds)
              {
                   const Point& pos = eped->GetPos();
                   tmp._x = (int)( pos._x*factor );
                   tmp._y = (int)( pos._y*factor );
                   discrete_positions.push_back( tmp );
                   velocities_vector.push_back( eped->GetV() );
                   goal_vector.push_back( eped->GetFinalDestination() );

                   //calculating the mean, using it for the fake pedestrians
                   v += eped->GetV();
                   no++;
              }
              // sum up the weighted velocity in the loop
              v = v/no; //this is the mean of all velocities

              //adding fake people to the vector for constructing voronoi diagram
              //for (unsigned int i=0; i<subroom->GetPolygon().size(); i++ )
              for(auto fake_ped: fake_peds)
              {
                   discrete_positions.push_back( fake_ped );
                   velocities_vector.push_back( v );
                   goal_vector.push_back( -10 );
              }

              //constructing the diagram
              voronoi_diagram<double> vd;
              construct_voronoi(discrete_positions.begin(), discrete_positions.end(), &vd);
#if PLOT_VORONOI_DIAGRAM
              plotVoronoi(discrete_positions, vd, subroom, factor);
#endif
              voronoi_diagram<double>::const_vertex_iterator chosen_it = vd.vertices().begin();
              double dis = 0;
              //std::default_random_engine gen = dist->GetGenerator();
              if(!src->Greedy())
                    VoronoiBestVertexRandMax(src, discrete_positions, vd, subroom, factor, chosen_it, dis, radius);
              else
                    VoronoiBestVertexGreedy(src, discrete_positions, vd, subroom, factor, chosen_it, dis, radius);

              if( dis > 4*radius*radius)
              {
                   Point pos( chosen_it->x()/factor, chosen_it->y()/factor ); //check!

                   ped->SetPos(pos , true);
                   VoronoiAdjustVelocityNeighbour(chosen_it, ped, velocities_vector, goal_vector);
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

         }// >0


    }//for loop

    // for(auto x : existing_peds)
    //      std::cout << "-- " << x->GetPos()._x << ", " << x->GetPos()._y << "---\n";
    // getc(stdin);

    //maybe not all pedestrians could find a place, requeue them in the source
    if(peds_without_place.size()>0)
         src->AddAgentsToPool(peds_without_place);


    return return_value;
}

//gives an agent the mean velocity of his voronoi-neighbors
void VoronoiAdjustVelocityNeighbour(voronoi_diagram<double>::const_vertex_iterator& chosen_it, Pedestrian* ped,
        const std::vector<Point>& velocities_vector, const std::vector<int>& goal_vector)
{
     //finding the neighbors (nearest pedestrians) of the chosen vertex
     const voronoi_diagram<double>::vertex_type &vertex = *chosen_it;
     const voronoi_diagram<double>::edge_type *edge = vertex.incident_edge();
     double no1=0,no2=0;
     double backup_speed = 0;
     //std::size_t index;
     Point v(0,0);
     if(ped->GetExitLine() != nullptr)
          v = (ped->GetExitLine()->ShortestPoint(ped->GetPos())- ped->GetPos()).Normalized(); //the direction
     else
     {
          int gotRoute = ped->FindRoute();
          if(gotRoute < 0) printf("\nWARNING: source agent %d can not get exit\n", ped->GetID());
     }
     double speed = 0;
     do
     {
          std::size_t index = ( edge->cell() )->source_index();
          if( ped->GetFinalDestination() == goal_vector[index]  )
          {
                  no1++;
                  speed += velocities_vector[index].Norm();
          }
          else
          {
                  no2++;
                  backup_speed += velocities_vector[index].Norm();
          }
          edge = edge->rot_next();
     } while (edge != vertex.incident_edge());

     if(no1)
         speed = speed/no1;
     else
         speed = backup_speed/(no2*3.0); //just some small speed

     v = v*speed;
     ped->SetV(v);

}


//gives the voronoi vertex with max distance
//void VoronoiBestVertexMax(const std::vector<Point>& discrete_positions, const voronoi_diagram<double>& vd,
//        SubRoom* subroom, double factor, voronoi_diagram<double>::const_vertex_iterator& max_it, double& max_dis,
//        double radius)
//{
//     double dis = 0;
//     double score;
//     double max_score = -100; //calculated using distance and considering the goal
//
//
//
//     for (auto it = vd.vertices().begin(); it != vd.vertices().end(); ++it)
//     {
//          Point vert_pos( it->x()/factor, it->y()/factor );
//          if( subroom->IsInSubRoom(vert_pos) )
//               if( IsEnoughInSubroom( subroom, vert_pos, radius ) )
//               {
//                    const voronoi_diagram<double>::vertex_type &vertex = *it;
//                    const voronoi_diagram<double>::edge_type *edge = vertex.incident_edge();
//
//                    std::size_t index = ( edge->cell() )->source_index();
//                    Point p = discrete_positions[index];
//
//                    dis = ( p._x - it->x() )*( p._x - it->x() )  + ( p._y - it->y() )*( p._y - it->y() )  ;
//
//                    score = dis;
//
//
//                    //constructing the checking line
///*
//                    Point p2 = (ped->GetExitLine()->ShortestPoint(vert_pos)-vert_pos).Normalized(); //problem: ped does not have a position
//                    p2 = p2 + p2; //looking 2m in front
//                    Line check_line(vert_pos, vert_pos + p2);  //this is the first 2m of exit line
//
//                    do
//                    {
//                      //do something
//                      if( goal_vector[index]!=-3 &&  goal_vector[index]!=ped->GetFinalDestination() ) //
//                              if( check_line.IntersectionWithCircle(p,1.0) )    //0.7 because the radius is around 0.3
//                              {
//                                      score -= 100;
//                                      break;
//                              }
//
//
//                      //change edge
//                      edge = edge->rot_next();
//                      index = ( edge->cell() )->source_index();
//                      p = discrete_positions[index]/factor;
//
//                    } while( edge != vertex.incident_edge() );
//*/
//                    if(score > max_score)
//                    {
//                         max_score =score;
//                       max_dis = dis;
//                         max_it = it;
//                    }
//               }
//     }
//     //at the end, max_it is the choosen vertex, or the first vertex - max_dis=0 assures that this position will not be taken
//}

/**
 * Returns a Voronoi vertex randomly with respect to weights proportional to distances^2
 * For vertexes \f$v_i\f$  and distances \f$d_i\f$ to their surrounding seeds
 * calculate the probabilities \f$p_i\f$ as
 * \f[
 *     p_i= \frac{d_i^2}{\sum_j^n d_j^2}
 * \f]
 *
 * @param discrete_positions: agents and imaginary agents
 * @param vd: Voronoi diagram
 * @param subroom
 * @param factor: used to convert the coordinates back from int (was necessary for Boost Voronoi calculations)
 * @param chosen_it: return best_vertex
 * @param dis: distance squared of  the best_vertex to its surrouding seeds.
 * @param radius: radius of pedestrian
 */
void VoronoiBestVertexRandMax (AgentsSource* src, const std::vector<Point>& discrete_positions, const voronoi_diagram<double>& vd, SubRoom* subroom,
                               double factor, voronoi_diagram<double>::const_vertex_iterator& chosen_it, double& dis	, double radius)
{
     std::vector< voronoi_diagram<double>::const_vertex_iterator > possible_vertices;
     std::vector<double> partial_sums;
     unsigned long size=0;
     for (auto it = vd.vertices().begin(); it != vd.vertices().end(); ++it){
          Point vert_pos = Point( it->x()/factor, it->y()/factor );
          float BBxmin = src->GetBoundaries()[0];
          float BBxmax = src->GetBoundaries()[1];
          float BBymin = src->GetBoundaries()[2];
          float BBymax = src->GetBoundaries()[3];
          bool inBox = (vert_pos._x <= BBxmax) && (vert_pos._x >= BBxmin) && (vert_pos._y <= BBymax) && (vert_pos._y >= BBymin);
          if( subroom->IsInSubRoom( vert_pos ) && inBox)
               if( IsEnoughInSubroom(subroom, vert_pos,radius) )
               {
                    const voronoi_diagram<double>::vertex_type &vertex = *it;
                    const voronoi_diagram<double>::edge_type *edge = vertex.incident_edge();
                    std::size_t index = ( edge->cell() )->source_index();
                    Point p = discrete_positions[index];

                    dis = ( p._x - it->x() )*( p._x - it->x() )   + ( p._y - it->y() )*( p._y - it->y() ) ;
                    dis = dis / factor / factor;
                    possible_vertices.push_back( it );
                    partial_sums.push_back( dis );

                    size = partial_sums.size();
                    if( size > 1 )
                    {
                         partial_sums[ size - 1 ] += partial_sums[ size - 2 ];
                    }
               }
     }
     // partial_sums: [d_0^2,  d_0^2 + d_1^2,  d_0^2 + d_1^2 + d_2^2, ..., \sum_i^{n-1} d_i^3]
     //now we have the vector of possible vertices and weights and we can choose one randomly

     if(partial_sums.empty())
     {
          Log->Write("Warning: No possible vertices. Maybe BB too small for %d agents?", src->GetChunkAgents());
 //         exit(EXIT_FAILURE); // maybe not exit, just ignore
          // dis = 0;
          return;
     }

     double lower_bound = 0;
     double upper_bound = partial_sums[size-1];
     std::random_device rd;
     //std::mt19937 gen(rd()); //@todo use seed instead of rd(). Generator should not be here
     std::mt19937 gen(1); //@todo use seed instead of rd(). Generator should not be here
     std::uniform_real_distribution<double> distribution(lower_bound, upper_bound); //std::nextafter(upper_bound, DBL_MAX));
     std::vector<double> random_numbers;
     for(unsigned int r=0; r<size;r++)
           random_numbers.push_back(distribution(gen));

     shuffle(random_numbers.begin(), random_numbers.end(), gen);
     double a_random_double = random_numbers[0];

     //the first element in the range [first, last) that is not less than a_random_double
     auto lower = std::lower_bound(partial_sums.begin(), partial_sums.end(), a_random_double);
     int iposition = lower - partial_sums.begin();
     // if iposition == size then no element is found. Should not happen..
     chosen_it = possible_vertices[iposition];
     dis = partial_sums[iposition];
     if (iposition>1)
           dis -= partial_sums[iposition-1];
}


/**
 * Returns a Voronoi vertex with the largest distances^2
 * For vertexes \f$v_i\f$  and distances \f$d_i\f$ to their surrounding seeds
 * Use the "greedy" approach by choosing the best_vertex as the
 * vertex with the biggest distance to the surrounding seeds
 *
 * @param discrete_positions: agents and imaginary agents
 * @param vd: Voronoi diagram
 * @param subroom
 * @param factor: used to convert the coordinates back from int (was necessary for Boost Voronoi calculations)
 * @param chosen_it: return best_vertex
 * @param dis: distance squared of  the best_vertex to its surrouding seeds.
 * @param radius: radius of pedestrian
 */
void VoronoiBestVertexGreedy (AgentsSource* src, const std::vector<Point>& discrete_positions, const voronoi_diagram<double>& vd, SubRoom* subroom,
                              double factor, voronoi_diagram<double>::const_vertex_iterator& chosen_it, double& dis	, double radius)
{
     std::vector< voronoi_diagram<double>::const_vertex_iterator > possible_vertices;
     std::vector<double> distances;
     for (auto it = vd.vertices().begin(); it != vd.vertices().end(); ++it){
          Point vert_pos = Point( it->x()/factor, it->y()/factor );
          float BBxmin = src->GetBoundaries()[0];
          float BBxmax = src->GetBoundaries()[1];
          float BBymin = src->GetBoundaries()[2];
          float BBymax = src->GetBoundaries()[3];
          bool inBox = (vert_pos._x <= BBxmax) && (vert_pos._x >= BBxmin) && (vert_pos._y <= BBymax) && (vert_pos._y >= BBymin);
          if( subroom->IsInSubRoom( vert_pos ) && inBox )
               if( IsEnoughInSubroom(subroom, vert_pos, radius) )
               {
                    const voronoi_diagram<double>::vertex_type &vertex = *it;
                    const voronoi_diagram<double>::edge_type *edge = vertex.incident_edge();
                    std::size_t index = ( edge->cell() )->source_index();
                    Point p = discrete_positions[index];
                    dis = ( p._x - it->x() )*( p._x - it->x() )   + ( p._y - it->y() )*( p._y - it->y() ) ;
                    dis = dis / factor / factor;
                    possible_vertices.push_back( it );
                    distances.push_back( dis );
               }
     }
     if(possible_vertices.empty())
     {
          Log->Write("Warning: No possible vertices. Maybe BB too small for %d agents?", src->GetChunkAgents());
 //         exit(EXIT_FAILURE); // maybe not exit, just ignore
          dis = 0;
    return;
     }

     auto biggest = std::max_element(distances.begin(), distances.end());
     int iposition = biggest - distances.begin(); // first biggest distance
     chosen_it = possible_vertices[iposition];
     dis = distances[iposition];
}


//gives a random voronoi vertex
//void VoronoiBestVertexRand (const std::vector<Point>& discrete_positions, const voronoi_diagram<double>& vd, SubRoom* subroom,
//          double factor, voronoi_diagram<double>::const_vertex_iterator& chosen_it, double& dis, double radius	)
//{
//     std::vector< voronoi_diagram<double>::const_vertex_iterator > possible_vertices;
//     std::vector<double> distances;
//
//     for (auto it = vd.vertices().begin(); it != vd.vertices().end(); ++it)
//     {
//          Point vert_pos = Point( it->x()/factor, it->y()/factor );
//          if( subroom->IsInSubRoom(vert_pos) )
//               if( IsEnoughInSubroom(subroom, vert_pos, radius) )
//               {
//                    const voronoi_diagram<double>::vertex_type &vertex = *it;
//                    const voronoi_diagram<double>::edge_type *edge = vertex.incident_edge();
//
//                    std::size_t index = ( edge->cell() )->source_index();
//                    Point p = discrete_positions[index];
//
//                    dis = ( p._x - it->x() )*( p._x - it->x() )   + ( p._y - it->y() )*( p._y - it->y() )  ;
//
//                    possible_vertices.push_back( it );
//                    distances.push_back( dis );
//               }
//     }
//     //now we have all the possible vertices and their distances and we can choose one randomly
//     //TODO: get the seed from the simulation/argumentparser
//     //srand (time(NULL));
//     unsigned int i = rand() % possible_vertices.size();
//     chosen_it = possible_vertices[i];
//     dis = distances[i];
//
//}



void plotVoronoi(const std::vector<Point>& discrete_positions, const voronoi_diagram<double>& vd, SubRoom* subroom, double factor)
{
     // =============== plot Voronoi Diagram =====================
     char name [50];
     sprintf(name,  "log_%.3d.py", global_count);
     FILE * f;
     f = fopen(name, "w");
    // plot cells
     fprintf(f, "# ------------------------------\n");
     fprintf(f, "import matplotlib.pyplot as plt\n");
     //  plot seeds
     for(auto pos : discrete_positions)
     {
           fprintf(f, "plt.plot([%f], [%f], \"or\")\n", pos._x/factor, pos._y/factor);
     }
// plot cells
     for (auto it = vd.cells().begin(); it != vd.cells().end(); ++it){
           const voronoi_diagram<double>::cell_type &cell = *it;
           const voronoi_diagram<double>::edge_type *edge = cell.incident_edge();
         do {
                 if(edge->vertex0() && edge->vertex1())
                 {
                       fprintf(f, "plt.plot([%f, %f], [%f, %f], \"bo-\", lw=2)\n",  edge->vertex0()->x()/factor, edge->vertex1()->x()/factor, edge->vertex0()->y()/factor, edge->vertex1()->y()/factor);
                 }

              edge = edge->next();
        } while (edge != cell.incident_edge());
     }
// plot geometry
     double max_x=std::numeric_limits<double>::min(), min_x=std::numeric_limits<double>::max();
     double max_y=std::numeric_limits<double>::min(), min_y=std::numeric_limits<double>::max();
     const std::vector<Point> polygon = subroom->GetPolygon();
     for(auto it = polygon.begin(); it != polygon.end(); ) {
           Point gpoint = *(it++);
           // Point gpointNext = *it;
           if(gpoint._x > max_x)
                 max_x = gpoint._x;
           if(gpoint._y > max_y)
                 max_y = gpoint._y;
           if(gpoint._x < min_x)
                 min_x = gpoint._x;
           if(gpoint._y < min_y)
                 min_y = gpoint._y;

           // fprintf(f, "plt.plot([%f, %f], [%f, %f], \"k-\", lw=2)\n",  gpoint._x, gpointNext._x, gpoint._y, gpointNext._y);
           if(it == polygon.end()){
                 // fprintf(f, "plt.plot([%f, %f], [%f, %f], \"k-\", lw=2)\n",  gpointNext._x, polygon.begin()->_x,  gpointNext._y, polygon.begin()->_y );
                 break;
           }
     }
     double eps=0.0;
     fprintf(f, "plt.xlim([%f, %f])\n", min_x-eps, max_x+eps);
     fprintf(f, "plt.ylim([%f, %f])\n", min_y-eps, max_y+eps);
     fprintf(f, "plt.title(\"agents = %3d\")\n", (int)discrete_positions.size());
     fprintf(f, "plt.savefig(\"%.4d.png\", dpi=600)\n", global_count++);

     fclose(f);
}

Point ComputeBestPositionVoronoiBoost(Pedestrian* ped, SubRoom* subRoom, std::vector<Pedestrian*> existingPeds)
{
     // some ids
//     int pedID = ped->GetID();
//     int roomID = ped->GetRoomID();
//     int subroomID = ped->GetSubRoomID();
//
//     std::cout << "Ped:" << std::endl;
//     std::cout << ped->GetID() << std::endl;
//     std::cout << ped->GetBuilding() << std::endl;
//     std::cout << "Rooms:" << std::endl;
//
//     for (auto room: ped->GetBuilding()->GetAllRooms()){
//          std::cout << room.second << std::endl;
//     }
     double radius = 0.3; //radius of a person, 0.3 is just some number(needed for the fake_peds bellow). should be changed afterwards
//     SubRoom* subRoom = ped->GetBuilding()->GetRoom( roomID )->GetSubRoom(subroomID);
     double factor = 100;  //factor for conversion to integer for the boost voronoi

     std::vector<Point> fakePeds;
     Point temp(0,0);
     //fake_peds will be the positions of "fake" pedestrians, multiplied by factor and converted to int
     double xMin = std::numeric_limits<double>::max(),
               xMax =std::numeric_limits<double>::min(),
               yMin = std::numeric_limits<double>::max(),
               yMax = std::numeric_limits<double>::min();

     for (auto poly : subRoom->GetPolygon()){
          xMin = (xMin <= poly._x)?(xMin):(poly._x);
          xMax = (xMax >= poly._x)?(xMax):(poly._x);

          yMin = (yMin <= poly._y)?(yMin):(poly._y);
          yMax = (yMax >= poly._y)?(yMax):(poly._y);
     }

     bool haveBB = (xMin != std::numeric_limits<float>::lowest()) ||
               (xMax != std::numeric_limits<float>::max()) ||
               (yMin != std::numeric_limits<float>::lowest()) ||
               (yMax != std::numeric_limits<float>::max());

     if(haveBB == false)
          for (auto vert: subRoom->GetPolygon() ) //room vertices
          {
               const Point& center_pos = subRoom->GetCentroid();
               temp._x = ( center_pos._x-vert._x );
               temp._y = ( center_pos._y-vert._y );
               temp = temp/temp.Norm();
               temp = temp*(radius*1.4);  //now the norm of the vector is ~r*sqrt(2), pointing to the center
               temp = temp + vert;
               temp._x = (int)(temp._x*factor);
               temp._y = (int)(temp._y*factor);
               fakePeds.push_back( temp );
          }
     else
     {
          fakePeds.push_back(Point(xMin, yMin)*factor);
          fakePeds.push_back(Point(xMin, yMax)*factor);
          fakePeds.push_back(Point(xMax, yMin)*factor);
          fakePeds.push_back(Point(xMax, yMax)*factor);
     }

     // Get existing peds
//     std::vector<Pedestrian*> existingPeds;
//     ped->GetBuilding()->GetPedestrians(roomID, subroomID, existingPeds);

     for (auto ped : fakePeds){
          std::cout << "fake_ped at " << ped.toString() << std::endl;
     }

     for (auto ped : existingPeds){
          std::cout << "existing_ped at " << ped->GetPos().toString() << std::endl;
     }

     std::vector<Point> discretePositions;
     discretePositions.insert( discretePositions.end(), fakePeds.begin(), fakePeds.end() );
     for (auto p : existingPeds){
          if (ped->GetID() != p->GetID()){
               discretePositions.push_back(ped->GetPos()*factor);
          }
     }

     for (auto dp : discretePositions){
          std::cout << "dp: " << dp.toString() << std::endl;
     }

     voronoi_diagram<double> vd;
     construct_voronoi(discretePositions.begin(), discretePositions.end(), &vd);
     plotVoronoi(discretePositions, vd, subRoom, factor);

     voronoi_diagram<double>::const_vertex_iterator chosenIt = vd.vertices().begin();
     double dis = 0;

     Point pos;

     do{
          VoronoiBestVertexRand(discretePositions, vd, subRoom, factor, chosenIt, dis, radius);
          pos = Point( chosenIt->x()/factor, chosenIt->y()/factor ); //check!
          std::cout << "Pos: " << pos.toString() << std::endl;
     }while(!subRoom->IsInSubRoom(pos));

     return pos;


}

void VoronoiBestVertexRand (const std::vector<Point>& discrete_positions, const voronoi_diagram<double>& vd, SubRoom* subroom, double factor,
          voronoi_diagram<double>::const_vertex_iterator& max_it, double& max_dis, double radius	)
{
     std::vector< voronoi_diagram<double>::const_vertex_iterator > possible_vertices;
     std::vector<double> partial_sums;
     unsigned long size=0;
     std::vector<double> bounding_box = subroom->GetBoundingBox();
     for (auto it = vd.vertices().begin(); it != vd.vertices().end(); ++it){
          Point vert_pos = Point( it->x()/factor, it->y()/factor );
          float BBxmin = bounding_box[0];
          float BBxmax = bounding_box[1];
          float BBymin = bounding_box[2];
          float BBymax = bounding_box[3];
          bool inBox = (vert_pos._x <= BBxmax) && (vert_pos._x >= BBxmin) && (vert_pos._y <= BBymax) && (vert_pos._y >= BBymin);
          if( subroom->IsInSubRoom( vert_pos ) && inBox)
               if( IsEnoughInSubroom(subroom, vert_pos,radius) )
               {
                    const voronoi_diagram<double>::vertex_type &vertex = *it;
                    const voronoi_diagram<double>::edge_type *edge = vertex.incident_edge();
                    std::size_t index = ( edge->cell() )->source_index();
                    Point p = discrete_positions[index];

                    max_dis = ( p._x - it->x() )*( p._x - it->x() )   + ( p._y - it->y() )*( p._y - it->y() ) ;
                    max_dis = max_dis / factor / factor;
                    possible_vertices.push_back( it );
                    partial_sums.push_back( max_dis );

                    size = partial_sums.size();
                    if( size > 1 )
                    {
                         partial_sums[ size - 1 ] += partial_sums[ size - 2 ];
                    }
               }
     }
     // partial_sums: [d_0^2,  d_0^2 + d_1^2,  d_0^2 + d_1^2 + d_2^2, ..., \sum_i^{n-1} d_i^3]
     //now we have the vector of possible vertices and weights and we can choose one randomly

     if(partial_sums.empty())
     {
          Log->Write("Warning: No possible vertices. Maybe BB too small?");
          //         exit(EXIT_FAILURE); // maybe not exit, just ignore
          // dis = 0;
          return;
     }

     double lower_bound = 0;
     double upper_bound = partial_sums[size-1];
     std::random_device rd;
     //std::mt19937 gen(rd()); //@todo use seed instead of rd(). Generator should not be here
     std::mt19937 gen(1); //@todo use seed instead of rd(). Generator should not be here
     std::uniform_real_distribution<double> distribution(lower_bound, upper_bound); //std::nextafter(upper_bound, DBL_MAX));
     std::vector<double> random_numbers;
     for(unsigned int r=0; r<size;r++)
          random_numbers.push_back(distribution(gen));

     shuffle(random_numbers.begin(), random_numbers.end(), gen);
     double a_random_double = random_numbers[0];

     //the first element in the range [first, last) that is not less than a_random_double
     auto lower = std::lower_bound(partial_sums.begin(), partial_sums.end(), a_random_double);
     int iposition = lower - partial_sums.begin();
     // if iposition == size then no element is found. Should not happen..
     max_it = possible_vertices[iposition];
     max_dis = partial_sums[iposition];
     if (iposition>1)
          max_dis -= partial_sums[iposition-1];

}
