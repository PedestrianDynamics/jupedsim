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
//#include "FloorfieldViaFM.h"
//#include "../../geometry/Building.h"

int FFRouter::_cnt = 0;

FFRouter::FFRouter()
{

}

FFRouter::FFRouter(int id, RoutingStrategy s, bool hasSpecificGoals, Configuration* config):Router(id,s) {
     _config = config;
     _building = nullptr;
     _hasSpecificGoals = hasSpecificGoals;
     _globalFF = nullptr;
     _targetWithinSubroom = true; //depending on exit_strat 8 => false, depending on exit_strat 9 => true;
     if (s == ROUTING_FF_QUICKEST) {
          _mode = quickest;
          _recalc_interval = _config->get_recalc_interval();
     } else if (s == ROUTING_FF_LOCAL_SHORTEST) {
          _mode = local_shortest;
          _localShortestSafedPeds.clear();
          _localShortestSafedPeds.reserve(500);
     } else if (s == ROUTING_FF_GLOBAL_SHORTEST) {
          _mode = global_shortest;
     }
}

//FFRouter::FFRouter(const Building* const building)

FFRouter::~FFRouter()
{
     if (_globalFF) {
          delete _globalFF;
     }
     //delete localffs
     std::map<int, UnivFFviaFM*>::reverse_iterator delIter;
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
          std::vector<int> goalIDs;
          goalIDs.clear();
          //get global field to manage goals (which are not in a subroom)
          _globalFF = new FloorfieldViaFM(building, 0.25, 0.25, 0.0, false, true);
          for (auto &itrGoal : building->GetAllGoals()) {
               _globalFF->createMapEntryInLineToGoalID(itrGoal.first);
               goalIDs.emplace_back(itrGoal.first);
          }
          _goalToLineUIDmap = _globalFF->getGoalToLineUIDmap();
          _goalToLineUIDmap2 = _globalFF->getGoalToLineUIDmap2();
          _goalToLineUIDmap3 = _globalFF->getGoalToLineUIDmap3();
          //_globalFF->writeGoalFF("goal.vtk", goalIDs);
     }
     //get all door UIDs
     _allDoorUIDs.clear();
     _TransByUID.clear();
     _ExitsByUID.clear();
     _CroTrByUID.clear();
     auto& allTrans = building->GetAllTransitions();
     auto& allCross = building->GetAllCrossings();
     std::vector<std::pair<int, int>> roomAndCroTrVector;
     roomAndCroTrVector.clear();
     for (auto& pair:allTrans) {
          if (pair.second->IsOpen()) {
               _allDoorUIDs.emplace_back(pair.second->GetUniqueID());
               _CroTrByUID.insert(std::make_pair(pair.second->GetUniqueID(), pair.second));
               if (pair.second->IsExit()) {
                    _ExitsByUID.insert(std::make_pair(pair.second->GetUniqueID(), pair.second));
               }
               Room* room1 = pair.second->GetRoom1();
               if (room1) roomAndCroTrVector.emplace_back(std::make_pair(room1->GetID(), pair.second->GetUniqueID()));
               Room* room2 = pair.second->GetRoom2();
               if (room2) roomAndCroTrVector.emplace_back(std::make_pair(room2->GetID(), pair.second->GetUniqueID()));
          }
     }
     for (auto& pair:allCross) {
          if (pair.second->IsOpen()) {
               _allDoorUIDs.emplace_back(pair.second->GetUniqueID());
               _CroTrByUID.insert(std::make_pair(pair.second->GetUniqueID(), pair.second));
               Room* room1 = pair.second->GetRoom1();
               if (room1) roomAndCroTrVector.emplace_back(std::make_pair(room1->GetID(), pair.second->GetUniqueID()));
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
               //pathsMatrix[i][j] = i or j ? (follow wiki:path_reconstruction, it should be j)
               _pathsMatrix.insert(std::make_pair( key , id2 ));
               //_subroomMatrix.insert(std::make_pair(key, nullptr));
          }
     }

     //prepare all room-floor-fields-objects (one room = one instance)
     _locffviafm.clear();
     //type of allRooms: const std::map<int, std::unique_ptr<Room> >&
     const std::map<int, std::shared_ptr<Room> >& allRooms = _building->GetAllRooms();


     for (unsigned int i = 0; i < allRooms.size(); ++i) {

#ifdef DEBUG
          std::cerr << "Creating Floorfield for Room: " << pair.first << std::endl;
#endif

          auto pairRoomIt = allRooms.begin();
          std::advance(pairRoomIt, i);
          UnivFFviaFM *locffptr = nullptr;
          locffptr = new UnivFFviaFM(pairRoomIt->second.get(), building, 0.0625, 0.0, false);

          locffptr->setUser(DISTANCE_MEASUREMENTS_ONLY);
          locffptr->setMode(CENTERPOINT);
          locffptr->setSpeedMode(FF_HOMO_SPEED);
          locffptr->addAllTargetsParallel();
          //locffptr->writeFF("UnivFF"+std::to_string(pairRoomIt->first)+".vtk", locffptr->getKnownDoorUIDs());
          Log->Write("INFO: \tAdding distances in Room %d to matrix", (*pairRoomIt).first);
//#pragma omp critical(_locffviafm)
          _locffviafm.insert(std::make_pair((*pairRoomIt).first, locffptr));
     }


     // nowait, because the parallel region ends directly afterwards
//#pragma omp for nowait
     //@todo: @ar.graf: it would be easier to browse thru doors of each field directly after "addAllTargetsParallel" as
     //                 we do only want doors of same subroom anyway. BUT the router would have to switch from room-scope
     //                 to subroom-scope. Nevertheless, we could omit the room info (used to acces correct field), if we
     //                 do it like in "ReInit()".
     for (unsigned int i = 0; i < roomAndCroTrVector.size(); ++i) {
          auto rctIt = roomAndCroTrVector.begin();
          std::advance(rctIt, i);

          ////loop over upper triangular matrice (i,j) and write to (j,i) as well
          for (auto otherDoor : roomAndCroTrVector) {
               if (otherDoor.first != rctIt->first) continue; // we only want doors with one room in common
               if (otherDoor.second <= rctIt->second) continue; // calculate every path only once
               // if we exclude otherDoor.second == rctIt->second, the program loops forever

               //if the door is closed, then don't calc distances
               //if (!_CroTrByUID.at(*otherDoor)->IsOpen()) {
               //     continue;
               //}

               // if the two doors are not within the same subroom, do not consider (ar.graf)
               // should fix problems of oscillation caused by doorgaps in the distancegraph
               int thisUID1 = (_CroTrByUID.at(rctIt->second)->GetSubRoom1()) ? _CroTrByUID.at(rctIt->second)->GetSubRoom1()->GetUID() : -10 ;
               int thisUID2 = (_CroTrByUID.at(rctIt->second)->GetSubRoom2()) ? _CroTrByUID.at(rctIt->second)->GetSubRoom2()->GetUID() : -20 ;
               int otherUID1 = (_CroTrByUID.at(otherDoor.second)->GetSubRoom1()) ? _CroTrByUID.at(otherDoor.second)->GetSubRoom1()->GetUID() : -30 ;
               int otherUID2 = (_CroTrByUID.at(otherDoor.second)->GetSubRoom2()) ? _CroTrByUID.at(otherDoor.second)->GetSubRoom2()->GetUID() : -40 ;

               if (
                         (thisUID1 != otherUID1) &&
                         (thisUID1 != otherUID2) &&
                         (thisUID2 != otherUID1) &&
                         (thisUID2 != otherUID2)      ) {
                    continue;
               }

               UnivFFviaFM* locffptr = _locffviafm[rctIt->first];
               double tempDistance = locffptr->getDistanceBetweenDoors(rctIt->second, otherDoor.second);

               if (tempDistance < locffptr->getGrid()->Gethx()) {
                    Log->Write("WARNING:\tIgnoring distance of doors %d and %d because it is too small: %f",rctIt->second, otherDoor.second, tempDistance);
                    //Log->Write("^^^^^^^^\tIf there are scattered subrooms, which are not connected, this is ok.");
                    continue;
               }
//
               std::pair<int, int> key_ij = std::make_pair(otherDoor.second, rctIt->second);
               std::pair<int, int> key_ji = std::make_pair(rctIt->second, otherDoor.second);


#pragma omp critical(_distMatrix)
               if (_distMatrix.at(key_ij) > tempDistance) {
                    _distMatrix.erase(key_ij);
                    _distMatrix.erase(key_ji);
                    _distMatrix.insert(std::make_pair(key_ij, tempDistance));
                    _distMatrix.insert(std::make_pair(key_ji, tempDistance));
               }
          } // otherDoor
     } // roomAndCroTrVector

     FloydWarshall();

     //debug output in file
//     _locffviafm[4]->writeFF("ffTreppe.vtk", _allDoorUIDs);

     //int roomTest = (*(_locffviafm.begin())).first;
     //int transTest = (building->GetRoom(roomTest)->GetAllTransitionsIDs())[0];
     //auto test = _CroTrByUID.at(1253);

     if (_config->get_write_VTK_files()) {
          for (unsigned int i = 0; i < _locffviafm.size(); ++i) {
               auto iter = _locffviafm.begin();
               std::advance(iter, i);
               int roomNr = iter->first;
               iter->second->writeFF("ffrouterOfRoom" + std::to_string(roomNr) + ".vtk", _allDoorUIDs);
          }
     }

//     std::ofstream matrixfile;
//     matrixfile.open("Matrix.txt");
//
//     for (auto mapItem : _distMatrix) {
//          matrixfile << mapItem.first.first << " to " << mapItem.first.second << " : " << mapItem.second << "\t via \t" << _pathsMatrix[mapItem.first];
//          matrixfile << "\t" << _CroTrByUID.at(mapItem.first.first)->GetID() << " to " << _CroTrByUID.at(mapItem.first.second)->GetID() << "\t via \t";
//          matrixfile << _CroTrByUID.at(_pathsMatrix[mapItem.first])->GetID() << std::endl;
//     }
//     matrixfile.close();
     Log->Write("INFO: \tFF Router Init done.");
     return true;
}

bool FFRouter::ReInit()
{
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
               //pathsMatrix[i][j] = i or j ? (follow wiki:path_reconstruction, it should be j)
               _pathsMatrix.insert(std::make_pair( key , id2 ));
          }
     }

     for (auto floorfield : _locffviafm) {
          floorfield.second->setSpeedMode(FF_PED_SPEED);
          //@todo: ar.graf: create a list of local ped-ptr instead of giving all peds-ptr
          floorfield.second->createPedSpeed(_building->GetAllPedestrians().data(), _building->GetAllPedestrians().size(), _mode, 1.);
          floorfield.second->recreateAllForQuickest();
          std::vector<int> allDoors(floorfield.second->getKnownDoorUIDs());
          for (auto firstDoor : allDoors) {
               for (auto secondDoor : allDoors) {
                    if (secondDoor <= firstDoor) continue; // calculate every path only once
                    // if the two doors are not within the same subroom, do not consider (ar.graf)
                    // should fix problems of oscillation caused by doorgaps in the distancegraph
                    int thisUID1 = (_CroTrByUID.at(firstDoor)->GetSubRoom1()) ? _CroTrByUID.at(firstDoor)->GetSubRoom1()->GetUID() : -10 ;
                    int thisUID2 = (_CroTrByUID.at(firstDoor)->GetSubRoom2()) ? _CroTrByUID.at(firstDoor)->GetSubRoom2()->GetUID() : -20 ;
                    int otherUID1 = (_CroTrByUID.at(secondDoor)->GetSubRoom1()) ? _CroTrByUID.at(secondDoor)->GetSubRoom1()->GetUID() : -30 ;
                    int otherUID2 = (_CroTrByUID.at(secondDoor)->GetSubRoom2()) ? _CroTrByUID.at(secondDoor)->GetSubRoom2()->GetUID() : -40 ;

                    if (
                              (thisUID1 != otherUID1) &&
                              (thisUID1 != otherUID2) &&
                              (thisUID2 != otherUID1) &&
                              (thisUID2 != otherUID2)      ) {
                         continue;
                    }

                    //double tempDistance = floorfield.second->getCostToDestination(firstDoor, _CroTrByUID.at(secondDoor)->GetCentre());
                    double tempDistance = floorfield.second->getDistanceBetweenDoors(firstDoor, secondDoor);
                    if (tempDistance < floorfield.second->getGrid()->Gethx()) {
                         Log->Write("WARNING:\tDistance of doors %d and %d is too small: %f",firstDoor, secondDoor, tempDistance);
                         //Log->Write("^^^^^^^^\tIf there are scattered subrooms, which are not connected, this is ok.");
                         continue;
                    }
                    std::pair<int, int> key_ij = std::make_pair(secondDoor, firstDoor);
                    std::pair<int, int> key_ji = std::make_pair(firstDoor, secondDoor);
                    if (_distMatrix.at(key_ij) > tempDistance) {
                         _distMatrix.erase(key_ij);
                         _distMatrix.erase(key_ji);
                         _distMatrix.insert(std::make_pair(key_ij, tempDistance));
                         _distMatrix.insert(std::make_pair(key_ji, tempDistance));
                    }
               } //secondDoor(s)
          } //firstDoor(s)
     } //allRooms
     FloydWarshall();
     _plzReInit = false;
     return true;
}



int FFRouter::FindExit(Pedestrian* p)
{
//     if (_mode == local_shortest) {
//          if ((_locffviafm.at(p->GetRoomID())->getGrid()->includesPoint(p->GetPos())) &&
//              (p->GetSubRoomUID() != _locffviafm.at(p->GetRoomID())->getSubroomUIDAt(p->GetPos()))) {
//               //pedestrian is still in the room, but changed subroom
//               _localShortestSafedPeds.emplace_back(p->GetID());
//          }
//
//          //if needed: quickest-mechanic part 2 of 2
//          if (!(_locffviafm.at(p->GetRoomID())->getGrid()->includesPoint(p->GetPos()))) {
//               //pedestrian left the room and therefore changed subroom
//               _localShortestSafedPeds.emplace_back(p->GetID());
//          }
//     }
     if (_mode == quickest) {
          if (p->GetGlobalTime() > _recalc_interval && _building->GetRoom(p->GetRoomID())->GetSubRoom(p->GetSubRoomID())->IsInSubRoom(p)
              && _locffviafm[p->GetRoomID()]->getCostToDestination(p->GetExitIndex(), p->GetPos()) > 3.0
                    && p->GetExitIndex() != -1) {
               //delay possible
               if ((int) p->GetGlobalTime() % 10 != p->GetID() % 10) {
                    return p->GetExitIndex();     //stay with old target
               }
          }
          //new version: recalc densityspeed every x seconds
          if ((p->GetGlobalTime() > _timeToRecalc) && (p->GetGlobalTime() > Pedestrian::GetMinPremovementTime() + _recalc_interval)) {
               _plzReInit = true;
          }
     }
     double minDist = DBL_MAX;
     int bestDoor = -1;

     int goalID = p->GetFinalDestination();
     std::vector<int> validFinalDoor; //UIDs of doors
     validFinalDoor.clear();
     if (goalID == -1) {
          for (auto& pairDoor : _ExitsByUID) {
               //we add the all exits,
               validFinalDoor.emplace_back(pairDoor.first); //UID
          }
     } else {  //only one specific goal, goalToLineUIDmap gets
               //populated in Init()
          if ((_goalToLineUIDmap.count(goalID) == 0) || (_goalToLineUIDmap[goalID] == -1)) {
               Log->Write("ERROR: \t ffRouter: unknown/unreachable goalID: %d in FindExit(Ped)",goalID);
          } else {
               validFinalDoor.emplace_back(_goalToLineUIDmap.at(goalID));
          }
     }

     std::vector<int> DoorUIDsOfRoom;
     DoorUIDsOfRoom.clear();
     if (_building->GetRoom(p->GetRoomID())->GetSubRoom(p->GetSubRoomID())->IsInSubRoom(p->GetPos())) {
          //ped is in the subroom, according to its member attribs
     } else {
          bool located = false;
          SubRoom* oldSubRoom = _building->GetRoom(p->GetRoomID())->GetSubRoom(p->GetSubRoomID());
          for (auto& room : _building->GetAllRooms()) {
               if (located) {break;}
               for (auto& subroom : room.second->GetAllSubRooms()) {
                    if (subroom.second->IsInSubRoom(p->GetPos()) && subroom.second->IsDirectlyConnectedWith(oldSubRoom)) {
                         //maybe room on wrong floor
                         p->SetRoomID(room.second->GetID(), room.second->GetCaption());
                         p->SetSubRoomID(subroom.second->GetSubRoomID());
                         p->SetSubRoomUID(subroom.second->GetUID());
                         located = true;
                         break;
                    }
               }
          }
          if (!located) { //ped is outside
               return -1;
          }
     }
     DoorUIDsOfRoom.clear();
     if (!_targetWithinSubroom) {
          //candidates of current room (ID) (provided by Room)
          for (auto transUID : _building->GetRoom(p->GetRoomID())->GetAllTransitionsIDs()) {
               if ((_CroTrByUID.count(transUID) != 0) && (_CroTrByUID[transUID]->IsOpen())) {
                    DoorUIDsOfRoom.emplace_back(transUID);
               }
          }
          for (auto &subIPair : _building->GetRoom(p->GetRoomID())->GetAllSubRooms()) {
               for (auto &crossI : subIPair.second->GetAllCrossings()) {
                    if (crossI->IsOpen()) {
                         DoorUIDsOfRoom.emplace_back(crossI->GetUniqueID());
                    }
               }
          }
     }
     else
     {
          //candidates of current subroom only
          for (auto &crossI : _building->GetRoom(p->GetRoomID())->GetSubRoom(p->GetSubRoomID())->GetAllCrossings()) {
               if (crossI->IsOpen()) {
                    DoorUIDsOfRoom.emplace_back(crossI->GetUniqueID());
               }
          }

          for (auto &transI : _building->GetRoom(p->GetRoomID())->GetSubRoom(p->GetSubRoomID())->GetAllTransitions()) {
               if (transI->IsOpen()) {
                    DoorUIDsOfRoom.emplace_back(transI->GetUniqueID());
               }
          }
     }

     int bestFinalDoor = -1; // to silence the compiler
     for(int finalDoor : validFinalDoor) {
          //with UIDs, we can ask for shortest path
          for (int doorUID : DoorUIDsOfRoom) {
               double locDistToDoor = _locffviafm[p->GetRoomID()]->getCostToDestination(doorUID, p->GetPos(), _mode);
               if (locDistToDoor < -J_EPS) {     //for old ff: //this can happen, if the point is not reachable and therefore has init val -7
                    continue;
               }
               std::pair<int, int> key = std::make_pair(doorUID, finalDoor);
               //auto subroomDoors = _building->GetSubRoomByUID(p->GetSubRoomUID())->GetAllGoalIDs();
               //only consider, if paths exists
               if (_pathsMatrix.count(key)==0) {
                    Log->Write("no key for %d %d", key.first, key.second);
                    continue;
               }

               if ((_distMatrix.count(key)!=0) && (_distMatrix.at(key) != DBL_MAX)) {
                    if ((_distMatrix.at(key) + locDistToDoor) < minDist) {
                         minDist = _distMatrix.at(key) + locDistToDoor;
                         bestDoor = key.first; //doorUID
                         bestFinalDoor = key.second;
                    }
               }
          }
     }

     //at this point, bestDoor is either a crossing or a transition
     if ((!_targetWithinSubroom) && (_CroTrByUID.count(bestDoor) != 0)) {
          while (!_CroTrByUID[bestDoor]->IsTransition()) {
               std::pair<int, int> key = std::make_pair(bestDoor, bestFinalDoor);
               bestDoor = _pathsMatrix[key];
          }
     }

//#pragma omp critical(finalDoors)
//     _finalDoors.emplace(std::make_pair(p->GetID(), bestFinalDoor));

     if (_CroTrByUID.count(bestDoor)) {
          p->SetExitIndex(bestDoor);
          p->SetExitLine(_CroTrByUID.at(bestDoor));
     }
     return bestDoor; //-1 if no way was found, doorUID of best, if path found
}

void FFRouter::FloydWarshall()
{
     bool change = false;
     double savedDistance = 0.;
     int totalnum = _allDoorUIDs.size();
     for(int k = 0; k<totalnum; ++k) {
          for(int i = 0; i<totalnum; ++i) {
               for(int j= 0; j<totalnum; ++j) {
                    std::pair<int, int> key_ij = std::make_pair(_allDoorUIDs[i], _allDoorUIDs[j]);
                    std::pair<int, int> key_ik = std::make_pair(_allDoorUIDs[i], _allDoorUIDs[k]);
                    std::pair<int, int> key_kj = std::make_pair(_allDoorUIDs[k], _allDoorUIDs[j]);
                    if ((_distMatrix[key_ik] < DBL_MAX) && (_distMatrix[key_kj] < DBL_MAX) &&
                       (_distMatrix[key_ik] + _distMatrix[key_kj] < _distMatrix[key_ij]))
                    {
                         savedDistance = _distMatrix[key_ij] - _distMatrix[key_ik] - _distMatrix[key_kj];
                         _distMatrix.erase(key_ij);
                         _distMatrix.insert(std::make_pair(key_ij, _distMatrix[key_ik] + _distMatrix[key_kj]));
                         _pathsMatrix.erase(key_ij);
                         _pathsMatrix.insert(std::make_pair(key_ij, _pathsMatrix[key_ik]));
                         change = true;
                    }
               }
          }
     }
     if (change) {
          //Log->Write("Floyd nochmal!!! %f", savedDistance);
          FloydWarshall();
     } else {
          Log->Write("INFO:\t FloydWarshall done!");
     }
}

void FFRouter::SetMode(std::string s)
{
     if (s == "global_shortest"){
          _mode = global_shortest;
          return;
     }

     if (s == "quickest") {
          _mode = quickest;
          return;
     }

     _mode = global_shortest;
     return;
}

bool FFRouter::MustReInit() {
     return _plzReInit;
}

void FFRouter::SetRecalc(double t) {
     _timeToRecalc = t + _recalc_interval;
}