//
// Created by Tobias Schrödter on 2019-06-11.
//

#include "WaitingProbability.h"

#include "../../precomputation/floorfield/mesh/RectGrid.h"

#include "../../../geometry/Room.h"
#include "../../../geometry/SubRoom.h"
#include "../../../pedestrian/Pedestrian.h"
#include "../../../geometry/Point.h"

#include <iomanip>

void  WaitingProbability::Init(Building* building){
     Log->Write("Start init Waiting Probability");
     _building = building;
     parseBuilding();

     computeStatic();
//     drawLinesOnGrid<double>(_exitsFromScope, _distance, OPEN_TRANSITION);
//     drawLinesOnGrid<double>(_wall, _distance, CLOSED_TRANSITION);

//     writeVTK(_grid, std::string("waiting.vtk"));
     Log->Write("End init Waiting Probability-----------------");

}

void WaitingProbability::parseBuilding(){
//     double stepSizeX = 0.3;
//     double stepSizeY = 0.3;

     double stepSizeX = 0.01;
     double stepSizeY = 0.01;

     std::vector<Line> _wall;
     std::vector<Line> _exitsFromScope;


     for (auto roomItr : _building->GetAllRooms()){
          for (auto subRoomItr : roomItr.second->GetAllSubRooms()){
               std::shared_ptr<SubRoom> subroom = subRoomItr.second;

               for (auto trans : subroom->GetAllTransitions()) {
                    if (trans->IsExit() && !trans->IsClose()){
                         _exitsFromScope.emplace_back(Line ( (Line) *(trans)));
                    } else if (!trans->IsOpen()){
                         _wall.emplace_back(Line ( (Line) *(trans)));
                    }
               }

//               _numOfExits = (unsigned int) _exitsFromScope.size();

               for (auto obsItr : subroom->GetAllObstacles()){
                    for (auto obsWalls : obsItr->GetAllWalls()){
                         _wall.emplace_back(Line( (Line) obsWalls));
                    }
               }

               for (auto itWall : subroom->GetAllWalls()) {
                    _wall.emplace_back( Line( (Line) itWall));
               }

               for (auto cross : subroom->GetAllCrossings()) {
                    if (!cross->IsOpen()) {
                         _wall.emplace_back(Line((Line) *cross));
                    }
               }

               std::vector<double> bb = subroom->GetBoundingBox();
               double xMin = bb[0];
               double xMax = bb[1];
               double yMin = bb[2];
               double yMax = bb[3];

               // Create rect grid
               int uid = subroom->GetUID();
               _gridMap[uid] = new RectGrid();

//               _gridMap[uid]->setBoundaries(xMin-stepSizeX, yMin-stepSizeY, xMax+stepSizeX, yMax+stepSizeY);
               _gridMap[uid]->setBoundaries(xMin, yMin, xMax, yMax);
               _gridMap[uid]->setSpacing(stepSizeX, stepSizeX);
               _gridMap[uid]->createGrid();

               _flowMap[uid]=std::vector<double>(_gridMap[uid]->GetnPoints(), 0.);
               _boundaryMap[uid]=std::vector<double>(_gridMap[uid]->GetnPoints(), 0.);
               _distanceMap[uid]=std::vector<double>(_gridMap[uid]->GetnPoints(), 0.);
               _angleMap[uid]=std::vector<double>(_gridMap[uid]->GetnPoints(), 0.);
               _staticMap[uid]=std::vector<double>(_gridMap[uid]->GetnPoints(), 0.);
          }
     }
}

Point WaitingProbability::GetWaitingPosition(Room* room, Pedestrian* ped) const{
     SubRoom* subRoom = room->GetSubRoom(ped->GetSubRoomID());
     return subRoom->GetCentroid();
}

void WaitingProbability::computeStatic(){

     for (auto roomItr : _building->GetAllRooms()){
          for (auto subRoomItr : roomItr.second->GetAllSubRooms()) {
               std::shared_ptr<SubRoom> subroom = subRoomItr.second;
               computeDistanceCost(subroom);
               computeAngleCost(subroom);
               writeVTK(subroom, "prob_" + std::to_string(subroom->GetUID()) + ".vtk");
          }
     }
}

void WaitingProbability::computeFlowAvoidance(){

}

void WaitingProbability::computeBoundaryPreference(){

}

void WaitingProbability::computeDistanceCost(std::shared_ptr<SubRoom> subroom){
     double x,y, minDist, tmpDist;

     int uid = subroom->GetUID();
     for (int i=0; i< _gridMap.at(uid)->GetnPoints(); ++i) {
          minDist = DBL_MAX;

          x = _gridMap.at(uid)->get_x_fromKey(i);
          y = _gridMap.at(uid)->get_y_fromKey(i);
          Point p(x, y);

          if (subroom->IsInSubRoom(p)){
               for (auto trans : subroom->GetAllTransitions()) {
                    minDist = std::min(minDist, Distance(p, trans->GetCentre()));
               }
               _distanceMap.at(uid).at(i) = minDist;
          }
     }
}

void WaitingProbability::computeAngleCost(std::shared_ptr<SubRoom> subroom){
     double x,y, minAngle, tmpAngle1, tmpAngle2, tmpAngleC;

     int uid = subroom->GetUID();
     std::fill(_angleMap.at(uid).begin(), _angleMap.at(uid).end(), -1);

     for (int i=0; i< _gridMap.at(uid)->GetnPoints(); ++i) {
          minAngle = DBL_MAX;
          tmpAngle1 = DBL_MAX;
          tmpAngle2 = DBL_MAX;
          tmpAngleC = DBL_MAX;

          x = _gridMap.at(uid)->get_x_fromKey(i);
          y = _gridMap.at(uid)->get_y_fromKey(i);
          Point p(x, y);

          if (subroom->IsInSubRoom(p)){
               for (auto trans : subroom->GetAllTransitions()) {
                    Point p0 = trans->GetPoint2() - trans->GetPoint1();

                    Point l1 = trans->GetPoint1() -p;
                    double angle1 = Angle(p0, l1);

                    Point l2 = trans->GetPoint2() -p;
                    double angle2 = Angle(p0, l2);

                    minAngle = std::min(minAngle, checkAngles(angle1, angle2));

               }

               _angleMap.at(uid).at(i) = minAngle;
          }
     }
}

double WaitingProbability::checkAngles(double a, double b){
     double pi = 3.141592653589793238463;
     double ad = a * (180.0/pi) ;
     double bd = b * (180.0/pi) ;

     if (ad >= 90 && bd <= 90){
          return 90 * pi / 180.0;
     } else {
          if (ad > 90 && bd > 90){
               a -= 90 * pi / 180.0;
               b -= 90 * pi / 180.0;
               return 90 * pi / 180.0 - std::min(a,b);
          } else {
               return std::max(a, b);
          }
     }

}

void WaitingProbability::writeVTK( std::shared_ptr<SubRoom> subroom, std::string filename){

     Log->Write("INFO: \tWrite Floorfield to file");
     Log->Write(filename);
     std::ofstream file;

     RectGrid* grid = _gridMap[subroom->GetUID()];
     int uid = subroom->GetUID();
     file.open(_building->GetConfig()->GetProjectRootDir()+filename);

//     file << "# vtk DataFile Version 3.0" << std::endl;
//     file << "Testdata: Fast Marching: Test: " << std::endl;
//     file << "ASCII" << std::endl;
//     file << "DATASET STRUCTURED_GRID" << std::endl;
//     file << "DIMENSIONS " <<  std::to_string(grid->GetiMax()) << " " <<
//          std::to_string(grid->GetjMax()) << " 1" << std::endl;
//     file << "POINTS " << std::to_string(grid->GetnPoints()) << " float" << std::endl;
//     for (long int i = 0; i < grid->GetnPoints(); ++i) {
//          file << grid->get_x_fromKey(i) << " " << grid->get_y_fromKey(i) << " 0" << std::endl;
//     }

     file << "# vtk DataFile Version 3.0" << std::endl;
     file << "WaitingProb Test: " << std::endl;
     file << "ASCII" << std::endl;
     file << "DATASET STRUCTURED_POINTS" << std::endl;
     file << "DIMENSIONS " <<  std::to_string(grid->GetiMax()) << " " <<
          std::to_string(grid->GetjMax()) << " 1" << std::endl;
     file << "ORIGIN " << grid->GetxMin() << " " << grid->GetyMin() << " 0" << std::endl;
     file << "SPACING " << std::to_string(grid->Gethx()) << " " << std::to_string(grid->Gethy()) << " 1" << std::endl;
     file << "POINT_DATA " << std::to_string(grid->GetnPoints()) << std::endl;

     // Print gcode//
     file << "SCALARS flow float 1" << std::endl;
     file << "LOOKUP_TABLE default" << std::endl;
     for (long int i = 0; i < grid->GetnPoints(); ++i) {
          file << _flowMap.at(uid).at(i) << std::endl;
     }

     file << "SCALARS boundary float 1" << std::endl;
     file << "LOOKUP_TABLE default" << std::endl;
     for (long int i = 0; i < grid->GetnPoints(); ++i) {
          file << _boundaryMap.at(uid).at(i) << std::endl;
     }

     file << "SCALARS distance float 1" << std::endl;
     file << "LOOKUP_TABLE default" << std::endl;
     for (long int i = 0; i < grid->GetnPoints(); ++i) {
          file << _distanceMap.at(uid).at(i) << std::endl;
     }

     file << "SCALARS angle float 1" << std::endl;
     file << "LOOKUP_TABLE default" << std::endl;
     for (long int i = 0; i < grid->GetnPoints(); ++i) {
          file << _angleMap.at(uid).at(i) << std::endl;
     }

     file << "SCALARS static float 1" << std::endl;
     file << "LOOKUP_TABLE static" << std::endl;
     for (long int i = 0; i < grid->GetnPoints(); ++i) {
          file << _staticMap.at(uid).at(i) << std::endl;
     }

     file.close();
}

template <typename T>
void WaitingProbability::drawLinesOnGrid(std::vector<Line>& wallArg, T* const target, const T value) { //no init, plz init elsewhere
// i~x; j~y;
//http://stackoverflow.com/questions/10060046/drawing-lines-with-bresenhams-line-algorithm
//src in answer of "Avi"; adapted to fit this application

//    //init with inside value:
//    long int indexMax = grid->GetnPoints();
//    for (long int i = 0; i < indexMax; ++i) {
//        target[i] = inside;
//    }

     //grid handeling local vars:
//     long int iMax  = _grid->GetiMax();
//
//     long int iStart, iEnd;
//     long int jStart, jEnd;
//     long int iDot, jDot;
//     long int key;
//     long int deltaX, deltaY, deltaX1, deltaY1, px, py, xe, ye, i; //Bresenham Algorithm
//
//     for (auto& line : wallArg) {
//          key = _grid->getKeyAtPoint(line.GetPoint1());
//          iStart = (long) _grid->get_i_fromKey(key);
//          jStart = (long) _grid->get_j_fromKey(key);
//
//          key = _grid->getKeyAtPoint(line.GetPoint2());
//          iEnd = (long) _grid->get_i_fromKey(key);
//          jEnd = (long) _grid->get_j_fromKey(key);
//
//          deltaX = (int) (iEnd - iStart);
//          deltaY = (int) (jEnd - jStart);
//          deltaX1 = abs( (int) (iEnd - iStart));
//          deltaY1 = abs( (int) (jEnd - jStart));
//
//          px = 2*deltaY1 - deltaX1;
//          py = 2*deltaX1 - deltaY1;
//
//          if(deltaY1<=deltaX1) {
//               if(deltaX>=0) {
//                    iDot = iStart;
//                    jDot = jStart;
//                    xe = iEnd;
//               } else {
//                    iDot = iEnd;
//                    jDot = jEnd;
//                    xe = iStart;
//               }
//               if ((_gcode[jDot*iMax + iDot] != WALL) && (_gcode[jDot*iMax + iDot] != CLOSED_CROSSING) && (_gcode[jDot*iMax + iDot] != CLOSED_TRANSITION)) {
//                    target[jDot * iMax + iDot] = value;
//               }
//               for (i=0; iDot < xe; ++i) {
//                    ++iDot;
//                    if(px<0) {
//                         px+=2*deltaY1;
//                    } else {
//                         if((deltaX<0 && deltaY<0) || (deltaX>0 && deltaY>0)) {
//                              ++jDot;
//                         } else {
//                              --jDot;
//                         }
//                         px+=2*(deltaY1-deltaX1);
//                    }
//                    if ((_gcode[jDot*iMax + iDot] != WALL) && (_gcode[jDot*iMax + iDot] != CLOSED_CROSSING) && (_gcode[jDot*iMax + iDot] != CLOSED_TRANSITION)) {
//                         target[jDot * iMax + iDot] = value;
//                    }
//               }
//          } else {
//               if(deltaY>=0) {
//                    iDot = iStart;
//                    jDot = jStart;
//                    ye = jEnd;
//               } else {
//                    iDot = iEnd;
//                    jDot = jEnd;
//                    ye = jStart;
//               }
//               if ((_gcode[jDot*iMax + iDot] != WALL) && (_gcode[jDot*iMax + iDot] != CLOSED_CROSSING) && (_gcode[jDot*iMax + iDot] != CLOSED_TRANSITION)) {
//                    target[jDot * iMax + iDot] = value;
//               }
//               for(i=0; jDot<ye; ++i) {
//                    ++jDot;
//                    if (py<=0) {
//                         py+=2*deltaX1;
//                    } else {
//                         if((deltaX<0 && deltaY<0) || (deltaX>0 && deltaY>0)) {
//                              ++iDot;
//                         } else {
//                              --iDot;
//                         }
//                         py+=2*(deltaX1-deltaY1);
//                    }
//                    if ((_gcode[jDot*iMax + iDot] != WALL) && (_gcode[jDot*iMax + iDot] != CLOSED_CROSSING) && (_gcode[jDot*iMax + iDot] != CLOSED_TRANSITION)) {
//                         target[jDot * iMax + iDot] = value;
//                    }
//               }
//          }
//     } //loop over all walls

} //drawLinesOnGrid