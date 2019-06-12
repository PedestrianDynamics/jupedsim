//
// Created by Tobias Schrödter on 2019-06-11.
//

#include "WaitingProbability.h"

#include "../../precomputation/floorfield/mesh/RectGrid.h"

#include "../../../geometry/Room.h"
#include "../../../geometry/SubRoom.h"
#include "../../../pedestrian/Pedestrian.h"
#include "../../../geometry/Point.h"

void  WaitingProbability::Init(Building* building){
     Log->Write("Start init Waiting Probability");
     _building = building;

     for (auto roomItr : _building->GetAllRooms()){
          for (auto subRoomItr : roomItr.second->GetAllSubRooms()){
//               _staticMap[subRoomItr.second->GetUID()] = boost::numeric::ublas::matrix<double>(0,0);
//               _distanceMap[subRoomItr.second->GetUID()] = boost::numeric::ublas::matrix<double>(0,0);
//               _flowMap[subRoomItr.second->GetUID()] = boost::numeric::ublas::matrix<double>(0,0);
//               _angleMap[subRoomItr.second->GetUID()] = boost::numeric::ublas::matrix<double>(0,0);
//               _boundaryMap[subRoomItr.second->GetUID()] = boost::numeric::ublas::matrix<double>(0,0);
               _staticMap[subRoomItr.second->GetUID()] = std::vector<double>;
               _distanceMap[subRoomItr.second->GetUID()] = std::vector<double>;
               _flowMap[subRoomItr.second->GetUID()] = std::vector<double>;
               _angleMap[subRoomItr.second->GetUID()] = std::vector<double>;
               _boundaryMap[subRoomItr.second->GetUID()] = std::vector<double>;

               vector<double> bb = subRoomItr.second->GetBoundingBox();
               RectGrid grid;
               double xMin = bb[0];
               double xMax = bb[1];
               double yMin = bb[2];
               double yMax = bb[3];

               grid.setBoundaries(xMin, yMin, xMax, yMax);
               grid.setSpacing(0.3, 0.3); //FIXME make not hard coded
               grid.createGrid();

               _gridMap[subRoomItr.second->GetUID()]= grid;
               computeStatic(grid, subRoomItr.second);
          }
     }
     Log->Write("End init Waiting Probability-----------------");

}

Point WaitingProbability::GetWaitingPosition(Room* room, Pedestrian* ped) const{
     SubRoom* subRoom = room->GetSubRoom(ped->GetSubRoomID());
     return subRoom->GetCentroid();
}

void WaitingProbability::computeStatic(RectGrid grid, std::shared_ptr<SubRoom>& subroom){

     computeDistanceCost(grid, subroom);
     writeVTK(grid, _distanceMap.at(subroom->GetUID()), "distanceCost_" + std::to_string(subroom->GetUID()) + ".vtk");
}

void WaitingProbability::computeFlowAvoidance(RectGrid grid, std::shared_ptr<SubRoom>& subroom){

}

void WaitingProbability::computeBoundaryPreference(RectGrid grid, std::shared_ptr<SubRoom>& subroom){

}
void WaitingProbability::computeDistanceCost(RectGrid grid, std::shared_ptr<SubRoom>& subroom){
     _distanceMap.at(subroom->GetUID()).resize(grid.GetnPoints());

     for (auto i : grid.GetnPoints()){
          _distanceMap.at(subroom->GetUID())[i] = i;
     }

}
void WaitingProbability::computeAngleCost(RectGrid grid, std::shared_ptr<SubRoom>& subroom){

}

void WaitingProbability::writeVTK(RectGrid grid, boost::numeric::ublas::matrix<double> values, std::string filename){
     Log->Write("INFO: \tWrite Floorfield to file");
     Log->Write(filename);
     std::ofstream file;

     file.open(_building->GetConfig()->GetProjectRootDir()+filename);

     file << "# vtk DataFile Version 3.0" << std::endl;
     file << "Testdata: Fast Marching: Test: " << std::endl;
     file << "ASCII" << std::endl;
     file << "DATASET STRUCTURED_POINTS" << std::endl;
     file << "DIMENSIONS " <<  std::to_string(grid.GetiMax()) << " " <<
          std::to_string(grid.GetjMax()) << " 1" << std::endl;
     file << "ORIGIN " << grid.GetxMin() << " " << grid.GetyMin() << " 0" << std::endl;
     file << "SPACING " << std::to_string(grid.Gethx()) << " " << std::to_string(grid.Gethy()) << " 1" << std::endl;
     file << "POINT_DATA " << std::to_string(grid.GetnPoints()) << std::endl;
     file << "SCALARS GCode float 1" << std::endl;
     file << "LOOKUP_TABLE default" << std::endl;

     for (long int i = 0; i < grid.GetnPoints(); ++i) {
          grid.
          file << values[i] << std::endl;
     }

     file.close();


}
