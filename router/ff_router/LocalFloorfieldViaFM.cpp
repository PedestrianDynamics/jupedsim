//
// Created by ar.graf on 1/28/16.
//
#include "LocalFloorfieldViaFM.h"

LocalFloorfieldViaFM::LocalFloorfieldViaFM(){};
LocalFloorfieldViaFM::LocalFloorfieldViaFM(const Room* const roomArg,
               const Building* buildingArg,
               const double hxArg, const double hyArg,
               const double wallAvoidDistance, const bool useDistancefield) {
     //ctor
     //_threshold = -1; //negative value means: ignore threshold
     _threshold = wallAvoidDistance;
     _building = buildingArg;
     _room = roomArg;

     if (hxArg != hyArg) std::cerr << "ERROR: hx != hy <=========";
     //parse building and create list of walls/obstacles (find xmin xmax, ymin, ymax, and add border?)
     //Log->Write("INFO: \tStart Parsing: Room %d", roomArg->GetID());
     parseRoom(roomArg, hxArg, hyArg);
     //Log->Write("INFO: \tFinished Parsing: Room %d", roomArg->GetID());
     //testoutput("AALineScan.vtk", "AALineScan.txt", dist2Wall);

     prepareForDistanceFieldCalculation(false);
     //here we need to draw blocker lines @todo: ar.graf
     //drawBlockerLines();
     //Log->Write("INFO: \tGrid initialized: Walls in room %d", roomArg->GetID());

     calculateDistanceField(-1.); //negative threshold is ignored, so all distances get calculated. this is important since distances is used for slowdown/redirect
     //Log->Write("INFO: \tGrid initialized: Walldistances in room %d", roomArg->GetID());

     setSpeed(useDistancefield); //use distance2Wall
     //Log->Write("INFO: \tGrid initialized: Speed in room %d", roomArg->GetID());
     calculateFloorfield(_exitsFromScope, _cost, _neggrad);
     //Log->Write("INFO: \tFloor field for \"goal -1\" done in room %d", roomArg->GetID());
};

//void LocalFloorfieldViaFM::getDirectionToGoalID(const int goalID){
//     std::cerr << "invalid call to LocalFloorfieldViaFM::getDirectionToGoalID!" << std::endl;
//};

void LocalFloorfieldViaFM::parseRoom(const Room* const roomArg,
      const double hxArg, const double hyArg)
{
     _room = roomArg;
     //init min/max before parsing
     double xMin = DBL_MAX;
     double xMax = -DBL_MAX;
     double yMin = xMin;
     double yMax = xMax;
     _costmap.clear();
     _neggradmap.clear();

     //create a list of walls
     //add all transition and put open doors at the beginning of "wall"
     std::map<int, Transition*> allTransitions;
     for (auto& itSubroom : _room->GetAllSubRooms()) {
          for (auto itTrans : itSubroom.second->GetAllTransitions()) {
               if (!allTransitions.count(itTrans->GetUniqueID())) {
                    allTransitions[itTrans->GetUniqueID()] = &(*itTrans);
                    if (itTrans->IsOpen()) {
                         _exitsFromScope.emplace_back(Line((Line) *itTrans));
                         _costmap.emplace(itTrans->GetUniqueID(), nullptr);
                         _neggradmap.emplace(itTrans->GetUniqueID(), nullptr);
                    }
               }
          }
     }
     _numOfExits = (unsigned int) _exitsFromScope.size();
     //put closed doors next, they are considered as walls later (index >= numOfExits)
     for (auto& trans : allTransitions) {
          if (!trans.second->IsOpen()) {
               _wall.emplace_back(Line ( (Line) *(trans.second)));
          }
     }

     for (const auto& itSubroom : _room->GetAllSubRooms()) {
          std::vector<Obstacle*> allObstacles = itSubroom.second->GetAllObstacles();
          for (std::vector<Obstacle*>::iterator itObstacles = allObstacles.begin(); itObstacles != allObstacles.end(); ++itObstacles) {

               std::vector<Wall> allObsWalls = (*itObstacles)->GetAllWalls();
               for (std::vector<Wall>::iterator itObsWall = allObsWalls.begin(); itObsWall != allObsWalls.end(); ++itObsWall) {
                    _wall.emplace_back(Line( (Line) *itObsWall));
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
               _wall.emplace_back( Line( (Line) *itWall));
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
                    _wall.emplace_back( Line( (Line) *crossPtr));

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
     _grid = new RectGrid();
     _grid->setBoundaries(xMin, yMin, xMax, yMax);
     _grid->setSpacing(hxArg, hyArg);
     _grid->createGrid();

     //create arrays
     //flag = new int[grid->GetnPoints()];
     _gcode = new int[_grid->GetnPoints()];
     _subrooms = new SubRoom*[_grid->GetnPoints()](); // initializes with nullptr
     _dist2Wall = new double[_grid->GetnPoints()];
     _speedInitial = new double[_grid->GetnPoints()];
     _modifiedspeed = new double[_grid->GetnPoints()];
     _densityspeed = new double[_grid->GetnPoints()];
     _cost = new double[_grid->GetnPoints()];
     _neggrad = new Point[_grid->GetnPoints()];
     _dirToWall = new Point[_grid->GetnPoints()];
     //trialfield = new Trial[grid->GetnPoints()];                 //created with other arrays, but not initialized yet

     _costmap.emplace(-1 , _cost);                         // enable default ff (closest exit)
     _neggradmap.emplace(-1, _neggrad);

     //init grid with -3 as unknown distance to any wall
     for(long int i = 0; i < _grid->GetnPoints(); ++i) {
          _dist2Wall[i] = -3.;
          _cost[i] = -2.;
          _gcode[i] = OUTSIDE;            //unknown
     }

     drawLinesOnGrid<double>(_wall, _dist2Wall, 0.);
     drawLinesOnGrid<double>(_wall, _cost, -7.);
     drawLinesOnGrid<int>(_wall, _gcode, WALL);
     drawLinesOnGrid<int>(_exitsFromScope, _gcode, OPEN_TRANSITION);
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

     for(auto& line : _wall) {
          key = _grid->getKeyAtPoint(line.GetPoint1());
          iStart = (long) _grid->get_i_fromKey(key);
          jStart = (long) _grid->get_j_fromKey(key);

          key = _grid->getKeyAtPoint(line.GetPoint2());
          iEnd = (long) _grid->get_i_fromKey(key);
          jEnd = (long) _grid->get_j_fromKey(key);

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

     for(auto& line : _exitsFromScope) {
          key = _grid->getKeyAtPoint(line.GetPoint1());
          iStart = (long) _grid->get_i_fromKey(key);
          jStart = (long) _grid->get_j_fromKey(key);

          key = _grid->getKeyAtPoint(line.GetPoint2());
          iEnd = (long) _grid->get_i_fromKey(key);
          jEnd = (long) _grid->get_j_fromKey(key);

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

SubRoom* LocalFloorfieldViaFM::isInside(const long int key) {
     Point probe = _grid->getPointFromKey(key);
     auto neighbors = _grid->getNeighbors(key);

     for (auto& neighbor : neighbors.key) {
          if (neighbor == -2) continue; // -2 is returned by getNeighbors() for invalid points
          SubRoom* subroom = _subrooms[neighbor];
          if (subroom && subroom->IsInSubRoom(probe)) return subroom;
     }

     // If we weren't successful so far, we have to search the whole list.
     const std::map<int, std::shared_ptr<SubRoom>>& subRoomMap = _room->GetAllSubRooms();

     for (auto& subRoomPair : subRoomMap) {
          SubRoom* subRoomPtr = subRoomPair.second.get();
          if (subRoomPtr->IsInSubRoom(probe)) {
               return subRoomPtr;
          }
     }

     return nullptr;
}

SubLocalFloorfieldViaFM::SubLocalFloorfieldViaFM(){};
SubLocalFloorfieldViaFM::SubLocalFloorfieldViaFM(SubRoom* const roomArg,
      const Building* buildingArg,
      const double hxArg, const double hyArg,
      const double wallAvoidDistance, const bool useDistancefield) {
     //ctor
     //_threshold = -1; //negative value means: ignore threshold
     _threshold = wallAvoidDistance;
     _building = buildingArg;
     _subroom = roomArg;

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
     calculateFloorfield(_exitsFromScope, _cost, _neggrad);
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


void SubLocalFloorfieldViaFM::parseRoom(SubRoom* const roomArg,
      const double hxArg, const double hyArg)
{
     _subroom = roomArg;
     //init min/max before parsing
     double xMin = DBL_MAX;
     double xMax = -DBL_MAX;
     double yMin = xMin;
     double yMax = xMax;
     _costmap.clear();
     _neggradmap.clear();

     //create a list of walls
     //add all transition and put open doors at the beginning of "wall"
     std::map<int, Transition*> allTransitions;

     for (auto itTrans : _subroom->GetAllTransitions()) {
          if (!allTransitions.count(itTrans->GetUniqueID())) {
               allTransitions[itTrans->GetUniqueID()] = &(*itTrans);
               if (itTrans->IsOpen()) {
                    _exitsFromScope.emplace_back(Line((Line) *itTrans));
                    _costmap.emplace(itTrans->GetUniqueID(), nullptr);
                    _neggradmap.emplace(itTrans->GetUniqueID(), nullptr);
               }
          }
     }

     for (auto itCross : _subroom->GetAllCrossings()) {
          if (itCross->IsOpen()) {
               _exitsFromScope.emplace_back(Line((Line) *itCross));
               _costmap.emplace(itCross->GetUniqueID(), nullptr);
               _neggradmap.emplace(itCross->GetUniqueID(), nullptr);
          } else {
               _wall.emplace_back(Line( (Line) *itCross));
          }
     }

     _numOfExits = _exitsFromScope.size();
     //put closed doors next, they are considered as walls later (index >= numOfExits)
     for (auto& trans : allTransitions) {
          if (!trans.second->IsOpen()) {
               _wall.emplace_back(Line ( (Line) *(trans.second)));
          }
     }


     std::vector<Obstacle*> allObstacles = _subroom->GetAllObstacles();
     for (std::vector<Obstacle*>::iterator itObstacles = allObstacles.begin(); itObstacles != allObstacles.end(); ++itObstacles) {

          std::vector<Wall> allObsWalls = (*itObstacles)->GetAllWalls();
          for (std::vector<Wall>::iterator itObsWall = allObsWalls.begin(); itObsWall != allObsWalls.end(); ++itObsWall) {
               _wall.emplace_back(Line( (Line) *itObsWall));
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

     std::vector<Wall> allWalls = _subroom->GetAllWalls();
     for (std::vector<Wall>::iterator itWall = allWalls.begin(); itWall != allWalls.end(); ++itWall) {
          _wall.emplace_back( Line( (Line) *itWall));
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

     const vector<Crossing*>& allCrossings = _subroom->GetAllCrossings();
     for (Crossing* crossPtr : allCrossings) {
          if (!crossPtr->IsOpen()) {
               _wall.emplace_back( Line( (Line) *crossPtr));

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
     _grid = new RectGrid();
     _grid->setBoundaries(xMin, yMin, xMax, yMax);
     _grid->setSpacing(hxArg, hyArg);
     _grid->createGrid();

     //create arrays
     _gcode = new int[_grid->GetnPoints()];
     _subrooms = new SubRoom*[_grid->GetnPoints()](); // initializes with nullptr
     _dist2Wall = new double[_grid->GetnPoints()];
     _speedInitial = new double[_grid->GetnPoints()];
     _modifiedspeed = new double[_grid->GetnPoints()];
     _densityspeed = new double[_grid->GetnPoints()];
     _cost = new double[_grid->GetnPoints()];
     _neggrad = new Point[_grid->GetnPoints()];
     _dirToWall = new Point[_grid->GetnPoints()];
     //trialfield = new Trial[grid->GetnPoints()];                 //created with other arrays, but not initialized yet

     _costmap.emplace(-1 , _cost);                         // enable default ff (closest exit)
     _neggradmap.emplace(-1, _neggrad);

     //init grid with -3 as unknown distance to any wall
     for(long int i = 0; i < _grid->GetnPoints(); ++i) {
          _dist2Wall[i] = -3.;
          _cost[i] = -2.;
          _gcode[i] = OUTSIDE;
     }

     drawLinesOnGrid<double>(_wall, _dist2Wall, 0.);
     drawLinesOnGrid<double>(_wall, _cost, -7.);
     drawLinesOnGrid<int>(_wall, _gcode, WALL);
     drawLinesOnGrid<int>(_exitsFromScope, _gcode, OPEN_TRANSITION);
}

SubRoom* SubLocalFloorfieldViaFM::isInside(const long int key) {
     Point probe = _grid->getPointFromKey(key);
     return _subroom->IsInSubRoom(probe) ? _subroom : nullptr;
}
