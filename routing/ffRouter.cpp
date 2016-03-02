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

#include <float.h>
#include "ffRouter.h"
#include "../geometry/Building.h"
#include "../pedestrian/Pedestrian.h"
#include "../IO/OutputHandler.h"

FFRouter::FFRouter()
{

}

FFRouter::FFRouter(int id, RoutingStrategy s):Router(id,s) {
     _building = nullptr;
}

FFRouter::FFRouter(const Building* const building)
{
     //FFRouter();
     Init(building);
     //FloydWarshall();
}

FFRouter::~FFRouter()
{
     delete _globalFF;
     //delete localffs todo

}

bool FFRouter::Init(const Building* const building)
{
     _building = building;
     //get global field to manage goals (which are not in a subroom)
     _globalFF = new FloorfieldViaFM(building, 0.125, 0.125, 0.0, false, "nofile");
     for (auto& itrGoal : building->GetAllGoals()) {
          _globalFF->getDirectionToGoalID(itrGoal.first);
     }
     goalToLineUIDmap  = _globalFF->getGoalToLineUIDmap();
     goalToLineUIDmap2 = _globalFF->getGoalToLineUIDmap2();
     goalToLineUIDmap3 = _globalFF->getGoalToLineUIDmap3();
     //get all door UIDs
     _allDoorUIDs.clear();
     auto allTrans = building->GetAllTransitions();
     for(auto& pair : allTrans) {
          _allDoorUIDs.emplace_back(pair.first);
          for(auto& pair2 : allTrans){
               std::pair<int, int> key   = std::make_pair(pair.first, pair2.first);
               double              value = (pair.first == pair2.first)? 0.0 : DBL_MAX;
               //distMatrix[i][j] = 0,   if i==j
               //distMatrix[i][j] = max, else
               _distMatrix.insert(std::make_pair( key , value));
               //pathsMatrix[i][j] = i
               _pathsMatrix.insert(std::make_pair( key , pair.first ));
          }
     }

     //prepare all room-floor-fields-objects (one room = one instance)
     _locffviafm.clear();
     LocalFloorfieldViaFM* ptrToNew = nullptr;
     double tempDistance = 0.;
     //type of allRooms: const std::map<int, std::unique_ptr<Room> >&
     const std::map<int, std::unique_ptr<Room> >& allRooms = _building->GetAllRooms();
     for(auto& pair : allRooms) {
#ifdef DEBUG
          std::cerr << "Creating Floorfield for Room: " << pair.first << std::endl;
#endif
          ptrToNew = new LocalFloorfieldViaFM(pair.second.get(), building, 0.125, 0.125, 0.0, false, "nofile");
          _locffviafm.insert( std::make_pair( pair.first, ptrToNew ) );

          //SetDistances
          const vector<int>& doorUIDs = pair.second->GetAllTransitionsIDs();
          //loop over upper triangular matrice (i,j) and write to (j,i) as well
          std::vector<int>::const_iterator outerPtr;
          std::vector<int>::const_iterator innerPtr;
          for (outerPtr = doorUIDs.begin(); outerPtr != doorUIDs.end(); ++outerPtr) {
               //if the door is closed, then dont calc distances
               if (!building->GetAllTransitions().at(*outerPtr)->IsOpen()) {
                    continue;
               }
               for (innerPtr = outerPtr; innerPtr != doorUIDs.end(); ++innerPtr) {
                    //if outerdoor == innerdoor or the inner door is closed
                    if (  (*outerPtr == *innerPtr) || (!building->GetAllTransitions().at(*innerPtr)->IsOpen())  ) {
                         continue;
                    }
                    tempDistance = _locffviafm[pair.first]->getCostToDestination(*outerPtr, building->GetAllTransitions().at(*innerPtr)->GetCentre());
                    std::pair<int, int> key_ij = std::make_pair(*outerPtr, *innerPtr);
                    std::pair<int, int> key_ji = std::make_pair(*innerPtr, *outerPtr);
                    _distMatrix[key_ij] = tempDistance;
                    _distMatrix[key_ji] = tempDistance;
               }
          }
     }
     FloydWarshall();
     Log->Write("INFO: \tFF Router Init done.");
     return true;
}

int FFRouter::FindExit(Pedestrian* p)
{
     //this function sets
//     int nav_id= navLine->GetUniqueID();
//     ped->SetExitIndex(nav_id);
//     ped->SetExitLine(navLine);
//     return nav_id;
     int goalID = p->GetFinalDestination();

     return 0;
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
                         _distMatrix[key_ij] = _distMatrix[key_ik] + _distMatrix[key_kj];
                         _pathsMatrix[key_ij] = _pathsMatrix[key_kj];
                    }
               }
          }
     }
}
