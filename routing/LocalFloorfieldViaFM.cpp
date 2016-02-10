//
// Created by ar.graf on 1/28/16.
//

#include "LocalFloorfieldViaFM.h"

LocalFloorfieldViaFM::LocalFloorfieldViaFM(){};
LocalFloorfieldViaFM::LocalFloorfieldViaFM(const Room* const roomArg,
               const Building* buildingArg,
               const double hxArg, const double hyArg,
               const double wallAvoidDistance, const bool useDistancefield,
               const std::string& filename) {
     //ctor
     threshold = -1; //negative value means: ignore threshold
     threshold = wallAvoidDistance;
     building = buildingArg;
     room = roomArg;

     if (hxArg != hyArg) std::cerr << "ERROR: hx != hy <=========";
     //parse building and create list of walls/obstacles (find xmin xmax, ymin, ymax, and add border?)
     Log->Write("INFO: \tStart Parsing: Room " + roomArg->GetID());
     parseRoom(roomArg, hxArg, hyArg);
     Log->Write("INFO: \tFinished Parsing: Room " + roomArg->GetID());
     //testoutput("AALineScan.vtk", "AALineScan.txt", dist2Wall);

     prepareForDistanceFieldCalculation(wall);
     Log->Write("INFO: \tGrid initialized: Walls");

     calculateDistanceField(-1.); //negative threshold is ignored, so all distances get calculated. this is important since distances is used for slowdown/redirect
     Log->Write("INFO: \tGrid initialized: Walldistances");

     setSpeed(useDistancefield); //use distance2Wall
     Log->Write("INFO: \tGrid initialized: Speed");
     calculateFloorfield(cost, neggrad);
};

void LocalFloorfieldViaFM::getDirectionToDestination(Pedestrian* ped,
      Point& direction) {
     //check, if UID of target is a valid UID inside the room
     //then call mother-class function

//     if (std::find(room->GetAllTransitionsIDs().begin(),
//                   room->GetAllTransitionsIDs().end(),
//                   ped->GetExitIndex()) != room->GetAllTransitionsIDs().end()) {
//          FloorfieldViaFM::getDirectionToDestination(ped, direction);
//          return;
//     }

     //std::cerr << "UID not in room: " << ped->GetExitIndex() << std::endl;
     //the check above is misleading, as it only checks if UID matches any transition
     FloorfieldViaFM::getDirectionToDestination(ped, direction);
     return;
}

void LocalFloorfieldViaFM::parseRoom(const Room* const roomArg,
      const double hxArg, const double hyArg)
{
     room = roomArg;
     //init min/max before parsing
     double xMin = FLT_MAX;
     double xMax = -FLT_MAX;
     double yMin = xMin;
     double yMax = xMax;
     costmap.clear();
     neggradmap.clear();

     //create a list of walls
     //add all transition and put open doors at the beginning of "wall"
     std::map<int, Transition*> allTransitions;
     for (auto& itSubroom : room->GetAllSubRooms()) {
          for (auto itTrans : itSubroom.second->GetAllTransitions()) {
               if (!allTransitions.count(itTrans->GetUniqueID())) {
                    allTransitions[itTrans->GetUniqueID()] = &(*itTrans);
                    if (itTrans->IsOpen()) {
                         wall.emplace_back(Line((Line) *itTrans));
                         costmap.emplace(itTrans->GetUniqueID(), nullptr);
                         neggradmap.emplace(itTrans->GetUniqueID(), nullptr);
                    }
               }
          }
     }
     numOfExits = wall.size();
     //put closed doors next, they are considered as walls later (index >= numOfExits)
     for (auto& trans : allTransitions) {
          if (!trans.second->IsOpen()) {
               wall.emplace_back(Line ( (Line) *(trans.second)));
          }
     }

     for (const auto& itSubroom : room->GetAllSubRooms()) {
          std::vector<Obstacle*> allObstacles = itSubroom.second->GetAllObstacles();
          for (std::vector<Obstacle*>::iterator itObstacles = allObstacles.begin(); itObstacles != allObstacles.end(); ++itObstacles) {

               std::vector<Wall> allObsWalls = (*itObstacles)->GetAllWalls();
               for (std::vector<Wall>::iterator itObsWall = allObsWalls.begin(); itObsWall != allObsWalls.end(); ++itObsWall) {
                    wall.emplace_back(Line( (Line) *itObsWall));
                    // xMin xMax
                    if ((*itObsWall).GetPoint1().GetX() < xMin) xMin = (*itObsWall).GetPoint1().GetX();
                    if ((*itObsWall).GetPoint2().GetX() < xMin) xMin = (*itObsWall).GetPoint2().GetX();
                    if ((*itObsWall).GetPoint1().GetX() > xMax) xMax = (*itObsWall).GetPoint1().GetX();
                    if ((*itObsWall).GetPoint2().GetX() > xMax) xMax = (*itObsWall).GetPoint2().GetX();
                    // yMin yMax
                    if ((*itObsWall).GetPoint1().GetY() < yMin) yMin = (*itObsWall).GetPoint1().GetY();
                    if ((*itObsWall).GetPoint2().GetY() < yMin) yMin = (*itObsWall).GetPoint2().GetY();
                    if ((*itObsWall).GetPoint1().GetY() > yMax) yMax = (*itObsWall).GetPoint1().GetY();
                    if ((*itObsWall).GetPoint2().GetY() > yMax) yMax = (*itObsWall).GetPoint2().GetY();
               }
          }

          std::vector<Wall> allWalls = itSubroom.second->GetAllWalls();
          for (std::vector<Wall>::iterator itWall = allWalls.begin(); itWall != allWalls.end(); ++itWall) {
               wall.emplace_back( Line( (Line) *itWall));
               // xMin xMax
               if ((*itWall).GetPoint1().GetX() < xMin) xMin = (*itWall).GetPoint1().GetX();
               if ((*itWall).GetPoint2().GetX() < xMin) xMin = (*itWall).GetPoint2().GetX();
               if ((*itWall).GetPoint1().GetX() > xMax) xMax = (*itWall).GetPoint1().GetX();
               if ((*itWall).GetPoint2().GetX() > xMax) xMax = (*itWall).GetPoint2().GetX();
               // yMin yMax
               if ((*itWall).GetPoint1().GetY() < yMin) yMin = (*itWall).GetPoint1().GetY();
               if ((*itWall).GetPoint2().GetY() < yMin) yMin = (*itWall).GetPoint2().GetY();
               if ((*itWall).GetPoint1().GetY() > yMax) yMax = (*itWall).GetPoint1().GetY();
               if ((*itWall).GetPoint2().GetY() > yMax) yMax = (*itWall).GetPoint2().GetY();
          }
     }

     //create Rect Grid
     grid = new RectGrid();
     grid->setBoundaries(xMin, yMin, xMax, yMax);
     grid->setSpacing(hxArg, hyArg);
     grid->createGrid();

     //create arrays
     flag = new int[grid->GetnPoints()];                  //flag:( 0 = unknown, 1 = singel, 2 = double, 3 = final, -7 = outside)
     dist2Wall = new double[grid->GetnPoints()];
     speedInitial = new double[grid->GetnPoints()];
     modifiedspeed = new double[grid->GetnPoints()];
     cost = new double[grid->GetnPoints()];
     neggrad = new Point[grid->GetnPoints()];
     dirToWall = new Point[grid->GetnPoints()];
     trialfield = new Trial[grid->GetnPoints()];                 //created with other arrays, but not initialized yet

     costmap.emplace(-1 , cost);                         // enable default ff (closest exit)
     neggradmap.emplace(-1, neggrad);

     //init grid with -3 as unknown distance to any wall
     for(long int i = 0; i < grid->GetnPoints(); ++i) {
          dist2Wall[i] = -3.;
     }
     drawLinesOnGrid(wall, dist2Wall, 0.);
}


SubLocalFloorfieldViaFM::SubLocalFloorfieldViaFM(){};
SubLocalFloorfieldViaFM::SubLocalFloorfieldViaFM(const SubRoom* const roomArg,
      const Building* buildingArg,
      const double hxArg, const double hyArg,
      const double wallAvoidDistance, const bool useDistancefield,
      const std::string& filename) {
     //ctor
     threshold = -1; //negative value means: ignore threshold
     threshold = wallAvoidDistance;
     building = buildingArg;
     subroom = roomArg;

     if (hxArg != hyArg) std::cerr << "ERROR: hx != hy <=========";
     //parse building and create list of walls/obstacles (find xmin xmax, ymin, ymax, and add border?)
     //Log->Write("INFO: \tStart Parsing: Room " + roomArg->GetUID());
     parseRoom(roomArg, hxArg, hyArg);
     //Log->Write("INFO: \tFinished Parsing: Room " + roomArg->GetUID());
     //testoutput("AALineScan.vtk", "AALineScan.txt", dist2Wall);

     prepareForDistanceFieldCalculation(wall);
     //Log->Write("INFO: \tGrid initialized: Walls");

     calculateDistanceField(-1.); //negative threshold is ignored, so all distances get calculated. this is important since distances is used for slowdown/redirect
     //Log->Write("INFO: \tGrid initialized: Walldistances");

     setSpeed(useDistancefield); //use distance2Wall
     //Log->Write("INFO: \tGrid initialized: Speed");
     calculateFloorfield(cost, neggrad);
};

void SubLocalFloorfieldViaFM::getDirectionToDestination(Pedestrian* ped,
      Point& direction)
{
     FloorfieldViaFM::getDirectionToDestination(ped, direction);
     return;
}

void SubLocalFloorfieldViaFM::parseRoom(const SubRoom* const roomArg,
      const double hxArg, const double hyArg)
{
     subroom = roomArg;
     //init min/max before parsing
     double xMin = FLT_MAX;
     double xMax = -FLT_MAX;
     double yMin = xMin;
     double yMax = xMax;
     costmap.clear();
     neggradmap.clear();

     //create a list of walls
     //add all transition and put open doors at the beginning of "wall"
     std::map<int, Transition*> allTransitions;

     for (auto itTrans : subroom->GetAllTransitions()) {
          if (!allTransitions.count(itTrans->GetUniqueID())) {
               allTransitions[itTrans->GetUniqueID()] = &(*itTrans);
               if (itTrans->IsOpen()) {
                    wall.emplace_back(Line((Line) *itTrans));
                    costmap.emplace(itTrans->GetUniqueID(), nullptr);
                    neggradmap.emplace(itTrans->GetUniqueID(), nullptr);
               }
          }
     }

     for (auto itCross : subroom->GetAllCrossings()) {
          wall.emplace_back(Line( (Line) *itCross));
          costmap.emplace(itCross->GetUniqueID(), nullptr);
          neggradmap.emplace(itCross->GetUniqueID(), nullptr);
     }

     numOfExits = wall.size();
     //put closed doors next, they are considered as walls later (index >= numOfExits)
     for (auto& trans : allTransitions) {
          if (!trans.second->IsOpen()) {
               wall.emplace_back(Line ( (Line) *(trans.second)));
          }
     }


     std::vector<Obstacle*> allObstacles = subroom->GetAllObstacles();
     for (std::vector<Obstacle*>::iterator itObstacles = allObstacles.begin(); itObstacles != allObstacles.end(); ++itObstacles) {

          std::vector<Wall> allObsWalls = (*itObstacles)->GetAllWalls();
          for (std::vector<Wall>::iterator itObsWall = allObsWalls.begin(); itObsWall != allObsWalls.end(); ++itObsWall) {
               wall.emplace_back(Line( (Line) *itObsWall));
               // xMin xMax
               if ((*itObsWall).GetPoint1().GetX() < xMin) xMin = (*itObsWall).GetPoint1().GetX();
               if ((*itObsWall).GetPoint2().GetX() < xMin) xMin = (*itObsWall).GetPoint2().GetX();
               if ((*itObsWall).GetPoint1().GetX() > xMax) xMax = (*itObsWall).GetPoint1().GetX();
               if ((*itObsWall).GetPoint2().GetX() > xMax) xMax = (*itObsWall).GetPoint2().GetX();
               // yMin yMax
               if ((*itObsWall).GetPoint1().GetY() < yMin) yMin = (*itObsWall).GetPoint1().GetY();
               if ((*itObsWall).GetPoint2().GetY() < yMin) yMin = (*itObsWall).GetPoint2().GetY();
               if ((*itObsWall).GetPoint1().GetY() > yMax) yMax = (*itObsWall).GetPoint1().GetY();
               if ((*itObsWall).GetPoint2().GetY() > yMax) yMax = (*itObsWall).GetPoint2().GetY();
          }
     }

     std::vector<Wall> allWalls = subroom->GetAllWalls();
     for (std::vector<Wall>::iterator itWall = allWalls.begin(); itWall != allWalls.end(); ++itWall) {
          wall.emplace_back( Line( (Line) *itWall));
          // xMin xMax
          if ((*itWall).GetPoint1().GetX() < xMin) xMin = (*itWall).GetPoint1().GetX();
          if ((*itWall).GetPoint2().GetX() < xMin) xMin = (*itWall).GetPoint2().GetX();
          if ((*itWall).GetPoint1().GetX() > xMax) xMax = (*itWall).GetPoint1().GetX();
          if ((*itWall).GetPoint2().GetX() > xMax) xMax = (*itWall).GetPoint2().GetX();
          // yMin yMax
          if ((*itWall).GetPoint1().GetY() < yMin) yMin = (*itWall).GetPoint1().GetY();
          if ((*itWall).GetPoint2().GetY() < yMin) yMin = (*itWall).GetPoint2().GetY();
          if ((*itWall).GetPoint1().GetY() > yMax) yMax = (*itWall).GetPoint1().GetY();
          if ((*itWall).GetPoint2().GetY() > yMax) yMax = (*itWall).GetPoint2().GetY();
     }


     //create Rect Grid
     grid = new RectGrid();
     grid->setBoundaries(xMin, yMin, xMax, yMax);
     grid->setSpacing(hxArg, hyArg);
     grid->createGrid();

     //create arrays
     flag = new int[grid->GetnPoints()];                  //flag:( 0 = unknown, 1 = singel, 2 = double, 3 = final, -7 = outside)
     dist2Wall = new double[grid->GetnPoints()];
     speedInitial = new double[grid->GetnPoints()];
     modifiedspeed = new double[grid->GetnPoints()];
     cost = new double[grid->GetnPoints()];
     neggrad = new Point[grid->GetnPoints()];
     dirToWall = new Point[grid->GetnPoints()];
     trialfield = new Trial[grid->GetnPoints()];                 //created with other arrays, but not initialized yet

     costmap.emplace(-1 , cost);                         // enable default ff (closest exit)
     neggradmap.emplace(-1, neggrad);

     //init grid with -3 as unknown distance to any wall
     for(long int i = 0; i < grid->GetnPoints(); ++i) {
          dist2Wall[i] = -3.;
     }
     drawLinesOnGrid(wall, dist2Wall, 0.);
}
