/**
 * \file        ffRouter.h
 * \date        Feb 19, 2016
 * \version     v0.8
 * \copyright   <2016-2022> Forschungszentrum Jülich GmbH. All rights reserved.
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
 * This router is an update of the former Router.{cpp, h} - Global-, Quickest
 * Router System. In the __former__ version, a graph was created with doors and
 * hlines as nodes and the distances of (doors, hlines), connected with a line-
 * of-sight, was used as edge-costs. If there was no line-of-sight, there was no
 * connecting edge. On the resulting graph, the Floyd-Warshall algorithm was
 * used to find any paths. In the "quickest-___" variants, the edge cost was not
 * determined by the distance, but by the distance multiplied by a speed-
 * estimate, to find the path with minimum travel times. This whole construct
 * worked pretty well, but dependend on hlines to create paths with line-of-
 * sights to the next target (hline/door).
 *
 * In the ffRouter, we want to overcome hlines by using floor fields to
 * determine the distances. A line of sight is not required any more. We hope to
 * reduce the graph complexity and the preparation-needs for new geometries.
 *
 * To find a counterpart for the "quickest-____" router, we can either use
 * __special__ floor fields, that respect the travel time in the input-speed map,
 * or take the distance-floor field and multiply it by a speed-estimate (analog
 * to the former construct.
 *
 * We will derive from the <Router> class to fit the interface.
 *
 **/

#include <cfloat>
#include <algorithm>
#include "ffRouter.h"
#include "../geometry/Building.h"
#include "../pedestrian/Pedestrian.h"
#include "../IO/OutputHandler.h"

FFRouter::FFRouter()
{

}

FFRouter::FFRouter(int id, RoutingStrategy s, bool hasSpecificGoals):Router(id,s) {
     _building = nullptr;
     _hasSpecificGoals = hasSpecificGoals;
     _globalFF = nullptr;
}

//FFRouter::FFRouter(const Building* const building)

FFRouter::~FFRouter()
{
     if (_globalFF) {
          delete _globalFF;
     }
     //delete localffs
     std::map<int, LocalFloorfieldViaFM*>::reverse_iterator delIter;
     for (delIter = _locffviafm.rbegin();
          delIter != _locffviafm.rend();
          ++delIter) {
          delete (*delIter).second;
     }
}

bool FFRouter::Init(Building* building)
{
     _building = building;
     if (_hasSpecificGoals) {
          //get global field to manage goals (which are not in a subroom)
          _globalFF = new FloorfieldViaFM(building, 0.25, 0.25, 0.0, false, "nofile");
          for (auto &itrGoal : building->GetAllGoals()) {
               _globalFF->createLineToGoalID(itrGoal.first);
          }
          goalToLineUIDmap = _globalFF->getGoalToLineUIDmap();
          goalToLineUIDmap2 = _globalFF->getGoalToLineUIDmap2();
          goalToLineUIDmap3 = _globalFF->getGoalToLineUIDmap3();
     }
     //get all door UIDs
     _allDoorUIDs.clear();
     _TransByUID.clear();
     _ExitsByUID.clear();
     _CroTrByUID.clear();
     auto& allTrans = building->GetAllTransitions();
     auto& allCross = building->GetAllCrossings();
     for (auto& pair:allTrans) {
          if (pair.second->IsOpen()) {
               _allDoorUIDs.emplace_back(pair.second->GetUniqueID());
               _CroTrByUID.insert(std::make_pair(pair.second->GetUniqueID(), (Crossing *) pair.second));
               if (pair.second->IsExit()) {
                    _ExitsByUID.insert(std::make_pair(pair.second->GetUniqueID(), pair.second));
               }
          }
     }
     for (auto& pair:allCross) {
          if (pair.second->IsOpen()) {
               _allDoorUIDs.emplace_back(pair.second->GetUniqueID());
               _CroTrByUID.insert(std::make_pair(pair.second->GetUniqueID(), pair.second));
          }
     }
     //make unique
     _allDoorUIDs.erase( std::unique(_allDoorUIDs.begin(),_allDoorUIDs.end()), _allDoorUIDs.end());

     //cleanse maps
     _distMatrix.clear();
     _pathsMatrix.clear();

     //init, yet no distances, only create map entries
     for(auto& id1 : _allDoorUIDs) {
          for(auto& id2 : _allDoorUIDs){
               std::pair<int, int> key   = std::make_pair(id1, id2);
               double              value = (id1 == id2)? 0.0 : DBL_MAX;
               //distMatrix[i][j] = 0,   if i==j
               //distMatrix[i][j] = max, else
               _distMatrix.insert(std::make_pair( key , value));
               //pathsMatrix[i][j] = i
               _pathsMatrix.insert(std::make_pair( key , id1 ));
          }
     }

     //prepare all room-floor-fields-objects (one room = one instance)
     _locffviafm.clear();
     LocalFloorfieldViaFM* ptrToNew = nullptr;
     double tempDistance = 0.;
     //type of allRooms: const std::map<int, std::unique_ptr<Room> >&
     const std::map<int, std::shared_ptr<Room> >& allRooms = _building->GetAllRooms();
     for(auto& pairRoom : allRooms) {
#ifdef DEBUG
          std::cerr << "Creating Floorfield for Room: " << pair.first << std::endl;
#endif
          ptrToNew = new LocalFloorfieldViaFM(pairRoom.second.get(), building, 0.0625, 0.0625, 0.0, false, "nofile");
          //for (long int i = 0; i < ptrToNew)
          Log->Write("Created room-scale floorfield for Room %d", pairRoom.first);
          _locffviafm.insert( std::make_pair( pairRoom.first, ptrToNew ) );

          //SetDistances
          vector<int> doorUIDs;
          doorUIDs.clear();
          for (int transI: pairRoom.second->GetAllTransitionsIDs()) {
               if (_CroTrByUID[transI]->IsOpen()) {
                    doorUIDs.emplace_back(transI);
                    //Log->Write("Door UID: %d", transI);
                    //Log->Write(_CroTrByUID[transI]->GetDescription());
               }
          }

          for (auto& subI : pairRoom.second->GetAllSubRooms()) {
               for (auto& crossI : subI.second->GetAllCrossings()) { //if clause checks so that only new doors get added
                    if ((crossI->IsOpen()) && (std::find(doorUIDs.begin(), doorUIDs.end(), crossI->GetUniqueID()) == doorUIDs.end())) {
                         doorUIDs.emplace_back(crossI->GetUniqueID());
                         //Log->Write("Crossing: %d", crossI->GetUniqueID());
                         //Log->Write(crossI->GetDescription());
                    }
               }
          }
          //loop over upper triangular matrice (i,j) and write to (j,i) as well
          std::vector<int>::const_iterator outerPtr;
          std::vector<int>::const_iterator innerPtr;
          Log->Write("Found %d Doors (Cross + Trans)", doorUIDs.size());
          for (outerPtr = doorUIDs.begin(); outerPtr != doorUIDs.end(); ++outerPtr) {
               //if the door is closed, then dont calc distances
               if (!_CroTrByUID.at(*outerPtr)->IsOpen()) {
                    continue;
               }
               for (innerPtr = outerPtr; innerPtr != doorUIDs.end(); ++innerPtr) {
                    //if outerdoor == innerdoor or the inner door is closed
                    if (  (*outerPtr == *innerPtr) || (!_CroTrByUID.at(*innerPtr)->IsOpen())  ) {
                         continue;
                    }
                    //The distance is checked by reading the timecost of a wave starting at the line(!) to reach a point(!)
                    //That will have the following implications:
                    //distance (a to b) can be different than distance (b ta a)
                    //     for this reason, we calc only (a to b) and set (b to a) to the same value
                    //distance (line to center) can be larger than (line to endpoint). to get closer to the min-distance
                    //we take the minimum of three shots: center, and a point close to each endpoint
                    //
                    //note: we can not assume: (a to c) = (a to b) + (b to c) for the reasons above.
                    //question: if (a to c) > (a to b) + (b to c), then FloyedWarshall will favour intermediate goal b
                    //          as a precessor to c. This might be very important, if there are edges among lines, that
                    //          are not adjacent.
                    tempDistance = ptrToNew->getCostToDestination(*outerPtr, _CroTrByUID.at(*innerPtr)->GetCentre());
                    Point endA = _CroTrByUID.at(*innerPtr)->GetCentre() * .9 + _CroTrByUID.at(*innerPtr)->GetPoint1() * .1;
                    Point endB = _CroTrByUID.at(*innerPtr)->GetCentre() * .9 + _CroTrByUID.at(*innerPtr)->GetPoint2() * .1;
                    if (ptrToNew->getCostToDestination(*outerPtr, endA) < tempDistance) {
                         tempDistance = ptrToNew->getCostToDestination(*outerPtr, endA);
                    }
                    if (ptrToNew->getCostToDestination(*outerPtr, endB) < tempDistance) {
                         tempDistance = ptrToNew->getCostToDestination(*outerPtr, endB);
                    }
                    if (tempDistance < 0) {
                         Crossing* crossTest = _CroTrByUID.at(*innerPtr);
                         Point a = crossTest->GetPoint1();
                         Point b = crossTest->GetPoint2();
                         Log->Write(a.toString());
                         Log->Write(b.toString());
                    }
//                    tempDistance = ptrToNew->getCostToDestination(*outerPtr, _CroTrByUID[*innerPtr]->GetCentre());
                    std::pair<int, int> key_ij = std::make_pair(*outerPtr, *innerPtr);
                    std::pair<int, int> key_ji = std::make_pair(*innerPtr, *outerPtr);
                    _distMatrix.erase(key_ij);
                    _distMatrix.erase(key_ji);
                    _distMatrix.insert(std::make_pair(key_ij, tempDistance));
                    _distMatrix.insert(std::make_pair(key_ji, tempDistance));
               }
          }
     }
     FloydWarshall();
//     Log->Write("1-3: %f \t 1-2: %f \t 2-3: %f",
//                _distMatrix.at(std::make_pair(400, 382)),
//                _distMatrix.at(std::make_pair(400, 381)),
//                _distMatrix.at(std::make_pair(381, 382)));
//     Log->Write("1-3: %f \t 1-2: %f \t 2-3: %f",
//                _distMatrix.at(std::make_pair(382, 400)),
//                _distMatrix.at(std::make_pair(382, 381)),
//                _distMatrix.at(std::make_pair(381, 400)));
     Log->Write("INFO: \tFF Router Init done.");
     return true;
}

int FFRouter::FindExit(Pedestrian* p)
{
     double minDist = DBL_MAX;
     int bestDoor = -1;

     int goalID = p->GetFinalDestination();
     std::vector<int> validFinalDoor; //UIDs of doors
     validFinalDoor.clear();
     if (goalID == -1) {
          for (auto& pairDoor : _ExitsByUID) {
               //we add the closest Doors of every goal, goalToLineUIDmap gets
               //populated in Init()
               validFinalDoor.emplace_back(pairDoor.first); //UID
          }
     } else {  //only one specific goal
          if (goalToLineUIDmap.count(goalID) == 0) {
               Log->Write("ERROR: \t ffRouter: unknown goalID: %d in FindExit(Ped)",goalID);
          } else {
               validFinalDoor.emplace_back(goalToLineUIDmap.at(goalID));
          }
     }

     //candidates of current room (ID) (provided by Room)
     std::vector<int> DoorUIDsOfRoom;
     DoorUIDsOfRoom = _building->GetRoom(p->GetRoomID())->GetAllTransitionsIDs();
     for(auto& subIPair : _building->GetRoom(p->GetRoomID())->GetAllSubRooms()) {
          for (auto& crossI : subIPair.second->GetAllCrossings()) {
               DoorUIDsOfRoom.emplace_back(crossI->GetUniqueID());
          }
     }

     for(int finalDoor : validFinalDoor) {
          //with UIDs, we can ask for shortest path
          for (int doorUID : DoorUIDsOfRoom) {
               double locDistToDoor = _locffviafm[p->GetRoomID()]->getCostToDestination(doorUID, p->GetPos());
               std::pair<int, int> key = std::make_pair(doorUID, finalDoor);
               if ((_distMatrix.count(key)!=0) && (_distMatrix.at(key) != DBL_MAX)
                     && ( (_distMatrix.at(key) + locDistToDoor) < minDist)) {
                    minDist = _distMatrix.at(key) + locDistToDoor;
                    bestDoor = key.first;
               }
          }
     }
     if (_CroTrByUID.count(bestDoor)) {
          p->SetExitIndex(bestDoor);
          p->SetExitLine(_CroTrByUID.at(bestDoor));
     }
     return bestDoor; //-1 if no way was found, doorUID of best, if path found
}

void FFRouter::Reset()
{
     for(auto& pair : _distMatrix) {
          //distMatrix[i][j] = 0,   if i==j
          //distMatrix[i][j] = max, else
          pair.second = (pair.second == 0.0) ? 0.0 : DBL_MAX;
     }
     for(auto& pair : _pathsMatrix) {
          //pathsMatrix[i][j] = i
          pair.second = pair.first.first;
     }
}

void FFRouter::FloydWarshall()
{
//     for (int k = 0; k < n; k++)
//          for (int i = 0; i < n; i++)
//               for (int j = 0; j < n; j++)
//                    if (_distMatrix[i][k] + _distMatrix[k][j] < _distMatrix[i][j]) {
//                         _distMatrix[i][j] = _distMatrix[i][k] + _distMatrix[k][j];
//                         _pathsMatrix[i][j] = _pathsMatrix[k][j];
//                    }

     int totalnum = _building->GetAllTransitions().size();
     for(int k = 0; k<totalnum; ++k) {
          for(int i = 0; i<totalnum; ++i) {
               for(int j= 0; j<totalnum; ++j) {
                    std::pair<int, int> key_ij = std::make_pair(i, j);
                    std::pair<int, int> key_ik = std::make_pair(i, k);
                    std::pair<int, int> key_kj = std::make_pair(k, j);
                    if(_distMatrix[key_ik] + _distMatrix[key_kj] < _distMatrix[key_ij]) {
                         _distMatrix.erase(key_ij);
                         _distMatrix.insert(std::make_pair(key_ij, _distMatrix[key_ik] + _distMatrix[key_kj]));
                         _pathsMatrix.erase(key_ij);
                         _pathsMatrix.insert(std::make_pair(key_ij, _pathsMatrix[key_kj]));
                    }
               }
          }
     }
}

void FFRouter::SetMode(std::string s)
{
     if (s == "global_shortest"){
          _mode = global_shortest;
          return;
     }

     _mode = global_shortest;
     return;
}
