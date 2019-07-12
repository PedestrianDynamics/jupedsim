/**
 * \file        DirectionStrategy.cpp
 * \date        Dec 13, 2010
 * \version     v0.7
 * \copyright   <2009-2015> Forschungszentrum Jülich GmbH. All rights reserved.
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
 *
 *
 **/

//#include "DirectionStrategy.h"
#include "../geometry/Line.h"
#include "../geometry/NavLine.h"
#include "../geometry/Building.h"
//#include "../geometry/Room.h"
#include "../pedestrian/Pedestrian.h"
#include "../geometry/SubRoom.h"
//#include "../geometry/Wall.h"
#include "../routing/ff_router/UnivFFviaFM.h"
#include "../routing/ff_router/FloorfieldViaFM.h"
#include "../routing/ff_router/ffRouter.h"
//#include <fstream>
//#include <ctime>
#include <chrono>

DirectionStrategy::DirectionStrategy()
{
}


DirectionStrategy::~DirectionStrategy()
{
}

double DirectionStrategy::GetDistance2Wall(Pedestrian* /*ped*/) const
{
     return -1.;
}
double DirectionStrategy::GetDistance2Target(Pedestrian* /*ped*/, int /*UID*/)
{
     return -1.;
}

/// 1
Point DirectionMiddlePoint::GetTarget(Room* /*room*/, Pedestrian* ped) const
{
    return (ped->GetExitLine()->GetPoint1() + ped->GetExitLine()->GetPoint2())*0.5;
}
/// 2
Point DirectionMinSeperationShorterLine::GetTarget(Room* /*room*/, Pedestrian* ped) const
{
     double d = ped->GetEllipse().GetBmin() + 0.1 ; // shoulder//0.5;
     const Point& p1 = ped->GetExitLine()->GetPoint1();
     const Point& p2 = ped->GetExitLine()->GetPoint2();
     if(p1 == p2) return p1;

     double length = (p1 - p2).Norm();
     if(d >= 0.5*length) return (p1 + p2)*0.5; // return the middle point, since line is anyway too short
     double u = d/length; // d is supposed to be smaller than length, then u is in [0, 1]
     //Point diff = (p1 - p2).Normalized() * d;
     Line e_neu = Line(p1 + (p2-p1)*u, p1 + (p2-p1)*(1-u), 0);
     Point target = e_neu.ShortestPoint(ped->GetPos());
     // if(ped->GetID() == 81)
     // {
      // printf("=======\nX=[%.2f], Y=[%.2f]\n", ped->GetPos().GetX(), ped->GetPos().GetY());
      //     printf("p1=[%.2f, %.2f], p2=[%.2f, %.2f]\n", p1.GetX(), p1.GetY(), p2.GetX(), p2.GetY());
      //     printf("u= %f length=%f\n", u, length);
      //     printf("e_neu=[%.2f, %.2f]===[%.2f, %.2f]\n", e_neu.GetPoint1().GetX(), e_neu.GetPoint1().GetY(), e_neu.GetPoint2().GetX(), e_neu.GetPoint2().GetY() );

     // printf("target=[%.2f, %.2f]\n====\n\n", target.GetX(), target.GetY());
     //getc(stdin);
     //}
     // kürzester Punkt auf der Linie

     return  target;

}
/// 3
Point DirectionInRangeBottleneck::GetTarget(Room* /*room*/, Pedestrian* ped) const
{
    const Point& p1 = ped->GetExitLine()->GetPoint1();
    const Point& p2 = ped->GetExitLine()->GetPoint2();
    Line ExitLine = Line(p1, p2, 0);
    Point Lot = ExitLine.LotPoint( ped->GetPos() );
    Point ExitMiddle = (p1+p2)*0.5;
    double d = 0.2;
    // fix when using triangulation to avoid steering too near to the walls
    // double lenSq = ExitLine.LengthSquare();
    // if (lenSq >= 19 && lenSq < 50)
    //   d = 1;
    // else if(lenSq >= 50 &&lenSq < 100)
    //   d = 2;
    // else if(lenSq >= 100)
    //   d = 3;


    Point diff = (p1 - p2).Normalized() * d;
    Line e_neu = Line(p1 - diff, p2 + diff, 0);

    // if(ped->GetID() == )
    // {
    //     printf("=======\nX=[%.2f], Y=[%.2f]\n", ped->GetPos().GetX(), ped->GetPos().GetY());
    //     printf("p1=[%.2f, %.2f], p2=[%.2f, %.2f]\n", p1.GetX(), p1.GetY(), p2.GetX(), p2.GetY());
    //     printf("e_neu=[%.2f, %.2f]===[%.2f, %.2f]\n", e_neu.GetPoint1().GetX(), e_neu.GetPoint1().GetY(), e_neu.GetPoint2().GetX(), e_neu.GetPoint2().GetY() );
    // }

    if ( e_neu.IsInLineSegment(Lot) ) {
        // if(ped->GetID() == -10){
        //     printf("Return Lot=[%.2f, %.2f]\n", Lot.GetX(), Lot.GetY() );
        //     if(0 && ped->GetPos().GetX() > 56)
        //         getc(stdin);}
        return Lot;
    } else {
        // if(ped->GetID() == -10){
        //     printf("Return Middle=[%.2f, %.2f]\n", ExitMiddle.GetX(), ExitMiddle.GetY() );
        //     if(0 && ped->GetPos().GetX() > 56)
        //         getc(stdin);}
        return ExitMiddle;
    }

}



/// 4
Point DirectionGeneral::GetTarget(Room* room, Pedestrian* ped) const
{
#define DEBUG 0
      using namespace std;
      const Point& p1 = ped->GetExitLine()->GetPoint1();
      const Point& p2 = ped->GetExitLine()->GetPoint2();
      Line ExitLine = Line(p1, p2, 0);
      //Point Lot = ExitLine.LotPoint( ped->GetPos() );
      double d = 0.2; //shorten the line by  20 cm
      Point diff = (p1 - p2).Normalized() * d;
      Line e_neu = Line(p1 - diff, p2 + diff, 0);
      Point NextPointOnLine =  e_neu.ShortestPoint(ped->GetPos());

      Line tmpDirection = Line(ped->GetPos(), NextPointOnLine, 0);//This direction will be rotated if
      // it intersects a wall || obstacle.
      // check for intersection with walls
      // @todo: make a FUNCTION of this

#if DEBUG
      printf("\n----------\nEnter GetTarget() with PED=%d\n----------\n",ped->GetID());
      printf("nextPointOn Line: %f %f\n", NextPointOnLine._x, NextPointOnLine._y);
#endif
      double dist;
      int inear = -1;
      int iObs = -1;
      double minDist = 20001;
      int subroomId = ped->GetSubRoomID();
      SubRoom * subroom = room->GetSubRoom(subroomId);

      //============================ WALLS ===========================
      const vector<Wall>& walls = subroom->GetAllWalls();
      for (unsigned int i = 0; i < walls.size(); i++) {
            dist = tmpDirection.GetDistanceToIntersectionPoint(walls[i]);
            if (dist < minDist) {
                  inear = i;
                  minDist = dist;

#if DEBUG
                  printf("Check wall number %d. Dist = %f (%f)\n", i, dist, minDist);
                  printf("%f    %f --- %f    %f\n===========\n",walls[i].GetPoint1()._x,walls[i].GetPoint1()._y, walls[i].GetPoint2()._x,walls[i].GetPoint2()._y);
#endif

            }
      }//walls

      //============================ WALLS ===========================


      //============================ OBST ===========================
      const vector<Obstacle*>& obstacles = subroom->GetAllObstacles();
      for(unsigned int obs=0; obs<obstacles.size(); ++obs) {
            const vector<Wall>& owalls = obstacles[obs]->GetAllWalls();
            for (unsigned int i = 0; i < owalls.size(); i++) {
                  dist = tmpDirection.GetDistanceToIntersectionPoint(owalls[i]);
                  if (dist < minDist) {
                        inear = i;
                        minDist = dist;
                        iObs = obs;
#if DEBUG
                        printf("Check OBS:obs=%d, i=%d Dist = %f (%f)\n", obs, i, dist, minDist);
                        printf("%f    %f --- %f    %f\n===========\n",owalls[i].GetPoint1()._x,owalls[i].GetPoint1()._y, owalls[i].GetPoint2()._x,owalls[i].GetPoint2()._y);
#endif
                  }
            }//walls of obstacle
      }// obstacles
      //============================ OBST ===========================


      double angle = 0;
      if (inear >= 0) {
            ped->SetNewOrientationFlag(true); //Mark this pedestrian for next target calculation
            ped->SetDistToBlockade(minDist);
            if(iObs >= 0){ // obstacle is nearest
                  const vector<Wall>& owalls = obstacles[iObs]->GetAllWalls();
                  angle = tmpDirection.GetObstacleDeviationAngle(owalls, walls);

                  // angle =  tmpDirection.GetDeviationAngle(owalls[inear].enlarge(2*ped->GetLargerAxis()));

#if DEBUG
                  printf("COLLISION WITH OBSTACLE %f    %f --- %f    %f\n===========\n",owalls[inear].GetPoint1()._x,owalls[inear].GetPoint1()._y, owalls[inear].GetPoint2()._x,owalls[inear].GetPoint2()._y);

#endif
            } //iObs
            else{ // wall is nearest
                  angle =  tmpDirection.GetDeviationAngle(walls[inear].Enlarge(2*ped->GetLargerAxis()));

#if DEBUG
                  printf("COLLISION WITH WALL %f    %f --- %f    %f\n===========\n",walls[inear].GetPoint1()._x,walls[inear].GetPoint1()._y, walls[inear].GetPoint2()._x,walls[inear].GetPoint2()._y);
#endif
            } //else
      }//inear
      else{

            if(ped->GetNewOrientationFlag()){ //this pedestrian could not see the target and now he can see it clearly.
                  // printf("ped->GetNewOrientationFlag()=%d\n",ped->GetNewOrientationFlag());getc(stdin);
                  ped->SetSmoothTurning(); // so the turning should be adapted accordingly.
                  ped->SetNewOrientationFlag(false);
            }
      }
////////////////////////////////////////////////////////////


      Point  G;
      if (fabs(angle) > J_EPS)
            //G  =  tmpDirection.GetPoint2().Rotate(cos(angle), sin(angle)) ;
            G  = (NextPointOnLine-ped->GetPos()).Rotate(cos(angle), sin(angle))+ped->GetPos() ;
      else {
            if(ped->GetNewOrientationFlag()) //this pedestrian could not see the target and now he can see it clearly.
                  ped->SetSmoothTurning(); // so the turning should be adapted accordingly.

            G  =  NextPointOnLine;
      }

#if DEBUG
      printf("inear=%d, iObs=%d, minDist=%f\n", inear, iObs, minDist);
      printf("PED=%d\n",  ped->GetID());
      printf ("MC Posx = %.2f, Posy=%.2f, Lot=[%.2f, %.2f]\n", ped->GetPos()._x, ped->GetPos()._y, NextPointOnLine._x, NextPointOnLine._y);
      printf("MC p1=[%.2f, %.2f] p2=[%.2f, %.2f]\n", p1._x, p1._y,  p2._x, p2._y);
      printf("angle=%f, G=[%.2f, %.2f]\n", angle, G._x, G._y);
      printf("\n----------\nLEAVE function with PED=%d\n----------\n",ped->GetID());
      // getc(stdin);


#endif

      // if( ped->GetID() == 21)
      //       fprintf(stderr, "%.2f %.2f %.2f %.2f %f %f %d %.2f %.2f %.2f\n", NextPointOnLine.GetX(), NextPointOnLine.GetY(),
      //               ped->GetPos().GetX(), ped->GetPos().GetY(), G.GetX(), G.GetY(), ped->GetID(), ped->GetV0().GetX(), ped->GetV0().GetY(), ped->GetGlobalTime());
// this stderr output can be used with plot_desired_velocity.py



      // if( ped->GetID() == 1)
            // fprintf(stderr, "%.2f %.2f %.2f %.2f %f %f %d %.2f %.2f %.2f\n", NextPointOnLine.GetX(), NextPointOnLine.GetY(),
                    // ped->GetPos().GetX(), ped->GetPos().GetY(), G.GetX(), G.GetY(), ped->GetID(), ped->GetV0().GetX(), ped->GetV0().GetY(), ped->GetGlobalTime());
// this stderr output can be used with scripts/plot_desired_velocity.py

      return G;
}

/// 6
Point DirectionFloorfield::GetTarget(Room* /*room*/, Pedestrian* ped) const
{
#if DEBUG

    if (1) {
#endif // DEBUG

        Point p;
        _ffviafm->getDirectionToDestination(ped, p);
        p = p.Normalized();     // @todo: argraf : scale with costvalue: " * ffviafm->getCostToTransition(ped->GetTransitionID(), ped->GetPos()) "
        return (p + ped->GetPos());

#if DEBUG
    }
#endif // DEBUG

    //this should not execute:
    std::cerr << "Failure in DirectionFloorfield::GetTarget!!" << std::endl;
    exit(EXIT_FAILURE);
}

Point DirectionFloorfield::GetDir2Wall(Pedestrian* ped) const
{
    Point p;
    _ffviafm->getDir2WallAt(ped->GetPos(), p);
    return p;
}

double DirectionFloorfield::GetDistance2Wall(Pedestrian* ped) const
{
    return _ffviafm->getDistance2WallAt(ped->GetPos());
}

void DirectionFloorfield::Init(Building* building, double stepsize,
                               double threshold, bool useDistancMap) {

     std::chrono::time_point<std::chrono::system_clock> start, end;
     start = std::chrono::system_clock::now();
     Log->Write("INFO: \tCalling Construtor of FloorfieldViaFM");
     _ffviafm = new FloorfieldViaFM(building, stepsize, stepsize, threshold,
                                   useDistancMap, false);
     end = std::chrono::system_clock::now();
     std::chrono::duration<double> elapsed_seconds = end-start;
     Log->Write("INFO: \tTaken time: " + std::to_string(elapsed_seconds.count()));

    _initDone = true;
}

DirectionFloorfield::DirectionFloorfield() {
    _ffviafm = nullptr;
    _initDone = false;
};

DirectionFloorfield::~DirectionFloorfield() {
    if (_ffviafm) {
        delete _ffviafm;
    }
}

/// 8
Point DirectionLocalFloorfield::GetTarget(Room* room, Pedestrian* ped) const
{
#if DEBUG
     if (1) {
#endif // DEBUG

     Point p;
     UnivFFviaFM* floorfield = _locffviafm.at(room->GetID());
#if DEBUG
     if (!floorfield->getGrid()->includesPoint(ped->GetPos())) {
          Log->Write("ERROR: \tDirectionLocalFloorfield::GetTarget is accessing wrong floorfield. Pedestrian is not inside!");
          p = Point(0.0,0.0);
          return p;
     }
#endif
     floorfield->getDirectionToUID(ped->GetExitIndex(), ped->GetPos(), p);
//     if (floorfield->getCostToDestination(ped->GetExitIndex(), ped->GetPos()) < 1.0) {
//          p = p * floorfield->getCostToDestination(ped->GetExitIndex(), ped->GetPos());
//     }
     return (p + ped->GetPos());

#if DEBUG
     }
#endif // DEBUG

     //this should not execute:
     std::cerr << "Failure in DirectionFloorfield::GetTarget!!" << std::endl;
    // exit(EXIT_FAILURE);
}

Point DirectionLocalFloorfield::GetDir2Wall(Pedestrian* ped) const
{
     Point p;
     int roomID = ped->GetRoomID();
     _locffviafm.at(roomID)->getDir2WallAt(ped->GetPos(), p);
     return p;
}

double DirectionLocalFloorfield::GetDistance2Wall(Pedestrian* ped) const
{
     return _locffviafm.at(ped->GetRoomID())->getDistance2WallAt(ped->GetPos());
}

double DirectionLocalFloorfield::GetDistance2Target(Pedestrian* ped, int UID) {
    int roomID = ped->GetRoomID();
    return _locffviafm.at(roomID)->getCostToDestination(UID, ped->GetPos());
}

void DirectionLocalFloorfield::Init(Building* buildingArg, double stepsize,
                                    double threshold, bool useDistanceMap) {
     _stepsize = stepsize;
     _building = buildingArg;
     _wallAvoidDistance = threshold;
     _useDistancefield = useDistanceMap;


     std::chrono::time_point<std::chrono::system_clock> start, end;
     start = std::chrono::system_clock::now();
     Log->Write("INFO: \tCalling Constructor of UnivFFviaFM(Room-scale) in DirectionLocalFloorfield::Init(...)");

     for (auto& roomPair : _building->GetAllRooms()) {
         UnivFFviaFM* newfield =  new UnivFFviaFM(roomPair.second.get(), _building, stepsize, _wallAvoidDistance, _useDistancefield);
         _locffviafm[roomPair.first] = newfield;
          newfield->setUser(DISTANCE_AND_DIRECTIONS_USED);
          newfield->setMode(LINESEGMENT);
          if (useDistanceMap) {
               newfield->setSpeedMode(FF_WALL_AVOID);
          } else {
               newfield->setSpeedMode(FF_HOMO_SPEED);
          }
         newfield->addAllTargetsParallel();

         //newfield->writeFF("directionsOfRoom" + std::to_string(roomPair.first) + ".vtk", newfield->getKnownDoorUIDs());
     }

    if (_building->GetConfig()->get_write_VTK_files_direction()) {
        for (unsigned int i = 0; i < _locffviafm.size(); ++i) {
            auto iter = _locffviafm.begin();
            std::advance(iter, i);
            int roomNr = iter->first;
            iter->second->writeFF("direction" + std::to_string(roomNr) + ".vtk", iter->second->getKnownDoorUIDs());
        }
    }

    end = std::chrono::system_clock::now();
     std::chrono::duration<double> elapsed_seconds = end-start;
     Log->Write("INFO: \tTime to construct FF in DirectionLocalFloorfield: " + std::to_string(elapsed_seconds.count()));
     _initDone = true;
}

DirectionLocalFloorfield::DirectionLocalFloorfield() {
     _initDone = false;
}

DirectionLocalFloorfield::~DirectionLocalFloorfield() {
     for (auto pair : _locffviafm) {
          if (pair.second) {
               delete pair.second;
          }
     }
}


///9
Point DirectionSubLocalFloorfield::GetTarget(Room* room, Pedestrian* ped) const
{
     (void)room; // silence warning
#if DEBUG
     if (1) {
#endif // DEBUG

     Point p;
     UnivFFviaFM* floorfield = _locffviafm.at(ped->GetSubRoomUID());
#if DEBUG
     if (!floorfield->getGrid()->includesPoint(ped->GetPos())) {
          Log->Write("ERROR: \tDirectionSubLocalFloorfield::GetTarget is accessing wrong floorfield. Pedestrian is not inside!");
          p = Point(0.0,0.0);
          return p;
     }
#endif
     floorfield->getDirectionToUID(ped->GetExitIndex(), ped->GetPos(),p);
//     if (floorfield->getCostToDestination(ped->GetExitIndex(), ped->GetPos()) < 1.0){
//          p = p * floorfield->getCostToDestination(ped->GetExitIndex(), ped->GetPos());
//     }
     return (p + ped->GetPos());

#if DEBUG
     }
#endif // DEBUG
}

Point DirectionSubLocalFloorfield::GetDir2Wall(Pedestrian* ped) const
{
     Point p;
     int key = ped->GetSubRoomUID();
     _locffviafm.at(key)->getDir2WallAt(ped->GetPos(), p);
     return p;
}

double DirectionSubLocalFloorfield::GetDistance2Wall(Pedestrian* ped) const
{
     return _locffviafm.at(ped->GetSubRoomUID())->getDistance2WallAt(ped->GetPos());
}

double DirectionSubLocalFloorfield::GetDistance2Target(Pedestrian* ped, int UID) {
    int subroomUID = ped->GetSubRoomUID();
    return _locffviafm.at(subroomUID)->getCostToDestination(UID, ped->GetPos());
}

void DirectionSubLocalFloorfield::Init(Building* buildingArg, double stepsize,
                                       double threshold, bool useDistanceMap) {
     _stepsize = stepsize;
     _building = buildingArg;
     _wallAvoidDistance = threshold;
     _useDistancefield = useDistanceMap;

     std::chrono::time_point<std::chrono::system_clock> start, end;
     start = std::chrono::system_clock::now();
     Log->Write("INFO: \tCalling Construtor of UnivFFviaFM(Subroom-scale)");

     for (auto& roomPair : _building->GetAllRooms()) {
          for (auto& subPair : roomPair.second->GetAllSubRooms()) {
               int subUID = subPair.second->GetUID();
               UnivFFviaFM* floorfield = new UnivFFviaFM(subPair.second.get(), _building, stepsize, _wallAvoidDistance, _useDistancefield);
               _locffviafm[subUID] = floorfield;
               floorfield->setUser(DISTANCE_AND_DIRECTIONS_USED);
               floorfield->setMode(LINESEGMENT);
               if (useDistanceMap) {
                    floorfield->setSpeedMode(FF_WALL_AVOID);
               } else {
                    floorfield->setSpeedMode(FF_HOMO_SPEED);
               }
               floorfield->addAllTargetsParallel();
          }


     }

    if (_building->GetConfig()->get_write_VTK_files_direction()) {
        for (unsigned int i = 0; i < _locffviafm.size(); ++i) {
            auto iter = _locffviafm.begin();
            std::advance(iter, i);
            int roomNr = iter->first;
            iter->second->writeFF("direction" + std::to_string(roomNr) + ".vtk", iter->second->getKnownDoorUIDs());
        }
    }


    end = std::chrono::system_clock::now();
     std::chrono::duration<double> elapsed_seconds = end-start;
     Log->Write("INFO: \tTaken time: " + std::to_string(elapsed_seconds.count()));

     _initDone = true;

    //_locffviafm[0]->writeFF()
     //write floorfields to file, one file per subroom //ar.graf: [SWITCH writevtk ON/OFF]
//     for(unsigned int i = 0; i < subUIDs.size(); ++i) {
//          std::vector<int> targets = {};
//          targets.clear();
//          int subroomUID = subUIDs[i];
//          //if (subroomUID != 26) continue;
//
//          for (auto pair : subAndTarget) {
//               if (pair.first == subroomUID) {
//                    targets.emplace_back(pair.second);
//               }
//          }
//          std::string filename1 = "floorfield" + std::to_string(subroomUID) + ".vtk";
//          if (targets.size() > 0)
//               _locffviafm[subroomUID]->writeFF(filename1, targets);
//     }
}

DirectionSubLocalFloorfield::DirectionSubLocalFloorfield() {
     _initDone = false;
}

DirectionSubLocalFloorfield::~DirectionSubLocalFloorfield() {
     for (auto pair : _locffviafm) {
          delete pair.second;
     }
}

///10
Point DirectionSubLocalFloorfieldTrips::GetTarget(Room* /*room*/, Pedestrian* ped) const
{
     Goal* goal = ped->GetBuilding()->GetFinalGoal(ped->GetFinalDestination());
     // Pedestrian is inside a waiting area
     if ((goal!=nullptr) && (goal->IsInsideGoal(ped->GetPos()))){

          std::vector<Point> polygon(goal->GetPolygon());
          std::set<Point> triangle;

          int min=0, max;

          // Get randomly 3 points of polygon (
          while (triangle.size() < 3){
               max = polygon.size()-1;
               int index = min + (std::rand() % static_cast<int>(max - min + 1));

               triangle.insert(polygon.at(index));
          }

          double r1, r2;
          r1 = ((double) std::rand() / (RAND_MAX));
          r2 = ((double) std::rand() / (RAND_MAX));
          Point p1 = polygon[0] * (1. - sqrt(r1));
          Point p2 = polygon[1] * (sqrt(r1) * (1 - r2));
          Point p3 = polygon[2] * (sqrt(r1) * r2);

          Point p = p1 + p2 + p3;

//          UnivFFviaFM* floorfield = _locffviafm.at(ped->GetSubRoomUID());
//          floorfield->getDirectionToUID(ped->GetExitIndex(), ped->GetPos(),p);
//          return (p + ped->GetPos());
          return p;
     } else {
          Point p;
          UnivFFviaFM* floorfield = _locffviafm.at(ped->GetSubRoomUID());
          floorfield->getDirectionToUID(ped->GetExitIndex(), ped->GetPos(),p);
          return (p + ped->GetPos());

     }
}

Point DirectionSubLocalFloorfieldTrips::GetDir2Wall(Pedestrian* ped) const
{
     Point p;
     int key = ped->GetSubRoomUID();
     _locffviafm.at(key)->getDir2WallAt(ped->GetPos(), p);
     return p;
}

double DirectionSubLocalFloorfieldTrips::GetDistance2Wall(Pedestrian* ped) const
{
     return _locffviafm.at(ped->GetSubRoomUID())->getDistance2WallAt(ped->GetPos());
}

double DirectionSubLocalFloorfieldTrips::GetDistance2Target(Pedestrian* ped, int UID) {
     int subroomUID = ped->GetSubRoomUID();
     return _locffviafm.at(subroomUID)->getCostToDestination(UID, ped->GetPos());
}

void DirectionSubLocalFloorfieldTrips::Init(Building* buildingArg, double stepsize,
          double threshold, bool useDistanceMap) {
     _stepsize = stepsize;
     _building = buildingArg;
     _wallAvoidDistance = threshold;
     _useDistancefield = useDistanceMap;

     std::chrono::time_point<std::chrono::system_clock> start, end;
     start = std::chrono::system_clock::now();
     Log->Write("INFO: \tCalling Construtor of UnivFFviaFMTrips(Subroom-scale)");

     for (auto& roomPair : _building->GetAllRooms()) {
          for (auto& subPair : roomPair.second->GetAllSubRooms()) {
               int subUID = subPair.second->GetUID();
               UnivFFviaFM* floorfield = new UnivFFviaFM(subPair.second.get(), _building, stepsize, _wallAvoidDistance, _useDistancefield);
               _locffviafm[subUID] = floorfield;
               floorfield->setUser(DISTANCE_AND_DIRECTIONS_USED);
               floorfield->setMode(LINESEGMENT);
               if (useDistanceMap) {
                    floorfield->setSpeedMode(FF_WALL_AVOID);
               } else {
                    floorfield->setSpeedMode(FF_HOMO_SPEED);
               }
               floorfield->addAllTargetsParallel();
          }


     }

     if (_building->GetConfig()->get_write_VTK_files_direction()) {
          for (unsigned int i = 0; i < _locffviafm.size(); ++i) {
               auto iter = _locffviafm.begin();
               std::advance(iter, i);
               int roomNr = iter->first;
               iter->second->writeFF("direction" + std::to_string(roomNr) + ".vtk", iter->second->getKnownDoorUIDs());
          }
     }


     end = std::chrono::system_clock::now();
     std::chrono::duration<double> elapsed_seconds = end-start;
     Log->Write("INFO: \tTaken time: " + std::to_string(elapsed_seconds.count()));

     _initDone = true;

     //_locffviafm[0]->writeFF()
     //write floorfields to file, one file per subroom //ar.graf: [SWITCH writevtk ON/OFF]
//     for(unsigned int i = 0; i < subUIDs.size(); ++i) {
//          std::vector<int> targets = {};
//          targets.clear();
//          int subroomUID = subUIDs[i];
//          //if (subroomUID != 26) continue;
//
//          for (auto pair : subAndTarget) {
//               if (pair.first == subroomUID) {
//                    targets.emplace_back(pair.second);
//               }
//          }
//          std::string filename1 = "floorfield" + std::to_string(subroomUID) + ".vtk";
//          if (targets.size() > 0)
//               _locffviafm[subroomUID]->writeFF(filename1, targets);
//     }
}

DirectionSubLocalFloorfieldTrips::DirectionSubLocalFloorfieldTrips() {
     _initDone = false;
}

DirectionSubLocalFloorfieldTrips::~DirectionSubLocalFloorfieldTrips() {
     for (auto pair : _locffviafm) {
          delete pair.second;
     }
}

///11
Point DirectionSubLocalFloorfieldTripsVoronoi::GetTarget(Room* /*room*/, Pedestrian* ped) const
{
     Goal* goal = ped->GetBuilding()->GetFinalGoal(ped->GetFinalDestination());
     // Pedestrian is inside a waiting area
     if ((goal!=nullptr) && (goal->IsInsideGoal(ped->GetPos()))){

          std::vector<Point> polygon(goal->GetPolygon());
          std::set<Point> triangle;

          int min=0, max;

          // Get randomly 3 points of polygon (
          while (triangle.size() < 3){
               max = polygon.size()-1;
               int index = min + (std::rand() % static_cast<int>(max - min + 1));

               triangle.insert(polygon.at(index));
          }

          double r1, r2;
          r1 = ((double) std::rand() / (RAND_MAX));
          r2 = ((double) std::rand() / (RAND_MAX));
          Point p1 = polygon[0] * (1. - sqrt(r1));
          Point p2 = polygon[1] * (sqrt(r1) * (1 - r2));
          Point p3 = polygon[2] * (sqrt(r1) * r2);

          Point p = p1 + p2 + p3;

          return p;
     } else {
          Point p;
          UnivFFviaFM* floorfield = _locffviafm.at(ped->GetSubRoomUID());
          floorfield->getDirectionToUID(ped->GetExitIndex(), ped->GetPos(),p);

          return (p + ped->GetPos());

     }
}

Point DirectionSubLocalFloorfieldTripsVoronoi::GetDir2Wall(Pedestrian* ped) const
{
     Point p;
     int key = ped->GetSubRoomUID();
     _locffviafm.at(key)->getDir2WallAt(ped->GetPos(), p);
     return p;
}

double DirectionSubLocalFloorfieldTripsVoronoi::GetDistance2Wall(Pedestrian* ped) const
{
     return _locffviafm.at(ped->GetSubRoomUID())->getDistance2WallAt(ped->GetPos());
}

double DirectionSubLocalFloorfieldTripsVoronoi::GetDistance2Target(Pedestrian* ped, int UID) {
     int subroomUID = ped->GetSubRoomUID();
     return _locffviafm.at(subroomUID)->getCostToDestination(UID, ped->GetPos());
}

void DirectionSubLocalFloorfieldTripsVoronoi::Init(Building* buildingArg, double stepsize,
          double threshold, bool useDistanceMap) {
     _stepsize = stepsize;
     _building = buildingArg;
     _wallAvoidDistance = threshold;
     _useDistancefield = useDistanceMap;

     std::chrono::time_point<std::chrono::system_clock> start, end;
     start = std::chrono::system_clock::now();
     Log->Write("INFO: \tCalling Construtor of UnivFFviaFMTripsVoronoi(Subroom-scale)");

     for (auto& roomPair : _building->GetAllRooms()) {
          for (auto& subPair : roomPair.second->GetAllSubRooms()) {
               int subUID = subPair.second->GetUID();
               UnivFFviaFM* floorfield = new UnivFFviaFM(subPair.second.get(), _building, stepsize, _wallAvoidDistance, _useDistancefield);
               _locffviafm[subUID] = floorfield;
               floorfield->setUser(DISTANCE_AND_DIRECTIONS_USED);
               floorfield->setMode(LINESEGMENT);
               if (useDistanceMap) {
                    floorfield->setSpeedMode(FF_WALL_AVOID);
               } else {
                    floorfield->setSpeedMode(FF_HOMO_SPEED);
               }
               floorfield->addAllTargetsParallel();
          }


     }

     if (_building->GetConfig()->get_write_VTK_files_direction()) {
          for (unsigned int i = 0; i < _locffviafm.size(); ++i) {
               auto iter = _locffviafm.begin();
               std::advance(iter, i);
               int roomNr = iter->first;
               iter->second->writeFF("direction" + std::to_string(roomNr) + ".vtk", iter->second->getKnownDoorUIDs());
          }
     }


     end = std::chrono::system_clock::now();
     std::chrono::duration<double> elapsed_seconds = end-start;
     Log->Write("INFO: \tTaken time: " + std::to_string(elapsed_seconds.count()));

     _initDone = true;

     //_locffviafm[0]->writeFF()
     //write floorfields to file, one file per subroom //ar.graf: [SWITCH writevtk ON/OFF]
//     for(unsigned int i = 0; i < subUIDs.size(); ++i) {
//          std::vector<int> targets = {};
//          targets.clear();
//          int subroomUID = subUIDs[i];
//          //if (subroomUID != 26) continue;
//
//          for (auto pair : subAndTarget) {
//               if (pair.first == subroomUID) {
//                    targets.emplace_back(pair.second);
//               }
//          }
//          std::string filename1 = "floorfield" + std::to_string(subroomUID) + ".vtk";
//          if (targets.size() > 0)
//               _locffviafm[subroomUID]->writeFF(filename1, targets);
//     }
}

DirectionSubLocalFloorfieldTripsVoronoi::DirectionSubLocalFloorfieldTripsVoronoi() {
     _initDone = false;
}

DirectionSubLocalFloorfieldTripsVoronoi::~DirectionSubLocalFloorfieldTripsVoronoi() {
     for (auto pair : _locffviafm) {
          delete pair.second;
     }
}

// 12
Point DirectionTrain::GetTarget(Room* /*room*/, Pedestrian* ped) const
{

     Point p1 = ped->GetExitLine()->GetPoint1();
     Point p2 = ped->GetExitLine()->GetPoint2();
     Line ExitLine = Line(p1, p2, 0);
     auto TrainTypes = ped->GetBuilding()->GetTrainTypes();
     auto TrainTimeTables = ped->GetBuilding()->GetTrainTimeTables();
     auto now = ped->GetGlobalTime();
     string type_delme="";
     // std::cout << ">>> Enter with ped at " << ped->GetPos().toString().c_str() << "\n";
     for(auto && t: TrainTimeTables)
     {
          if(ped->GetRoomID() != t.second->rid) continue;

          if( (now>=t.second->tin) && (now<=t.second->tout) )
          {
               auto doors = TrainTypes[t.second->type]->doors;
               int i=-1, imin=0;
               double dist_min = 10000;
               for(auto door: doors)
               {
                    i++;
                    const Point & d1 = door.GetPoint1();
                    const Point & d2 = door.GetPoint2();
                    const Point & c = (d1+d2)*0.5;

                    double dist = (ped->GetPos()-c).Norm();
                    // std::cout << "door id: " << door.GetID()<< " dist: " << dist<< "\n";

                    if(dist <= dist_min)
                    {
                         dist_min = dist;
                         imin=i;
                         type_delme=t.second->type;
                         // std::cout << "    > imin " << imin << "  mindist " << dist_min << "\n";
                    }
               }// doors
               p1  = doors[imin].GetPoint1();
               p2 = doors[imin].GetPoint2();
               // std::cout << "\n>>> train: now " << now << ", type: " << type_delme.c_str() << "\n";
               // std::cout << ">>> p1=" << p1.toString().c_str() << ". p2=" << p2.toString().c_str()<< "\n";
               // std::cout << ">>> ped at " << ped->GetPos().toString().c_str() << "\n";
               // getc(stdin);

          }// if time in
     }


     return (p1+ p2)*0.5;
}
