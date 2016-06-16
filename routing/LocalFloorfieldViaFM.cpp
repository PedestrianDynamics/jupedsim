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
     Log->Write("INFO: \tStart Parsing: Room %d", roomArg->GetID());
     parseRoom(roomArg, hxArg, hyArg);
     Log->Write("INFO: \tFinished Parsing: Room %d", roomArg->GetID());
     //testoutput("AALineScan.vtk", "AALineScan.txt", dist2Wall);

     prepareForDistanceFieldCalculation(false);
     //here we need to draw blocker lines @todo: ar.graf
     //drawBlockerLines();
     Log->Write("INFO: \tGrid initialized: Walls in room %d", roomArg->GetID());

     calculateDistanceField(-1.); //negative threshold is ignored, so all distances get calculated. this is important since distances is used for slowdown/redirect
     Log->Write("INFO: \tGrid initialized: Walldistances in room %d", roomArg->GetID());

     setSpeed(useDistancefield); //use distance2Wall
     Log->Write("INFO: \tGrid initialized: Speed in room %d", roomArg->GetID());
     calculateFloorfield(exitsFromScope, cost, neggrad);
     Log->Write("INFO: \tFloor field for \"goal -1\" done in room %d", roomArg->GetID());
};

void LocalFloorfieldViaFM::getDirectionToDestination(Pedestrian* ped,
      Point& direction) {

     FloorfieldViaFM::getDirectionToDestination(ped, direction);
     return;
}

void LocalFloorfieldViaFM::getDirectionToGoalID(const int goalID){
     std::cerr << "invalid call to LocalFloorfieldViaFM::getDirectionToGoalID!" << std::endl;
};

void LocalFloorfieldViaFM::parseRoom(const Room* const roomArg,
      const double hxArg, const double hyArg)
{
     room = roomArg;
     //init min/max before parsing
     double xMin = DBL_MAX;
     double xMax = -DBL_MAX;
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
                         exitsFromScope.emplace_back(Line((Line) *itTrans));
                         costmap.emplace(itTrans->GetUniqueID(), nullptr);
                         neggradmap.emplace(itTrans->GetUniqueID(), nullptr);
                    }
               }
          }
     }
     numOfExits = exitsFromScope.size();
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
                    if ((*itObsWall).GetPoint1()._x < xMin) xMin = (*itObsWall).GetPoint1()._x;
                    if ((*itObsWall).GetPoint2()._x < xMin) xMin = (*itObsWall).GetPoint2()._x;
                    if ((*itObsWall).GetPoint1()._x > xMax) xMax = (*itObsWall).GetPoint1()._x;
                    if ((*itObsWall).GetPoint2()._x > xMax) xMax = (*itObsWall).GetPoint2()._x;
                    // yMin yMax
                    if ((*itObsWall).GetPoint1()._y < yMin) yMin = (*itObsWall).GetPoint1()._y;
                    if ((*itObsWall).GetPoint2()._y < yMin) yMin = (*itObsWall).GetPoint2()._y;
                    if ((*itObsWall).GetPoint1()._y > yMax) yMax = (*itObsWall).GetPoint1()._y;
                    if ((*itObsWall).GetPoint2()._y > yMax) yMax = (*itObsWall).GetPoint2()._y;
               }
          }

          std::vector<Wall> allWalls = itSubroom.second->GetAllWalls();
          for (std::vector<Wall>::iterator itWall = allWalls.begin(); itWall != allWalls.end(); ++itWall) {
               wall.emplace_back( Line( (Line) *itWall));
               // xMin xMax
               if ((*itWall).GetPoint1()._x < xMin) xMin = (*itWall).GetPoint1()._x;
               if ((*itWall).GetPoint2()._x < xMin) xMin = (*itWall).GetPoint2()._x;
               if ((*itWall).GetPoint1()._x > xMax) xMax = (*itWall).GetPoint1()._x;
               if ((*itWall).GetPoint2()._x > xMax) xMax = (*itWall).GetPoint2()._x;
               // yMin yMax
               if ((*itWall).GetPoint1()._y < yMin) yMin = (*itWall).GetPoint1()._y;
               if ((*itWall).GetPoint2()._y < yMin) yMin = (*itWall).GetPoint2()._y;
               if ((*itWall).GetPoint1()._y > yMax) yMax = (*itWall).GetPoint1()._y;
               if ((*itWall).GetPoint2()._y > yMax) yMax = (*itWall).GetPoint2()._y;
          }
          const vector<Crossing*>& allCrossings = itSubroom.second->GetAllCrossings();
          for (Crossing* crossPtr : allCrossings) {
               if (!crossPtr->IsOpen()) {
                    wall.emplace_back( Line( (Line) *crossPtr));

                    if (crossPtr->GetPoint1()._x < xMin) xMin = crossPtr->GetPoint1()._x;
                    if (crossPtr->GetPoint2()._x < xMin) xMin = crossPtr->GetPoint2()._x;
                    if (crossPtr->GetPoint1()._x > xMax) xMax = crossPtr->GetPoint1()._x;
                    if (crossPtr->GetPoint2()._x > xMax) xMax = crossPtr->GetPoint2()._x;

                    if (crossPtr->GetPoint1()._y < yMin) yMin = crossPtr->GetPoint1()._y;
                    if (crossPtr->GetPoint2()._y < yMin) yMin = crossPtr->GetPoint2()._y;
                    if (crossPtr->GetPoint1()._y > yMax) yMax = crossPtr->GetPoint1()._y;
                    if (crossPtr->GetPoint2()._y > yMax) yMax = crossPtr->GetPoint2()._y;
               }
          }
     }

     //create Rect Grid
     grid = new RectGrid();
     grid->setBoundaries(xMin, yMin, xMax, yMax);
     grid->setSpacing(hxArg, hyArg);
     grid->createGrid();

     //create arrays
     //flag = new int[grid->GetnPoints()];                  //flag:( 0 = unknown, 1 = singel, 2 = double, 3 = final, -7 = outside)
     gcode = new int[grid->GetnPoints()];
     subroomUID = new int[grid->GetnPoints()];
     dist2Wall = new double[grid->GetnPoints()];
     speedInitial = new double[grid->GetnPoints()];
     modifiedspeed = new double[grid->GetnPoints()];
     cost = new double[grid->GetnPoints()];
     neggrad = new Point[grid->GetnPoints()];
     dirToWall = new Point[grid->GetnPoints()];
     //trialfield = new Trial[grid->GetnPoints()];                 //created with other arrays, but not initialized yet

     costmap.emplace(-1 , cost);                         // enable default ff (closest exit)
     neggradmap.emplace(-1, neggrad);

     //init grid with -3 as unknown distance to any wall
     for(long int i = 0; i < grid->GetnPoints(); ++i) {
          dist2Wall[i] = -3.;
          cost[i] = -2.;
          gcode[i] = OUTSIDE;            //unknown
     }
     //drawLinesOnGrid(wall, dist2Wall, 0.);
     //drawLinesOnGrid(wall, cost, -7.);
     //drawLinesOnGrid(wall, gcode, WALL);
     //drawLinesOnGrid(exitsFromScope, gcode, OPEN_TRANSITION);

     drawLinesOnGrid(wall, dist2Wall, 0.);
     drawLinesOnGrid(wall, cost, -7.);
     drawLinesOnGrid(wall, gcode, WALL);
     drawLinesOnGrid(exitsFromScope, gcode, OPEN_TRANSITION);
}

void LocalFloorfieldViaFM::drawBlockerLines() {
     //std::vector<Line> exits(wall.begin(), wall.begin()+numOfExits);

     //grid handeling local vars:
     long int iMax  = grid->GetiMax();

     long int iStart, iEnd;
     long int jStart, jEnd;
     long int iDot, jDot;
     long int key;
     long int deltaX, deltaY, deltaX1, deltaY1, px, py, xe, ye, i; //Bresenham Algorithm

     for(auto& line : wall) {
          key = grid->getKeyAtPoint(line.GetPoint1());
          iStart = grid->get_i_fromKey(key);
          jStart = grid->get_j_fromKey(key);

          key = grid->getKeyAtPoint(line.GetPoint2());
          iEnd = grid->get_i_fromKey(key);
          jEnd = grid->get_j_fromKey(key);

          deltaX = (int) (iEnd - iStart);
          deltaY = (int) (jEnd - jStart);
          deltaX1 = abs( (int) (iEnd - iStart));
          deltaY1 = abs( (int) (jEnd - jStart));

          px = 2*deltaY1 - deltaX1;
          py = 2*deltaX1 - deltaY1;

          if(deltaY1<=deltaX1) {
               if(deltaX>=0) {
                    iDot = iStart;
                    jDot = jStart;
                    xe = iEnd;
               } else {
                    iDot = iEnd;
                    jDot = jEnd;
                    xe = iStart;
               }
               crossOutOutsideNeighbors(jDot*iMax + iDot);
               for (i=0; iDot < xe; ++i) {
                    ++iDot;
                    if(px<0) {
                         px+=2*deltaY1;
                    } else {
                         if((deltaX<0 && deltaY<0) || (deltaX>0 && deltaY>0)) {
                              ++jDot;
                         } else {
                              --jDot;
                         }
                         px+=2*(deltaY1-deltaX1);
                    }
                    crossOutOutsideNeighbors(jDot*iMax + iDot);
               }
          } else {
               if(deltaY>=0) {
                    iDot = iStart;
                    jDot = jStart;
                    ye = jEnd;
               } else {
                    iDot = iEnd;
                    jDot = jEnd;
                    ye = jStart;
               }
               crossOutOutsideNeighbors(jDot*iMax + iDot);
               for(i=0; jDot<ye; ++i) {
                    ++jDot;
                    if (py<=0) {
                         py+=2*deltaX1;
                    } else {
                         if((deltaX<0 && deltaY<0) || (deltaX>0 && deltaY>0)) {
                              ++iDot;
                         } else {
                              --iDot;
                         }
                         py+=2*(deltaX1-deltaY1);
                    }
                    crossOutOutsideNeighbors(jDot*iMax + iDot);
               }
          }
     } //loop over all walls

     for(auto& line : exitsFromScope) {
          key = grid->getKeyAtPoint(line.GetPoint1());
          iStart = grid->get_i_fromKey(key);
          jStart = grid->get_j_fromKey(key);

          key = grid->getKeyAtPoint(line.GetPoint2());
          iEnd = grid->get_i_fromKey(key);
          jEnd = grid->get_j_fromKey(key);

          deltaX = (int) (iEnd - iStart);
          deltaY = (int) (jEnd - jStart);
          deltaX1 = abs( (int) (iEnd - iStart));
          deltaY1 = abs( (int) (jEnd - jStart));

          px = 2*deltaY1 - deltaX1;
          py = 2*deltaX1 - deltaY1;

          if(deltaY1<=deltaX1) {
               if(deltaX>=0) {
                    iDot = iStart;
                    jDot = jStart;
                    xe = iEnd;
               } else {
                    iDot = iEnd;
                    jDot = jEnd;
                    xe = iStart;
               }
               crossOutOutsideNeighbors(jDot*iMax + iDot);
               for (i=0; iDot < xe; ++i) {
                    ++iDot;
                    if(px<0) {
                         px+=2*deltaY1;
                    } else {
                         if((deltaX<0 && deltaY<0) || (deltaX>0 && deltaY>0)) {
                              ++jDot;
                         } else {
                              --jDot;
                         }
                         px+=2*(deltaY1-deltaX1);
                    }
                    crossOutOutsideNeighbors(jDot*iMax + iDot);
               }
          } else {
               if(deltaY>=0) {
                    iDot = iStart;
                    jDot = jStart;
                    ye = jEnd;
               } else {
                    iDot = iEnd;
                    jDot = jEnd;
                    ye = jStart;
               }
               crossOutOutsideNeighbors(jDot*iMax + iDot);
               for(i=0; jDot<ye; ++i) {
                    ++jDot;
                    if (py<=0) {
                         py+=2*deltaX1;
                    } else {
                         if((deltaX<0 && deltaY<0) || (deltaX>0 && deltaY>0)) {
                              ++iDot;
                         } else {
                              --iDot;
                         }
                         py+=2*(deltaX1-deltaY1);
                    }
                    crossOutOutsideNeighbors(jDot*iMax + iDot);
               }
          }
     } //loop over all exits

}

void LocalFloorfieldViaFM::crossOutOutsideNeighbors(const long int key){
//     directNeighbor dNeigh = grid->getNeighbors(key);
//     long int aux = -1;
//
//     const std::map<int, std::unique_ptr<SubRoom> >& subRoomMap = room->GetAllSubRooms();
//
//
//     aux = dNeigh.key[0];
////     if ((aux != -2) && (cost[aux] < -0.1) && (flag[aux] != -7) && (flag[aux] != -5)) { //aux is key of vaild girdpoint && gridpoint is not on exitline (exits have cost = 0 in prepareForDistance..())
////          Point trialP = grid->getPointFromKey(aux);               //^^ and gridpoint is not wall nor blockpoint
////          bool isInside = false;
////          for (int i = 0; i < subRoomMap.size(); ++i) {
////               auto subRoomIt = subRoomMap.begin();
////               std::advance(subRoomIt, i);
////               if ((*subRoomIt).second->IsInSubRoom(trialP)) {
////                    isInside = true;
////               }
////          }
//          if (!isInside(aux)) {
//               flag[aux] = -5;
//               dist2Wall[aux] = 0.; //set dist2Wall == 0 to save this points from updates in FloorfieldViaFM::clearAndPrepareForFloorfieldReCalc
//               speedInitial[aux] = .001;
//               cost[aux]         = -8.;
//          }
////     }
//     aux = dNeigh.key[1];
////     if ((aux != -2) && (cost[aux] < 0.) && (flag[aux] != -7) && (flag[aux] != -5)) {
////          Point trialP = grid->getPointFromKey(aux);
////          bool isInside = false;
////          for (int i = 0; i < subRoomMap.size(); ++i) {
////               auto subRoomIt = subRoomMap.begin();
////               std::advance(subRoomIt, i);
////               if ((*subRoomIt).second->IsInSubRoom(trialP)) {
////                    isInside = true;
////               }
////          }
////          if (!isInside) {
////               flag[aux] = -5;
////               dist2Wall[aux] = 0.;
////               speedInitial[aux] = .001;
////               cost[aux]         = -8.;
////          }
////     }
//     aux = dNeigh.key[2];
////     if ((aux != -2) && (cost[aux] < 0.) && (flag[aux] != -7) && (flag[aux] != -5)) {
////          Point trialP = grid->getPointFromKey(aux);
////          bool isInside = false;
////          for (int i = 0; i < subRoomMap.size(); ++i) {
////               auto subRoomIt = subRoomMap.begin();
////               std::advance(subRoomIt, i);
////               if ((*subRoomIt).second->IsInSubRoom(trialP)) {
////                    isInside = true;
////               }
////          }
////          if (!isInside) {
////               flag[aux] = -5;
////               dist2Wall[aux] = 0.;
////               speedInitial[aux] = .001;
////               cost[aux]         = -8.;
////          }
////     }
//     aux = dNeigh.key[3];
////     if ((aux != -2) && (cost[aux] < 0.) && (flag[aux] != -7) && (flag[aux] != -5)) {
////          Point trialP = grid->getPointFromKey(aux);
////          bool isInside = false;
////          for (int i = 0; i < subRoomMap.size(); ++i) {
////               auto subRoomIt = subRoomMap.begin();
////               std::advance(subRoomIt, i);
////               if ((*subRoomIt).second->IsInSubRoom(trialP)) {
////                    isInside = true;
////               }
////          }
////          if (!isInside) {
////               flag[aux] = -5;
////               dist2Wall[aux] = 0.;
////               speedInitial[aux] = .001;
////               cost[aux]         = -8.;
////          }
////     }
}

int LocalFloorfieldViaFM::isInside(const long int key) {
     int temp = 0;
     Point probe = grid->getPointFromKey(key);

     const std::map<int, std::shared_ptr<SubRoom>>& subRoomMap = room->GetAllSubRooms();

     for (auto& subRoomPair : subRoomMap) {

          SubRoom* subRoomPtr = subRoomPair.second.get();

          if (subRoomPtr->IsInSubRoom(probe)) {
               temp = subRoomPtr->GetUID();
          }
     }


     return temp;
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
     //Log->Write("INFO: \tStart Parsing: Room %d" , roomArg->GetUID());
     parseRoom(roomArg, hxArg, hyArg);
     //Log->Write("INFO: \tFinished Parsing: Room %d" , roomArg->GetUID());
     //testoutput("AALineScan.vtk", "AALineScan.txt", dist2Wall);

     prepareForDistanceFieldCalculation(false);
     //here we need to draw blocker lines @todo: ar.graf
     //Log->Write("INFO: \tGrid initialized: Walls");

     calculateDistanceField(-1.); //negative threshold is ignored, so all distances get calculated. this is important since distances is used for slowdown/redirect
     //Log->Write("INFO: \tGrid initialized: Walldistances");

     setSpeed(useDistancefield); //use distance2Wall
     //Log->Write("INFO: \tGrid initialized: Speed");
     calculateFloorfield(exitsFromScope, cost, neggrad);
};

void SubLocalFloorfieldViaFM::getDirectionToDestination(Pedestrian* ped,
      Point& direction)
{
     FloorfieldViaFM::getDirectionToDestination(ped, direction);
     return;
}

void SubLocalFloorfieldViaFM::getDirectionToGoalID(const int goalID){
     std::cerr << "invalid call to SubLocalFloorfieldViaFM::getDirectionToGoalID!" << std::endl;
};


void SubLocalFloorfieldViaFM::parseRoom(const SubRoom* const roomArg,
      const double hxArg, const double hyArg)
{
     subroom = roomArg;
     //init min/max before parsing
     double xMin = DBL_MAX;
     double xMax = -DBL_MAX;
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
                    exitsFromScope.emplace_back(Line((Line) *itTrans));
                    costmap.emplace(itTrans->GetUniqueID(), nullptr);
                    neggradmap.emplace(itTrans->GetUniqueID(), nullptr);
               }
          }
     }

     for (auto itCross : subroom->GetAllCrossings()) {
          if (itCross->IsOpen()) {
               exitsFromScope.emplace_back(Line((Line) *itCross));
               costmap.emplace(itCross->GetUniqueID(), nullptr);
               neggradmap.emplace(itCross->GetUniqueID(), nullptr);
          } else {
               wall.emplace_back(Line( (Line) *itCross));
          }
     }

     numOfExits = exitsFromScope.size();
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
               if ((*itObsWall).GetPoint1()._x < xMin) xMin = (*itObsWall).GetPoint1()._x;
               if ((*itObsWall).GetPoint2()._x < xMin) xMin = (*itObsWall).GetPoint2()._x;
               if ((*itObsWall).GetPoint1()._x > xMax) xMax = (*itObsWall).GetPoint1()._x;
               if ((*itObsWall).GetPoint2()._x > xMax) xMax = (*itObsWall).GetPoint2()._x;
               // yMin yMax
               if ((*itObsWall).GetPoint1()._y < yMin) yMin = (*itObsWall).GetPoint1()._y;
               if ((*itObsWall).GetPoint2()._y < yMin) yMin = (*itObsWall).GetPoint2()._y;
               if ((*itObsWall).GetPoint1()._y > yMax) yMax = (*itObsWall).GetPoint1()._y;
               if ((*itObsWall).GetPoint2()._y > yMax) yMax = (*itObsWall).GetPoint2()._y;
          }
     }

     std::vector<Wall> allWalls = subroom->GetAllWalls();
     for (std::vector<Wall>::iterator itWall = allWalls.begin(); itWall != allWalls.end(); ++itWall) {
          wall.emplace_back( Line( (Line) *itWall));
          // xMin xMax
          if ((*itWall).GetPoint1()._x < xMin) xMin = (*itWall).GetPoint1()._x;
          if ((*itWall).GetPoint2()._x < xMin) xMin = (*itWall).GetPoint2()._x;
          if ((*itWall).GetPoint1()._x > xMax) xMax = (*itWall).GetPoint1()._x;
          if ((*itWall).GetPoint2()._x > xMax) xMax = (*itWall).GetPoint2()._x;
          // yMin yMax
          if ((*itWall).GetPoint1()._y < yMin) yMin = (*itWall).GetPoint1()._y;
          if ((*itWall).GetPoint2()._y < yMin) yMin = (*itWall).GetPoint2()._y;
          if ((*itWall).GetPoint1()._y > yMax) yMax = (*itWall).GetPoint1()._y;
          if ((*itWall).GetPoint2()._y > yMax) yMax = (*itWall).GetPoint2()._y;
     }

     const vector<Crossing*>& allCrossings = subroom->GetAllCrossings();
     for (Crossing* crossPtr : allCrossings) {
          if (!crossPtr->IsOpen()) {
               wall.emplace_back( Line( (Line) *crossPtr));

               if (crossPtr->GetPoint1()._x < xMin) xMin = crossPtr->GetPoint1()._x;
               if (crossPtr->GetPoint2()._x < xMin) xMin = crossPtr->GetPoint2()._x;
               if (crossPtr->GetPoint1()._x > xMax) xMax = crossPtr->GetPoint1()._x;
               if (crossPtr->GetPoint2()._x > xMax) xMax = crossPtr->GetPoint2()._x;

               if (crossPtr->GetPoint1()._y < yMin) yMin = crossPtr->GetPoint1()._y;
               if (crossPtr->GetPoint2()._y < yMin) yMin = crossPtr->GetPoint2()._y;
               if (crossPtr->GetPoint1()._y > yMax) yMax = crossPtr->GetPoint1()._y;
               if (crossPtr->GetPoint2()._y > yMax) yMax = crossPtr->GetPoint2()._y;
          }
     }


     //create Rect Grid
     grid = new RectGrid();
     grid->setBoundaries(xMin, yMin, xMax, yMax);
     grid->setSpacing(hxArg, hyArg);
     grid->createGrid();

     //create arrays
     gcode = new int[grid->GetnPoints()];                  //flag:( 0 = unknown, 1 = singel, 2 = double, 3 = final, -7 = outside)
     subroomUID = new int[grid->GetnPoints()];
     dist2Wall = new double[grid->GetnPoints()];
     speedInitial = new double[grid->GetnPoints()];
     modifiedspeed = new double[grid->GetnPoints()];
     cost = new double[grid->GetnPoints()];
     neggrad = new Point[grid->GetnPoints()];
     dirToWall = new Point[grid->GetnPoints()];
     //trialfield = new Trial[grid->GetnPoints()];                 //created with other arrays, but not initialized yet

     costmap.emplace(-1 , cost);                         // enable default ff (closest exit)
     neggradmap.emplace(-1, neggrad);

     //init grid with -3 as unknown distance to any wall
     for(long int i = 0; i < grid->GetnPoints(); ++i) {
          dist2Wall[i] = -3.;
          cost[i] = -2.;
          gcode[i] = OUTSIDE;
     }
     //drawLinesOnGrid(wall, gcode, WALL);
     //drawLinesOnGrid(exitsFromScope, gcode, OPEN_TRANSITION);
     drawLinesOnGrid(wall, dist2Wall, 0.);
     drawLinesOnGrid(wall, cost, -7.);
     drawLinesOnGrid(wall, gcode, WALL);
     drawLinesOnGrid(exitsFromScope, gcode, OPEN_TRANSITION);
}

int SubLocalFloorfieldViaFM::isInside(const long int key) {
     Point probe = grid->getPointFromKey(key);
     return subroom->IsInSubRoom(probe)?subroom->GetUID():0;
}