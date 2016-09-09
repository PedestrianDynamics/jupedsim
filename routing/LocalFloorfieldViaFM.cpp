//
// Created by ar.graf on 1/28/16.
//

// @todo f.mack move to macros.h
#define OUTSIDE_SUBROOM_PTR (SubRoom*)0x0751DE

#include "LocalFloorfieldViaFM.h"
#include <chrono>

LocalFloorfieldViaFM::LocalFloorfieldViaFM(){};
LocalFloorfieldViaFM::LocalFloorfieldViaFM(const Room* const roomArg,
               const Building* buildingArg,
               const double hxArg, const double hyArg,
               const double wallAvoidDistance, const bool useDistancefield) {
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

//void LocalFloorfieldViaFM::getDirectionToGoalID(const int goalID){
//     std::cerr << "invalid call to LocalFloorfieldViaFM::getDirectionToGoalID!" << std::endl;
//};

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
     numOfExits = (unsigned int) exitsFromScope.size();
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

     _subroomGrid = new RectGrid();
     _subroomGrid->setBoundaries(xMin, yMin, xMax, yMax);
     _subroomGrid->setSpacing(_subroomGridSpacing, _subroomGridSpacing);
     _subroomGrid->createGrid();
     //Log->Write("LocalFF for room %d: There are %ld points in _subroomGrid", room->GetID(), _subroomGrid->GetnPoints());
     std::chrono::time_point<std::chrono::system_clock> start, end;
     start = std::chrono::system_clock::now();
     // There was a case where _subroomGrid->GetKeyAtPoint(p) returns values >= nPoints (the check for includesPoints(p) was successful).
     // So we insert some additional entries to _subroomMap ("<=" instead of "<") to avoid out_of_range errors. --f.mack
     // @todo Investigate why this is happening.
     long int max_index = _subroomGrid->GetnPoints() + _subroomGrid->GetiMax() + _subroomGrid->GetjMax();
     for (long int i = 0; i < max_index; ++i) {
          SubRoom* subroom = isInsideInit(i);
          _subroomMap.insert(std::make_pair(i, subroom));
     }
     end = std::chrono::system_clock::now();
     std::chrono::duration<double> elapsed_seconds = end-start;
     Log->Write("INFO: \tinitializing _subroomMap (room %d, %ld points):", room->GetID(), _subroomGrid->GetnPoints());
     Log->Write("      \ttime: "+ std::to_string(elapsed_seconds.count()));

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
     densityspeed = new double[grid->GetnPoints()];
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
     //long int iMax  = grid->GetiMax();

     long int iStart, iEnd;
     long int jStart, jEnd;
     long int iDot, jDot;
     long int key;
     long int deltaX, deltaY, deltaX1, deltaY1, px, py, xe, ye, i; //Bresenham Algorithm

     for(auto& line : wall) {
          key = grid->getKeyAtPoint(line.GetPoint1());
          iStart = (long) grid->get_i_fromKey(key);
          jStart = (long) grid->get_j_fromKey(key);

          key = grid->getKeyAtPoint(line.GetPoint2());
          iEnd = (long) grid->get_i_fromKey(key);
          jEnd = (long) grid->get_j_fromKey(key);

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
               //crossOutOutsideNeighbors(jDot*iMax + iDot);
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
                    //crossOutOutsideNeighbors(jDot*iMax + iDot);
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
               //crossOutOutsideNeighbors(jDot*iMax + iDot);
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
                   // crossOutOutsideNeighbors(jDot*iMax + iDot);
               }
          }
     } //loop over all walls

     for(auto& line : exitsFromScope) {
          key = grid->getKeyAtPoint(line.GetPoint1());
          iStart = (long) grid->get_i_fromKey(key);
          jStart = (long) grid->get_j_fromKey(key);

          key = grid->getKeyAtPoint(line.GetPoint2());
          iEnd = (long) grid->get_i_fromKey(key);
          jEnd = (long) grid->get_j_fromKey(key);

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
               //crossOutOutsideNeighbors(jDot*iMax + iDot);
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
                    //crossOutOutsideNeighbors(jDot*iMax + iDot);
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
               //crossOutOutsideNeighbors(jDot*iMax + iDot);
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
                   // crossOutOutsideNeighbors(jDot*iMax + iDot);
               }
          }
     } //loop over all exits

}
//@todo Arne: Is this function needed? delete?
//void LocalFloorfieldViaFM::crossOutOutsideNeighbors(const long int key){
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
//}

SubRoom* LocalFloorfieldViaFM::isInsideInit(const long int key) {
     Point probe = _subroomGrid->getPointFromKey(key);

     const std::map<int, std::shared_ptr<SubRoom>>& subRoomMap = room->GetAllSubRooms();

     for (auto& subRoomPair : subRoomMap) {

          SubRoom* subRoomPtr = subRoomPair.second.get();

          if (subRoomPtr->IsInSubRoom(probe)) {
               return subRoomPtr;
          }
     }


     return OUTSIDE_SUBROOM_PTR;
}

int LocalFloorfieldViaFM::isInside(const long int key) {
     Point probe = grid->getPointFromKey(key);

     // rounds downwards to the next number that is a multiple of _subroomGridSpacing
     // if have to correct this by the offset because the _subroomGrid messes around with the values
     // @todo f.mack shouldn't it be probe._x - 1 + xOffset?
     double xOffset = fmod(_subroomGrid->GetxMin(), 1.);
     double yOffset = fmod(_subroomGrid->GetyMin(), 1.);
     double lowerX = _subroomGridSpacing * floor((probe._x - 1 + xOffset) / _subroomGridSpacing);
     double lowerY = _subroomGridSpacing * floor((probe._y - 1 + yOffset) / _subroomGridSpacing);

     Point subroomPoint = Point(lowerX, lowerY);
     Point p1 = subroomPoint;
     SubRoom* sub1 = _subroomGrid->includesPoint(subroomPoint) ? _subroomMap.at(_subroomGrid->getKeyAtPoint(subroomPoint)) : nullptr;
     if (sub1 && sub1 != OUTSIDE_SUBROOM_PTR && sub1->IsInSubRoom(probe)) return sub1->GetUID();

     subroomPoint._x += _subroomGridSpacing;
     Point p2 = subroomPoint;
     SubRoom* sub2 = _subroomGrid->includesPoint(subroomPoint) ? _subroomMap.at(_subroomGrid->getKeyAtPoint(subroomPoint)) : nullptr;
     if (sub2 && sub2 != OUTSIDE_SUBROOM_PTR && sub2 != sub1 && sub2->IsInSubRoom(probe)) return sub2->GetUID();

     subroomPoint._y += _subroomGridSpacing;
     Point p3 = subroomPoint;
     SubRoom* sub3;
     if (_subroomGrid->includesPoint(subroomPoint)) {
          long int sr_key = _subroomGrid->getKeyAtPoint(subroomPoint);
          sub3 = _subroomMap.at(sr_key);
     } else {
          sub3 = nullptr;
     }
     //SubRoom* sub3 = _subroomGrid->includesPoint(subroomPoint) ? _subroomMap.at(_subroomGrid->getKeyAtPoint(subroomPoint)) : nullptr;
     if (sub3 && sub3 != OUTSIDE_SUBROOM_PTR && sub3 != sub1 && sub3 != sub2 && sub3->IsInSubRoom(probe)) return sub3->GetUID();

     subroomPoint._x -= _subroomGridSpacing;
     Point p4 = subroomPoint;
     SubRoom* sub4 = _subroomGrid->includesPoint(subroomPoint) ? _subroomMap.at(_subroomGrid->getKeyAtPoint(subroomPoint)) : nullptr;
     if (sub4 && sub4 != OUTSIDE_SUBROOM_PTR && sub4 != sub1 && sub4 != sub2 && sub4 != sub3 && sub4->IsInSubRoom(probe)) return sub4->GetUID();

     bool b = false;
     if (sub1 == OUTSIDE_SUBROOM_PTR && sub2 == OUTSIDE_SUBROOM_PTR && sub3 == OUTSIDE_SUBROOM_PTR && sub4 == OUTSIDE_SUBROOM_PTR) {
          //return 0;
          b = true;
     }//*/
     //Log->Write("####### LocFF for room %d: for point %f, %f, trying all subrooms", room->GetID(), probe._x, probe._y);

     const std::map<int, std::shared_ptr<SubRoom>>& subRoomMap = room->GetAllSubRooms();

     for (auto& subRoomPair : subRoomMap) {

          SubRoom* subRoomPtr = subRoomPair.second.get();
          if (subRoomPtr == sub1 || subRoomPtr == sub2 || subRoomPtr == sub3 || subRoomPtr == sub4) continue;

          if (subRoomPtr->IsInSubRoom(probe)) {
               if (b) {
                    Log->Write("####### point %f, %f is surrounded by outside, but is in subroom [%d/%d]", probe._x, probe._y, subRoomPtr->GetRoomID(), subRoomPtr->GetSubRoomID());
                    Log->Write("surrounding points: %f, %f     %f, %f     %f, %f     %f, %f", p1._x, p1._y, p2._x, p2._y, p3._x, p3._y, p4._x, p4._y);
                    Log->Write("subrooms are: %p    %p    %p    %p", sub1, sub2, sub3, sub4);
                    Log->Write("keys are: %ld   %ld   %ld   %ld", _subroomGrid->getKeyAtPoint(p1), _subroomGrid->getKeyAtPoint(p2), _subroomGrid->getKeyAtPoint(p3), _subroomGrid->getKeyAtPoint(p4));
               }
               return subRoomPtr->GetUID();
          }
     }


     return 0;
}

SubLocalFloorfieldViaFM::SubLocalFloorfieldViaFM(){};
SubLocalFloorfieldViaFM::SubLocalFloorfieldViaFM(const SubRoom* const roomArg,
      const Building* buildingArg,
      const double hxArg, const double hyArg,
      const double wallAvoidDistance, const bool useDistancefield) {
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
     std::cerr << "invalid call to SubLocalFloorfieldViaFM::getDirectionToGoalID with goalID: " << goalID << std::endl;
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
     densityspeed = new double[grid->GetnPoints()];
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