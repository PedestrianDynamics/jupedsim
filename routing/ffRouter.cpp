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
//#include "../geometry/Building.h"
//#include "../pedestrian/Pedestrian.h"
//#include "../IO/OutputHandler.h"

FFRouter::FFRouter()
{

}

FFRouter::FFRouter(int id, RoutingStrategy s, bool hasSpecificGoals):Router(id,s) {
     _building = nullptr;
     _hasSpecificGoals = hasSpecificGoals;
     _globalFF = nullptr;
     _targetWithinSubroom = true; //depending on exit_strat 8 => false, depending on exit_strat 9 => true;
     if (s == ROUTING_FF_QUICKEST) {
          _mode = quickest;
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
          _globalFF = new FloorfieldViaFM(building, 0.25, 0.25, 0.0, false, true);
          for (auto &itrGoal : building->GetAllGoals()) {
               _globalFF->createMapEntryInLineToGoalID(itrGoal.first);
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
     std::vector<std::pair<int, int>> subroomAndCroTrVector;
     subroomAndCroTrVector.clear();
     for (auto& pair:allTrans) {
          if (pair.second->IsOpen()) {
               _allDoorUIDs.emplace_back(pair.second->GetUniqueID());
               _CroTrByUID.insert(std::make_pair(pair.second->GetUniqueID(), (Crossing *) pair.second));
               if (pair.second->IsExit()) {
                    _ExitsByUID.insert(std::make_pair(pair.second->GetUniqueID(), pair.second));
               }
               // @todo f.mack: Is it always the case that Subroom1 exists (i.e. is inside)?
               subroomAndCroTrVector.emplace_back(std::make_pair(pair.second->GetSubRoom1()->GetUID(), pair.second->GetUniqueID()));
               //subroomAndCroTrVector.emplace_back(std::make_pair(pair.second->GetSubRoom2()->GetUID(), pair.second->GetUniqueID()));
          }
     }
     for (auto& pair:allCross) {
          if (pair.second->IsOpen()) {
               _allDoorUIDs.emplace_back(pair.second->GetUniqueID());
               _CroTrByUID.insert(std::make_pair(pair.second->GetUniqueID(), pair.second));
               subroomAndCroTrVector.emplace_back(std::make_pair(pair.second->GetSubRoom1()->GetUID(), pair.second->GetUniqueID()));
               subroomAndCroTrVector.emplace_back(std::make_pair(pair.second->GetSubRoom2()->GetUID(), pair.second->GetUniqueID()));
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
          }
     }

     //prepare all room-floor-fields-objects (one room = one instance)
     _locffviafm.clear();
     _sublocffviafm.clear();
     //type of allRooms: const std::map<int, std::unique_ptr<Room> >&
     const std::map<int, std::shared_ptr<Room> >& allRooms = _building->GetAllRooms();
     std::map<int, std::shared_ptr<SubRoom>> allSubroomsByUID;
     allSubroomsByUID.clear();
     for (auto room: allRooms) {
          auto subrooms = room.second.get()->GetAllSubRooms();
          for (auto subroom: subrooms) {
               allSubroomsByUID.insert(std::make_pair(subroom.second.get()->GetUID(), subroom.second));
          }
     }

     // @todo f.mack Is it worth parallelizing this?
#pragma omp parallel for
     //for (auto &pairRoom : allRooms) {
     for (unsigned int i = 0; i < allSubroomsByUID.size(); ++i) {

#ifdef DEBUG
          std::cerr << "Creating Floorfield for subroom: " << pair.first << std::endl;
#endif
          auto pairSubroomIt = allSubroomsByUID.begin();
          std::advance(pairSubroomIt, i);
          SubLocalFloorfieldViaFM *ptrToNew = nullptr;
          ptrToNew = new SubLocalFloorfieldViaFM((*pairSubroomIt).second.get(), building, 0.125, 0.125, 0.0, false);

          Log->Write("####### initializing room %d", pairSubroomIt->first);
          //for (long int i = 0; i < ptrToNew)
          //Log->Write("INFO: \tAdding distances in Room %d to matrix", (*pairSubroomIt).first);
#pragma omp critical
          _sublocffviafm.insert(std::make_pair((*pairSubroomIt).first, ptrToNew));
         //Log->Write("####### added to _locffviafm");
     }
     Log->Write("####### initializing rooms done");
     Log->Write("Contents of _sublocffviavm");
     for (auto i:_sublocffviafm) {
          Log->Write("%d\t%p", i.first, i.second);
     }
     Log->Write("Contents of subroomAndCroTrVector");
     for (auto i: subroomAndCroTrVector) {
          Log->Write("room: %d\t door: %d", i.first, i.second);
     }

#pragma omp parallel for
     for (unsigned int i = 0; i < subroomAndCroTrVector.size(); ++i) {
          //Log->Write("##1");
          auto srctIt = subroomAndCroTrVector.begin();
          //Log->Write("##2");
          std::advance(srctIt, i);
          Log->Write("##3");
          //SetDistances
          //vector<int> doorUIDs;
          //doorUIDs.clear();
          //Room* room = _building->GetSubRoomByUID(srctIt->first);
          //for (int transI: (*pairRoomIt).second->GetAllTransitionsIDs()) {
          //for (int transI: room->GetAllTransitionsIDs())
          //     if ( (_CroTrByUID.count(transI) != 0) && (_CroTrByUID[transI]->IsOpen()) ) {
          //          doorUIDs.emplace_back(transI);
          //          //Log->Write("Door UID: %d", transI);
          //          //Log->Write(_CroTrByUID[transI]->GetDescription());
          //     }
          //}

          //for (auto &subI : (*pairRoomIt).second->GetAllSubRooms()) {
          //     for (auto &crossI : subI.second->GetAllCrossings()) { //if clause checks so that only new doors get added
          //          if ((crossI->IsOpen()) &&
          //              (std::find(doorUIDs.begin(), doorUIDs.end(), crossI->GetUniqueID()) == doorUIDs.end())) {
          //               doorUIDs.emplace_back(crossI->GetUniqueID());
          //               //Log->Write("Crossing: %d", crossI->GetUniqueID());
          //               //Log->Write(crossI->GetDescription());
          //          }
          //     }
          //}

          Log->Write("#######INFO: \tCalculating floorfield in subroom %d for door %d", srctIt->first, srctIt->second);

          ////loop over upper triangular matrice (i,j) and write to (j,i) as well
          //std::vector<int>::const_iterator outerPtr;
          //std::vector<int>::const_iterator innerPtr;
          //Log->Write("INFO: \tFound %d Doors (Cross + Trans) in room %d", doorUIDs.size(), (*pairRoomIt).first);
          //for (outerPtr = doorUIDs.begin(); outerPtr != doorUIDs.end(); ++outerPtr) {
          for (auto outerPtr: _CroTrByUID) {
               //if the door is closed, then dont calc distances
               //if (!_CroTrByUID.at(*outerPtr)->IsOpen()) {
               //     continue;
               //}
          //     // @todo: ar.graf: this following loop and the one directly wrapping this "for (outerPtr = ...)" could be
          //     // moved out of the parallel for loop into a follow up part. There we could parallelize the most inner loop
          //     // to achieve a better load balancing. You can have a look at DirectionStrategy.cpp at the DirectionLocalFloorfield::Init
          //     // and take that scheme.
          //     for (innerPtr = outerPtr; innerPtr != doorUIDs.end(); ++innerPtr) {
          //          //if outerdoor == innerdoor or the inner door is closed
          //          if ((*outerPtr == *innerPtr) || (!_CroTrByUID.at(*innerPtr)->IsOpen())) {
          //               continue;
          //          }

                    //if the two doors are not within the same subroom, do not consider (ar.graf)
                    //should fix problems of oscillation caused by doorgaps in the distancegraph
               //     int innerUID1 = (_CroTrByUID.at(srctIt->second)->GetSubRoom1()) ? _CroTrByUID.at(srctIt->second)->GetSubRoom1()->GetUID() : -1 ;
               //     int innerUID2 = (_CroTrByUID.at(srctIt->second)->GetSubRoom2()) ? _CroTrByUID.at(srctIt->second)->GetSubRoom2()->GetUID() : -2 ;
               //     int outerUID1 = (_CroTrByUID.at(*outerPtr)->GetSubRoom1()) ? _CroTrByUID.at(*outerPtr)->GetSubRoom1()->GetUID() : -3 ;
               //     int outerUID2 = (_CroTrByUID.at(*outerPtr)->GetSubRoom2()) ? _CroTrByUID.at(*outerPtr)->GetSubRoom2()->GetUID() : -4 ;

               // @todo f.mack: check this
               //int innerUID1 = _building->GetSubRoomByUID(srctIt->second) ? _CroTrByUID.at(srctIt->second)->GetSubRoom1()->GetUID() : -1 ;
               //int innerUID2 = (_CroTrByUID.at(srctIt->second)->GetSubRoom2()) ? _CroTrByUID.at(srctIt->second)->GetSubRoom2()->GetUID() : -2 ;
               int outerUID1 = (outerPtr.second->GetSubRoom1()) ? outerPtr.second->GetSubRoom1()->GetUID() : -3 ;
               int outerUID2 = (outerPtr.second->GetSubRoom2()) ? outerPtr.second->GetSubRoom2()->GetUID() : -4 ;
               int innerUID1 = srctIt->first;
               int innerUID2 = srctIt->first;

                    if (
                         (innerUID1 != outerUID1) &&
                         (innerUID1 != outerUID2) &&
                         (innerUID2 != outerUID1) &&
                         (innerUID2 != outerUID2)      ) {
                         printf("####### Ignoring way from %d to %d", srctIt->second, outerPtr.first);
                         continue;
                    }
                    double tempDistance = 0.;

                    //The distance is checked by reading the timecost of a wave starting at the line(!) to reach a point(!)
                    //That will have the following implications:
                    //distance (a to b) can be different than distance (b ta a)
                    //     for this reason, we calc only (a to b) and set (b to a) to the same value
                    //distance (line to center) can be larger than (line to endpoint). to get closer to the min-distance
                    //we did take the minimum of three shots: center, and a point close to each endpoint BUT not anymore
                    //
                    //note: we can not assume: (a to c) = (a to b) + (b to c) for the reasons above.
                    //question: if (a to c) > (a to b) + (b to c), then FloyedWarshall will favour intermediate goal b
                    //          as a precessor to c. This might be very important, if there are edges among lines, that
                    //          are not adjacent.
          auto ptrToNew = _sublocffviafm[srctIt->first];
               Log->Write("for subroom %d is ptrToNew %p", srctIt->first, ptrToNew);

                    //tempDistance = ptrToNew->getCostToDestination(*outerPtr,
                    //                                              _CroTrByUID.at(*innerPtr)->GetCentre());
          tempDistance = ptrToNew->getCostToDestination(srctIt->second, outerPtr.second->GetCentre());
//                    Point endA = _CroTrByUID.at(*innerPtr)->GetCentre() * .9 +
//                                 _CroTrByUID.at(*innerPtr)->GetPoint1() * .1;
//                    Point endB = _CroTrByUID.at(*innerPtr)->GetCentre() * .9 +
//                                 _CroTrByUID.at(*innerPtr)->GetPoint2() * .1;
//                    if (ptrToNew->getCostToDestination(*outerPtr, endA) < tempDistance) {
//                         tempDistance = ptrToNew->getCostToDestination(*outerPtr, endA);
//                    }
//                    if (ptrToNew->getCostToDestination(*outerPtr, endB) < tempDistance) {
//                         tempDistance = ptrToNew->getCostToDestination(*outerPtr, endB);
//                    }
//                    if (tempDistance < 0) {
//                         Crossing *crossTest = _CroTrByUID.at(*innerPtr);
//                         Point a = crossTest->GetPoint1();
//                         Point b = crossTest->GetPoint2();
//                         Log->Write("tempDistance < 0 with crossing: (below)");
//                         Log->Write(a.toString());
//                         Log->Write(b.toString());
//                    }
                    if (tempDistance < ptrToNew->getGrid()->Gethx()) {
                         //Log->Write("WARNING:\tDistance of doors %d and %d is too small: %f",*outerPtr, *innerPtr, tempDistance);
                         //Log->Write("^^^^^^^^\tIf there are scattered subrooms, which are not connected, this is ok.");
                         continue;
                    }
//                    tempDistance = ptrToNew->getCostToDestination(*outerPtr, _CroTrByUID[*innerPtr]->GetCentre());
                    std::pair<int, int> key_ij = std::make_pair(outerPtr.first, srctIt->second);
                    std::pair<int, int> key_ji = std::make_pair(srctIt->second, outerPtr.first);
                    _distMatrix.erase(key_ij);
                    _distMatrix.erase(key_ji);
                    _distMatrix.insert(std::make_pair(key_ij, tempDistance));
                    _distMatrix.insert(std::make_pair(key_ji, tempDistance));
               //}
          }
     }
     FloydWarshall();

     //debug output in file
//     _locffviafm[4]->writeFF("ffTreppe.vtk", _allDoorUIDs);

     //int roomTest = (*(_locffviafm.begin())).first;
     //int transTest = (building->GetRoom(roomTest)->GetAllTransitionsIDs())[0];
//     for (unsigned int i = 0; i < _locffviafm.size(); ++i) {
//          auto iter = _locffviafm.begin();
//          std::advance(iter, i);
//          int roomNr = iter->first;
//          iter->second->writeFF("testFF" + std::to_string(roomNr) + ".vtk", _allDoorUIDs);
//     }
//
     std::ofstream matrixfile;
     matrixfile.open("Matrix.txt");

     for (auto mapItem : _distMatrix) {
          matrixfile << mapItem.first.first << " to " << mapItem.first.second << " : " << mapItem.second << "\t via \t" << _pathsMatrix[mapItem.first];
          matrixfile << "\t" << _CroTrByUID.at(mapItem.first.first)->GetID() << " to " << _CroTrByUID.at(mapItem.first.second)->GetID() << "\t via \t";
          matrixfile << _CroTrByUID.at(_pathsMatrix[mapItem.first])->GetID() << std::endl;
     }
     matrixfile.close();
     Log->Write("INFO: \tFF Router Init done.");
     return true;
}

bool FFRouter::ReInit()
{
     if (_hasSpecificGoals) {
          //get global field to manage goals (which are not in a subroom)
          if (_globalFF) delete _globalFF;
          _globalFF = new FloorfieldViaFM(_building, 0.25, 0.25, 0.0, false, true);
          for (auto &itrGoal : _building->GetAllGoals()) {
               _globalFF->createMapEntryInLineToGoalID(itrGoal.first);
          }
          goalToLineUIDmap = _globalFF->getGoalToLineUIDmap(); //@todo: ar.graf: will this create mem-leak?
          goalToLineUIDmap2 = _globalFF->getGoalToLineUIDmap2();
          goalToLineUIDmap3 = _globalFF->getGoalToLineUIDmap3();
     }
     //get all door UIDs
     _allDoorUIDs.clear();
     _TransByUID.clear();
     _ExitsByUID.clear();
     _CroTrByUID.clear();
     auto& allTrans = _building->GetAllTransitions();
     auto& allCross = _building->GetAllCrossings();
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

     std::map< std::pair<int, int> , double > tmpdistMatrix;
     std::map< std::pair<int, int> , int >    tmppathsMatrix;

     tmpdistMatrix.clear();
     tmppathsMatrix.clear();
     tmpdistMatrix = std::move(_distMatrix);
     tmppathsMatrix = std::move(_pathsMatrix);

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

     for (auto ptr : _locffviafm) {
          delete ptr.second;
     }
     //prepare all room-floor-fields-objects (one room = one instance)
     _locffviafm.clear();
     //type of allRooms: const std::map<int, std::unique_ptr<Room> >&
     const std::map<int, std::shared_ptr<Room> >& allRooms = _building->GetAllRooms();
#pragma omp parallel for
     //for (auto &pairRoom : allRooms) {
     for (unsigned int i = 0; i < allRooms.size(); ++i) {

#ifdef DEBUG
          std::cerr << "Creating Floorfield for Room: " << pair.first << std::endl;
#endif
          auto pairRoomIt = allRooms.begin();
          std::advance(pairRoomIt, i);
          LocalFloorfieldViaFM* ptrToNew = nullptr;
          double tempDistance = 0.;
          ptrToNew = new LocalFloorfieldViaFM((*pairRoomIt).second.get(), _building, 0.125, 0.125, 0.0, false);
          //for (long int i = 0; i < ptrToNew)
          Log->Write("INFO: \tAdding distances in Room %d to matrix", (*pairRoomIt).first);
#pragma omp critical
          _locffviafm.insert(std::make_pair((*pairRoomIt).first, ptrToNew));

          //SetDistances
          vector<int> doorUIDs;
          doorUIDs.clear();
          for (int transI: (*pairRoomIt).second->GetAllTransitionsIDs()) {
               if ( (_CroTrByUID.count(transI) != 0) && (_CroTrByUID[transI]->IsOpen()) ) {
                    doorUIDs.emplace_back(transI);
                    //Log->Write("Door UID: %d", transI);
                    //Log->Write(_CroTrByUID[transI]->GetDescription());
               }
          }

          for (auto &subI : (*pairRoomIt).second->GetAllSubRooms()) {
               for (auto &crossI : subI.second->GetAllCrossings()) { //if clause checks so that only new doors get added
                    if ((crossI->IsOpen()) &&
                        (std::find(doorUIDs.begin(), doorUIDs.end(), crossI->GetUniqueID()) == doorUIDs.end())) {
                         doorUIDs.emplace_back(crossI->GetUniqueID());
                         //Log->Write("Crossing: %d", crossI->GetUniqueID());
                         //Log->Write(crossI->GetDescription());
                    }
               }
          }
          //loop over upper triangular matrice (i,j) and write to (j,i) as well
          std::vector<int>::const_iterator outerPtr;
          std::vector<int>::const_iterator innerPtr;
          Log->Write("INFO: \tFound %d Doors (Cross + Trans) in room %d", doorUIDs.size(), (*pairRoomIt).first);
          for (outerPtr = doorUIDs.begin(); outerPtr != doorUIDs.end(); ++outerPtr) {
               //if the door is closed, then dont calc distances
               if (!_CroTrByUID.at(*outerPtr)->IsOpen()) {
                    continue;
               }
               // @todo: ar.graf: this following loop and the one directly wrapping this "for (outerPtr = ...)" could be
               // moved out of the parallel for loop into a follow up part. There we could parallelize the most inner loop
               // to achieve a better load balancing. You can have a look at DirectionStrategy.cpp at the DirectionLocalFloorfield::Init
               // and take that scheme.
               for (innerPtr = outerPtr; innerPtr != doorUIDs.end(); ++innerPtr) {
                    //if outerdoor == innerdoor or the inner door is closed
                    if ((*outerPtr == *innerPtr) || (!_CroTrByUID.at(*innerPtr)->IsOpen())) {
                         continue;
                    }

                    //if the two doors are not within the same subroom, do not consider (ar.graf)
                    //should fix problems of oscillation caused by doorgaps in the distancegraph
                    int innerUID1 = (_CroTrByUID.at(*innerPtr)->GetSubRoom1()) ? _CroTrByUID.at(*innerPtr)->GetSubRoom1()->GetUID() : -1 ;
                    int innerUID2 = (_CroTrByUID.at(*innerPtr)->GetSubRoom2()) ? _CroTrByUID.at(*innerPtr)->GetSubRoom2()->GetUID() : -2 ;
                    int outerUID1 = (_CroTrByUID.at(*outerPtr)->GetSubRoom1()) ? _CroTrByUID.at(*outerPtr)->GetSubRoom1()->GetUID() : -3 ;
                    int outerUID2 = (_CroTrByUID.at(*outerPtr)->GetSubRoom2()) ? _CroTrByUID.at(*outerPtr)->GetSubRoom2()->GetUID() : -4 ;

                    if (
                              (innerUID1 != outerUID1) &&
                              (innerUID1 != outerUID2) &&
                              (innerUID2 != outerUID1) &&
                              (innerUID2 != outerUID2)      ) {
                         continue;
                    }

                    //The distance is checked by reading the timecost of a wave starting at the line(!) to reach a point(!)
                    //That will have the following implications:
                    //distance (a to b) can be different than distance (b ta a)
                    //     for this reason, we calc only (a to b) and set (b to a) to the same value
                    //distance (line to center) can be larger than (line to endpoint). to get closer to the min-distance
                    //we did take the minimum of three shots: center, and a point close to each endpoint BUT not anymore
                    //
                    //note: we can not assume: (a to c) = (a to b) + (b to c) for the reasons above.
                    //question: if (a to c) > (a to b) + (b to c), then FloyedWarshall will favour intermediate goal b
                    //          as a precessor to c. This might be very important, if there are edges among lines, that
                    //          are not adjacent.
                    std::pair<int, int> key_ij = std::make_pair(*outerPtr, *innerPtr);
                    std::pair<int, int> key_ji = std::make_pair(*innerPtr, *outerPtr);
                    if (tmpdistMatrix.count(key_ij) > 0) {
                         tempDistance = tmpdistMatrix.at(key_ij);
                    } else {
                         tempDistance = ptrToNew->getCostToDestination(*outerPtr,
                                                                       _CroTrByUID.at(*innerPtr)->GetCentre());
                    }
//                    Point endA = _CroTrByUID.at(*innerPtr)->GetCentre() * .9 +
//                                 _CroTrByUID.at(*innerPtr)->GetPoint1() * .1;
//                    Point endB = _CroTrByUID.at(*innerPtr)->GetCentre() * .9 +
//                                 _CroTrByUID.at(*innerPtr)->GetPoint2() * .1;
//                    if (ptrToNew->getCostToDestination(*outerPtr, endA) < tempDistance) {
//                         tempDistance = ptrToNew->getCostToDestination(*outerPtr, endA);
//                    }
//                    if (ptrToNew->getCostToDestination(*outerPtr, endB) < tempDistance) {
//                         tempDistance = ptrToNew->getCostToDestination(*outerPtr, endB);
//                    }
//                    if (tempDistance < 0) {
//                         Crossing *crossTest = _CroTrByUID.at(*innerPtr);
//                         Point a = crossTest->GetPoint1();
//                         Point b = crossTest->GetPoint2();
//                         Log->Write("tempDistance < 0 with crossing: (below)");
//                         Log->Write(a.toString());
//                         Log->Write(b.toString());
//                    }
                    if (tempDistance < ptrToNew->getGrid()->Gethx()) {
                         //Log->Write("WARNING:\tDistance of doors %d and %d is too small: %f",*outerPtr, *innerPtr, tempDistance);
                         //Log->Write("^^^^^^^^\tIf there are scattered subrooms, which are not connected, this is ok.");
                         continue;
                    }
//                    tempDistance = ptrToNew->getCostToDestination(*outerPtr, _CroTrByUID[*innerPtr]->GetCentre());
                    _distMatrix.erase(key_ij);
                    _distMatrix.erase(key_ji);
                    _distMatrix.insert(std::make_pair(key_ij, tempDistance));
                    _distMatrix.insert(std::make_pair(key_ji, tempDistance));
               }
          }
     }
     FloydWarshall();

     //debug output in file
//     _locffviafm[4]->writeFF("ffTreppe.vtk", _allDoorUIDs);

     //int roomTest = (*(_locffviafm.begin())).first;
     //int transTest = (building->GetRoom(roomTest)->GetAllTransitionsIDs())[0];
//     for (unsigned int i = 0; i < _locffviafm.size(); ++i) {
//          auto iter = _locffviafm.begin();
//          std::advance(iter, i);
//          int roomNr = iter->first;
//          iter->second->writeFF("testFF" + std::to_string(roomNr) + ".vtk", _allDoorUIDs);
//     }
//
     std::ofstream matrixfile;
     matrixfile.open("Matrix.txt");

     for (auto mapItem : _distMatrix) {
          matrixfile << mapItem.first.first << " to " << mapItem.first.second << " : " << mapItem.second << "\t via \t" << _pathsMatrix[mapItem.first];
          matrixfile << "\t" << _CroTrByUID.at(mapItem.first.first)->GetID() << " to " << _CroTrByUID.at(mapItem.first.second)->GetID() << "\t via \t";
          matrixfile << _CroTrByUID.at(_pathsMatrix[mapItem.first])->GetID() << std::endl;
     }
     matrixfile.close();
     Log->Write("INFO: \tFF Router Init done.");
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
          p->_ticksInThisRoom += 1;
          if (p->GetReroutingTime() > 0.) {
               p->UpdateReroutingTime();
               return p->GetExitIndex();
          } else {
               p->RerouteIn(5.);
          }
     }
     double minDist = DBL_MAX;
     int bestDoor = -1;
     //int bestGoal = -1;

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
          if ((goalToLineUIDmap.count(goalID) == 0) || (goalToLineUIDmap[goalID] == -1)) {
               Log->Write("ERROR: \t ffRouter: unknown/unreachable goalID: %d in FindExit(Ped)",goalID);
          } else {
               validFinalDoor.emplace_back(goalToLineUIDmap.at(goalID));
          }
     }

     std::vector<int> DoorUIDsOfSubRoom;
     DoorUIDsOfSubRoom.clear();
     if (_building->GetRoom(p->GetRoomID())->GetSubRoom(p->GetSubRoomID())->IsInSubRoom(p->GetPos())) {
          //ped is in the subroom, according to its member attribs
     } else {
          //Log->Write("ERROR: \tffRouter cannot handle incorrect room/subroom attribs of pedestrian %d!!", p->GetID());
     }
     if (!_targetWithinSubroom) {
          //candidates of current room (ID) (provided by Room)
          for (auto trans : _building->GetSubRoomByUID(p->GetSubRoomUID())->GetAllTransitions()) {
               if (trans->IsOpen()) {
                    DoorUIDsOfSubRoom.emplace_back(trans->GetUniqueID());
               }
          }
          for (auto cross : _building->GetSubRoomByUID(p->GetSubRoomUID())->GetAllCrossings()) {
                    if (cross->IsOpen()) {
                         DoorUIDsOfSubRoom.emplace_back(cross->GetUniqueID());
                    }
          }
     }
     else
     {
          //candidates of current subroom only
          for (auto &crossI : _building->GetRoom(p->GetRoomID())->GetSubRoom(p->GetSubRoomID())->GetAllCrossings()) {
               if (crossI->IsOpen()) {
                    DoorUIDsOfSubRoom.emplace_back(crossI->GetUniqueID());
               }
          }

          for (auto &transI : _building->GetRoom(p->GetRoomID())->GetSubRoom(p->GetSubRoomID())->GetAllTransitions()) {
               if (transI->IsOpen()) {
                    DoorUIDsOfSubRoom.emplace_back(transI->GetUniqueID());
               }
          }
     }

     for(int finalDoor : validFinalDoor) {
          //with UIDs, we can ask for shortest path
          for (int doorUID : DoorUIDsOfSubRoom) {
               double sublocDistToDoor = _sublocffviafm[p->GetSubRoomUID()]->getCostToDestination(doorUID, p->GetPos());
               if (sublocDistToDoor < -J_EPS) {     //this can happen, if the point is not reachable and therefore has init val -7
                    continue;
               }
               std::pair<int, int> key = std::make_pair(doorUID, finalDoor);
               auto subroomDoors = _building->GetSubRoomByUID(p->GetSubRoomUID())->GetAllGoalIDs();
               //only consider doors that lead to goal via a new subroom
               if (std::find(subroomDoors.begin(), subroomDoors.end(), _pathsMatrix.at(key)) != subroomDoors.end() &&
                   (finalDoor != doorUID)){
                    continue;
               }
               if (_mode == quickest) {
                    int locDistToDoorAdd = (_CroTrByUID[doorUID]->_lastTickTime2 > _CroTrByUID[doorUID]->_lastTickTime1)?_CroTrByUID[doorUID]->_lastTickTime2:_CroTrByUID[doorUID]->_lastTickTime1;
                    sublocDistToDoor = (sublocDistToDoor + locDistToDoorAdd * p->Getdt() * p->GetEllipse().GetV0())/2;
               }
               if ((_distMatrix.count(key)!=0) && (_distMatrix.at(key) != DBL_MAX)) {
                    if ( (_mode == local_shortest) &&
                         (std::find(_localShortestSafedPeds.begin(), _localShortestSafedPeds.end(), p->GetID()) == _localShortestSafedPeds.end()) ) {
                         sublocDistToDoor -= _distMatrix.at(key); // -x +x == +0, therefore only locDist is considered
                    }
                    if ((_distMatrix.at(key) + sublocDistToDoor) < minDist) {
                         minDist = _distMatrix.at(key) + sublocDistToDoor;
                         bestDoor = key.first; //doorUID
                         //bestGoal = key.second;//finalDoor
                    }
               }
          }
     }

     //at this point, bestDoor is either a crossing or a transition
//     if ((!_targetWithinSubroom) && (_CroTrByUID.count(bestDoor) != 0)) {
//          while (!_CroTrByUID[bestDoor]->IsTransition()) {
//               std::pair<int, int> key = std::make_pair(bestDoor, bestGoal);
//               bestDoor = _pathsMatrix[key];
//          }
//     }

     //avoid entering oscillation at doors alongside (real) shortest path
//     while (
//               (std::find(DoorUIDsOfSubRoom.begin(), DoorUIDsOfSubRoom.end(), _pathsMatrix[std::make_pair(bestDoor, bestGoal)]) != DoorUIDsOfSubRoom.end())
//            && (bestDoor != _pathsMatrix[std::make_pair(bestDoor, bestGoal)])        //last door has itself as _pathsMatrix[lastDooronPath]
//            && (bestDoor != bestGoal)
//            )
//     {
//          bestDoor = _pathsMatrix[std::make_pair(bestDoor, bestGoal)];
//     }
     if (_CroTrByUID.count(bestDoor)) {
          p->SetExitIndex(bestDoor);
          p->SetExitLine(_CroTrByUID.at(bestDoor));
     }
     //debug
//     if ((p->GetID() == 15) && ((p->GetSubRoomID() == 24) || (p->GetSubRoomID() == 26))) {
//          Log->Write("\nBest Door UID %d alias %d to final door %d", bestDoor, _CroTrByUID.at(bestDoor)->GetID(), _CroTrByUID.at(bestGoal)->GetID());
//          Log->Write("\n(Room) %d\t(Sub) %d \tRouting goes to:", p->GetRoomID(), p->GetSubRoomID());
//          int temp = bestDoor;
//          while (temp != bestGoal) {
//               Log->Write("\t%d", _CroTrByUID.at(temp)->GetID());
//               std::pair<int, int> key = std::make_pair(temp, bestGoal);
//               temp = _pathsMatrix[key];
//          }
//     }
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
     int totalnum = _allDoorUIDs.size();
     for(int k = 0; k<totalnum; ++k) {
          for(int i = 0; i<totalnum; ++i) {
               for(int j= 0; j<totalnum; ++j) {
                    std::pair<int, int> key_ij = std::make_pair(_allDoorUIDs[i], _allDoorUIDs[j]);
                    std::pair<int, int> key_ik = std::make_pair(_allDoorUIDs[i], _allDoorUIDs[k]);
                    std::pair<int, int> key_kj = std::make_pair(_allDoorUIDs[k], _allDoorUIDs[j]);
                    if(_distMatrix[key_ik] + _distMatrix[key_kj] < _distMatrix[key_ij]) {
                         _distMatrix.erase(key_ij);
                         _distMatrix.insert(std::make_pair(key_ij, _distMatrix[key_ik] + _distMatrix[key_kj]));
                         _pathsMatrix.erase(key_ij);
                         _pathsMatrix.insert(std::make_pair(key_ij, _pathsMatrix[key_ik]));
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

     if (s == "quickest") {
          _mode = quickest;
          return;
     }

     _mode = global_shortest;
     return;
}

void FFRouter::notifyDoor(Pedestrian *const p) {
     if (p->GetV().Norm() > 0.5) {
          return;
     }
     //find correct door
     auto lastSubRoom = _building->GetSubRoomByUID(p->GetSubRoomUID());
     auto doorsOfSubRoom = lastSubRoom->GetAllGoalIDs();
     double minDist = _CroTrByUID.at(doorsOfSubRoom[0])->DistTo(p->GetPos());
     double tmp = minDist;
     Crossing* minCross = _CroTrByUID.at(doorsOfSubRoom[0]);
     for(auto UID : doorsOfSubRoom) {
          tmp = _CroTrByUID.at(UID)->DistTo(p->GetPos());
          if (tmp < minDist) {
               minCross = _CroTrByUID.at(UID);
               minDist = tmp;
          }
     }

     //find correct direction, where direction means: subRoom1 uses TickTime1, subRoom2 uses TickTime2; order in the Crossing::HLine is defining
     if (
            (minCross->_lastTickTime2 == 0)
         && (minCross->GetSubRoom1()) && (minCross->GetSubRoom1()->IsInSubRoom(p->GetPos()))) { //p is in subRoom1, so he entered that from subRoom2
          minCross->_lastTickTime2 = p->_ticksInThisRoom;
          minCross->_refresh2 = 0;
     }
     if (
             (minCross->_lastTickTime1 == 0)
          && (minCross->GetSubRoom2()) && (minCross->GetSubRoom2()->IsInSubRoom(p->GetPos()))) {
          minCross->_lastTickTime1 = p->_ticksInThisRoom;
          minCross->_refresh1 = 0;
     }
     p->_ticksInThisRoom = 0;
}

void FFRouter::save(Pedestrian* const p) {
     _localShortestSafedPeds.emplace_back(p->GetID());
}
