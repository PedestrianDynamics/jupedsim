//
// Created by Tobias Schrödter on 2019-05-14.
//

#include "WaitingVoronoi.h"

#include "geometry/Room.h"
#include "geometry/SubRoom.h"
#include "pedestrian/Pedestrian.h"
#include "geometry/Point.h"

#include "voronoi-boost/VoronoiPositionGenerator.h"


Point WaitingVoronoi::GetWaitingPosition(Room* room, Pedestrian* ped) const{

     int roomID = ped->GetRoomID();
     int subroomID = ped->GetSubRoomID();
     SubRoom* subRoom = ped->GetBuilding()->GetRoom( roomID )->GetSubRoom(subroomID);

     std::vector<Pedestrian*> existingPeds;
     ped->GetBuilding()->GetPedestrians(roomID, subroomID, existingPeds);

     return ComputeBestPositionVoronoiBoost(ped, subRoom, existingPeds);

//     double radius = 0.3; //radius of a person, 0.3 is just some number(needed for the fake_peds bellow). should be changed afterwards
//
//     double factor = 100;  //factor for conversion to integer for the boost voronoi
//
//     std::vector<Point> fake_peds;
//     Point temp(0,0);
//     //fake_peds will be the positions of "fake" pedestrians, multiplied by factor and converted to int
//     double xMin = std::numeric_limits<double>::max(),
//               xMax =std::numeric_limits<double>::min(),
//               yMin = std::numeric_limits<double>::max(),
//               yMax = std::numeric_limits<double>::min();
//
//     for (auto poly : subRoom->GetPolygon()){
//          xMin = (xMin <= poly._x)?(xMin):(poly._x);
//          xMax = (xMax >= poly._x)?(xMax):(poly._x);
//
//          yMin = (yMin <= poly._y)?(yMin):(poly._y);
//          yMax = (yMax >= poly._y)?(yMax):(poly._y);
//     }
//
//     bool haveBB = (xMin != std::numeric_limits<float>::lowest()) ||
//               (xMax != std::numeric_limits<float>::max()) ||
//               (yMin != std::numeric_limits<float>::lowest()) ||
//               (yMax != std::numeric_limits<float>::max());
//
//     if(haveBB == false)
//          for (auto vert: subRoom->GetPolygon() ) //room vertices
//          {
//               const Point& center_pos = subRoom->GetCentroid();
//               temp._x = ( center_pos._x-vert._x );
//               temp._y = ( center_pos._y-vert._y );
//               temp = temp/temp.Norm();
//               temp = temp*(radius*1.4);  //now the norm of the vector is ~r*sqrt(2), pointing to the center
//               temp = temp + vert;
//               temp._x = (int)(temp._x*factor);
//               temp._y = (int)(temp._y*factor);
//               fake_peds.push_back( temp );
//          }
//     else
//     {
//          fake_peds.push_back(Point(xMin, yMin)*factor);
//          fake_peds.push_back(Point(xMin, yMax)*factor);
//          fake_peds.push_back(Point(xMax, yMin)*factor);
//          fake_peds.push_back(Point(xMax, yMax)*factor);
//     }
//     std::vector<Pedestrian*>::iterator iter_ped;
//
//
//     for (auto ped : fake_peds){
//          std::cout << "fake_ped at " << ped.toString() << std::endl;
//     }
//
//     for (auto ped : existing_peds){
//          std::cout << "existing_ped at " << ped->GetPos().toString() << std::endl;
//     }
//
//     std::vector<Point> discrete_positions;
//     discrete_positions.insert( discrete_positions.end(), fake_peds.begin(), fake_peds.end() );
//     for (auto p : existing_peds){
//          if (ped->GetID() != p->GetID()){
//               discrete_positions.push_back(ped->GetPos()*factor);
//          }
//     }
//
//     for (auto dp : discrete_positions){
//          std::cout << "dp: " << dp.toString() << std::endl;
//     }
//
//     return subRoom->GetCentroid();
//
////     voronoi_diagram<double> vd;
////     construct_voronoi(discrete_positions.begin(), discrete_positions.end(), &vd);
////     plotVoronoi(discrete_positions, vd, subroom, factor);
//
//
////     for (iter_ped = peds.begin(); iter_ped != peds.end(); )
////     {
////          Pedestrian* ped = *iter_ped;
////          radius = ped->GetEllipse().GetBmax(); //max radius of the current pedestrian
////          if(existing_peds.size() == 0 )
////          {
////               Point center_pos = subroom->GetCentroid();
////               if(haveBB)
////                    center_pos = Point(0.5*(BBxmax-BBxmin), 0.5*(BBymax-BBymin));
////
////               double x_coor = 3 * ( (double)rand() / (double)RAND_MAX ) - 1.5;
////               double y_coor = 3 * ( (double)rand() / (double)RAND_MAX ) - 1.5;
////               Point random_pos(x_coor, y_coor);
////               Point new_pos = center_pos + random_pos;
////               //this could be better, but needs to work with any polygon - random point inside a polygon?
////               bool inBox = (new_pos._x <= BBxmax) && (new_pos._x >= BBxmin) && (new_pos._y <= BBymax) && (new_pos._y >= BBymin);
////               if ( subroom->IsInSubRoom( new_pos ) && inBox)
////               {
////                    if( IsEnoughInSubroom(subroom, new_pos, radius ) )
////                    {
////                         ped->SetPos(center_pos + random_pos, true);
////                    }
////               }
////               else
////               {
////                    ped->SetPos(center_pos, true);
////               }
////
////               Point v;
////               if (ped->GetExitLine()) {
////                    v = (ped->GetExitLine()->ShortestPoint(ped->GetPos())- ped->GetPos()).Normalized();
////               } else {
////                    v = Point(0., 0.);
////               }
////               //double speed=ped->GetV0Norm();
////               double speed = ped->GetEllipse().GetV0(); //@todo: some peds do not have a navline. This should not be accepted.
////               v=v*speed;
////               ped->SetV(v);
////
////               existing_peds.push_back(ped);
////
////          }//0
////          else //more than one pedestrian
////          {
////               //it would be better to maybe have a mapping between discrete_positions and pointers to the pedestrians
////               //then there would be no need to remember the velocities_vector and goal_vector
////               std::vector<Point> discrete_positions;
////               std::vector<Point> velocities_vector;
////               std::vector<int> goal_vector;
////               Point tmp(0,0);
////               Point v(0,0);
////               double no = 0;
////
////               //points from double to integer
////               for (const auto& eped: existing_peds)
////               {
////                    const Point& pos = eped->GetPos();
////                    tmp._x = (int)( pos._x*factor );
////                    tmp._y = (int)( pos._y*factor );
////                    discrete_positions.push_back( tmp );
////                    velocities_vector.push_back( eped->GetV() );
////                    goal_vector.push_back( eped->GetFinalDestination() );
////
////                    //calculating the mean, using it for the fake pedestrians
////                    v += eped->GetV();
////                    no++;
////               }
////               // sum up the weighted velocity in the loop
////               v = v/no; //this is the mean of all velocities
////
////               //adding fake people to the vector for constructing voronoi diagram
////               //for (unsigned int i=0; i<subroom->GetPolygon().size(); i++ )
////               for(auto fake_ped: fake_peds)
////               {
////                    discrete_positions.push_back( fake_ped );
////                    velocities_vector.push_back( v );
////                    goal_vector.push_back( -10 );
////               }
////
////               //constructing the diagram
////               voronoi_diagram<double> vd;
////               construct_voronoi(discrete_positions.begin(), discrete_positions.end(), &vd);
////#if PLOT_VORONOI_DIAGRAM
////               plotVoronoi(discrete_positions, vd, subroom, factor);
////#endif
////               voronoi_diagram<double>::const_vertex_iterator chosen_it = vd.vertices().begin();
////               double dis = 0;
////               //std::default_random_engine gen = dist->GetGenerator();
////               if(!src->Greedy())
////                    VoronoiBestVertexRandMax(src, discrete_positions, vd, subroom, factor, chosen_it, dis, radius);
////               else
////                    VoronoiBestVertexGreedy(src, discrete_positions, vd, subroom, factor, chosen_it, dis, radius);
////
////               if( dis > 4*radius*radius)
////               {
////                    Point pos( chosen_it->x()/factor, chosen_it->y()/factor ); //check!
////
////                    ped->SetPos(pos , true);
////                    VoronoiAdjustVelocityNeighbour(chosen_it, ped, velocities_vector, goal_vector);
////                    // proceed to the next pedestrian
////                    existing_peds.push_back(ped);
////                    ++iter_ped;
////               }
////               else
////               {
////                    //reject the pedestrian:
////                    return_value = false;
////                    peds_without_place.push_back(*iter_ped); //Put in a different queue, they will be put back in the source.
////                    iter_ped=peds.erase(iter_ped); // remove from the initial vector since it should only contain the pedestrians that could find a place
////               }
////
////          }// >0
////
////
////     }//for loop
////
////     // for(auto x : existing_peds)
////     //      std::cout << "-- " << x->GetPos()._x << ", " << x->GetPos()._y << "---\n";
////     // getc(stdin);
////
////     //maybe not all pedestrians could find a place, requeue them in the source
////     if(peds_without_place.size()>0)
////          src->AddAgentsToPool(peds_without_place);
//     return Point(0,0);
}
