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
#include <random>

void  WaitingProbability::Init(Building* building){
     Log->Write("Start init Waiting Probability");
     _building = building;

     // Init random generator
     _rdGenerator=std::mt19937(building->GetConfig()->GetSeed());
     _rdDistribution = std::uniform_real_distribution<double> (0.,1.);

     parseBuilding();
     computeStatic();

     Log->Write("End init Waiting Probability-----------------");
}

void WaitingProbability::parseBuilding(){
     Log->Write("Start parseBuilding");

     double stepSizeX = 0.1;
     double stepSizeY = 0.1;

     std::vector<Line> _wall;
     std::vector<Line> _exitsFromScope;


     for (const auto roomItr : _building->GetAllRooms()){
          for (const auto subRoomItr : roomItr.second->GetAllSubRooms()){
               std::shared_ptr<SubRoom> subroom = subRoomItr.second;

               for (auto trans : subroom->GetAllTransitions()) {
                    if (trans->IsExit() && !trans->IsClose()){
                         _exitsFromScope.emplace_back(Line ( (Line) *(trans)));
                    } else if (!trans->IsOpen()){
                         _wall.emplace_back(Line ( (Line) *(trans)));
                    }
               }

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

               _gridMap[uid]->setBoundaries(xMin, yMin, xMax, yMax);
               _gridMap[uid]->setSpacing(stepSizeX, stepSizeY);
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

     //TODO random generator as class member
     std::random_device rd;
     std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
     std::uniform_real_distribution<> dis(0.0, 1.0);

     double random = dis(gen);

     double sum = 0.;

     int uid = subRoom->GetUID();

     double x=0., y=0.;

     for (int i = 0; i<_gridMap.at(uid)->GetnPoints(); ++i) {
          sum += _staticMap.at(uid).at(i);
          x = _gridMap.at(uid)->get_x_fromKey(i);
          y = _gridMap.at(uid)->get_y_fromKey(i);

          if (sum >= random){
               x = _gridMap.at(uid)->get_x_fromKey(i);
               y = _gridMap.at(uid)->get_y_fromKey(i);

               if (subRoom->IsInSubRoom(Point(x,y))){
                    break;
               }
          }

          if (i==_gridMap.at(uid)->GetnPoints()-1){
               x = subRoom->GetCentroid()._x;
               y = subRoom->GetCentroid()._y;
          }
     }

     Point ret(x,y);
     return ret;
}

void WaitingProbability::computeStatic(){
     Log->Write("Start compute static");

     for (auto roomItr : _building->GetAllRooms()){
          for (auto subRoomItr : roomItr.second->GetAllSubRooms()) {
               std::shared_ptr<SubRoom> subroom = subRoomItr.second;

               computeDistanceCost(subroom);
               computeAngleCost(subroom);
               computeFlowAvoidance(subroom);
               computeBoundaryPreference(subroom);

               computeStatic(subroom);

               writeVTK(subroom, "prob_" + std::to_string(subroom->GetUID()) + ".vtk");
          }
     }
}

void WaitingProbability::computeStatic(const std::shared_ptr<SubRoom> subroom)
{
     int uid = subroom->GetUID();

     for (int i = 0; i<_gridMap.at(uid)->GetnPoints(); ++i) {
          double value =
                    1.*_flowMap.at(uid).at(i)+
                    1.*_boundaryMap.at(uid).at(i)+
                    1.*_distanceMap.at(uid).at(i)+
                    1.*_angleMap.at(uid).at(i);

          _staticMap.at(uid).at(i) = value;
     }

     normalize(_staticMap.at(uid));
}

void WaitingProbability::computeFlowAvoidance(const std::shared_ptr<SubRoom> subroom){
     Log->Write("Start compute flow avoidance");

     double x,y, theta;
     int uid = subroom->GetUID();
     double f  = 1.5;
     double f1 = 0;
     double f2 = 0;
     Point xAxis(1., 0.);
     double flow = 0.;

     for (auto trans : subroom->GetAllTransitions()) {
          flow = 0.;

          Point centre = trans->GetCentre();
          double x0 = centre._x;
          double y0 = centre._y;

          f1 = trans->GetLength();
          f2 = 5.*f1;
          theta = Angle(xAxis, trans->GetPoint1()-trans->GetPoint2());

          float a = std::pow(cos(theta), 2.)/f1 + std::pow(sin(theta), 2.)/f2;
          float b = -1. * sin(2.*theta)/(2*f1) + sin(2.*theta)/(2.*f2);
          float c = std::pow(sin(theta), 2.)/f1 + std::pow(cos(theta), 2.)/f2;

          for (int i = 0; i<_gridMap.at(uid)->GetnPoints(); ++i) {
               flow = 0.;
               x = _gridMap.at(uid)->get_x_fromKey(i);
               y = _gridMap.at(uid)->get_y_fromKey(i);
               Point p(x, y);

               if(subroom->IsInSubRoom(p)){
                    flow = -1.*f*exp(-a * std::pow(x-x0, 2.) -2.*b*(x-x0)*(y-y0) -c*std::pow(y-y0, 2.));
               }

               _flowMap.at(uid).at(i) = std::min(_flowMap.at(uid).at(i), flow);
          }
     }
     postProcess(_flowMap.at(uid), subroom);

}

void WaitingProbability::computeBoundaryPreference(const std::shared_ptr<SubRoom> subroom){
     Log->Write("Start compute boundary preference");

     double x0, y0, x1, y1, sum;

     int uid = subroom->GetUID();

     double gridsize = _gridMap.at(uid)->Gethx()*_gridMap.at(uid)->Gethy();
     double b = 3.;

     for (int i = 0; i<_gridMap.at(uid)->GetnPoints(); ++i) {
          sum = 0;

          x0 = _gridMap.at(uid)->get_x_fromKey(i);
          y0 = _gridMap.at(uid)->get_y_fromKey(i);
          Point p0(x0, y0);

          if (subroom->IsInSubRoom(p0)) {
               for (int j = 0; j<_gridMap.at(uid)->GetnPoints(); ++j) {
                    x1 = _gridMap.at(uid)->get_x_fromKey(j);
                    y1 = _gridMap.at(uid)->get_y_fromKey(j);
                    Point p1(x1, y1);

                    if (subroom->IsInSubRoom(p1)) {
                         double distance = (p0-p1).Norm();
                         double bp = -1. * b*std::pow(1.+distance, -2.)*gridsize;
                         sum += bp;
                    }
               }
          }

          _boundaryMap.at(uid).at(i) = sum/5.;
     }
     postProcess(_boundaryMap.at(uid), subroom);
}

void WaitingProbability::computeDistanceCost(const std::shared_ptr<SubRoom> subroom){
     Log->Write("Start compute distance cost");

     double x,y, minDist;
     double d = 0.4;

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
               _distanceMap.at(uid).at(i) = -1. * d * minDist;
          }
     }
     postProcess(_distanceMap.at(uid), subroom);
}

void WaitingProbability::computeAngleCost(const std::shared_ptr<SubRoom> subroom){
     Log->Write("Start compute angle cost");

     double x,y, minAngle;

     double a = 0.05;
     double alpha = 3.;

     int uid = subroom->GetUID();

     for (int i=0; i< _gridMap.at(uid)->GetnPoints(); ++i) {
          minAngle = DBL_MAX;

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

                    Point xAxis(1., 0.);
                    double theta = Angle(xAxis, trans->GetPoint1()-trans->GetPoint2());

                    double tmpAngle = std::abs(checkAngles(angle1, angle2) - theta);
                    minAngle = std::min(minAngle, tmpAngle);
               }
               _angleMap.at(uid).at(i) = -1. * a * std::pow(minAngle, alpha);
          }
     }

     postProcess(_angleMap.at(uid), subroom);
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

void WaitingProbability::postProcess(std::vector<double>& data, const std::shared_ptr<SubRoom> subroom){
     double max = 0;
     for(auto it = data.begin(); it != data.end(); ++it){
          max = std::max(max, std::abs(*it));
     }

     for(auto it = data.begin(); it != data.end(); ++it){
          *it += max;
     }

     int uid = subroom->GetUID();
     double x, y;

     for (int i=0; i< _gridMap.at(uid)->GetnPoints(); ++i) {
          x = _gridMap.at(uid)->get_x_fromKey(i);
          y = _gridMap.at(uid)->get_y_fromKey(i);
          Point p(x, y);
          long int key = _gridMap.at(uid)->getKeyAtPoint(p);
          if (!subroom->IsInSubRoom(p)) {
               data.at(key) = 0;
          }
     }
}

void WaitingProbability::normalize(std::vector<double>& data){
     double sum = 0.;

     for(auto it = data.begin(); it != data.end(); ++it){
          sum += *it;
     }

     for(auto it = data.begin(); it != data.end(); ++it){
          *it /= sum;
     }
}

void WaitingProbability::writeVTK(const std::shared_ptr<SubRoom> subroom, const std::string filename){

     Log->Write("INFO: \tWrite Floorfield to file");
     Log->Write(filename);
     std::ofstream file;

     RectGrid* grid = _gridMap[subroom->GetUID()];
     int uid = subroom->GetUID();
     file.open(_building->GetConfig()->GetProjectRootDir()+filename);

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