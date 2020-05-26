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
#include "ffRouter.h"

#include "direction/walking/DirectionStrategy.h"
#include "geometry/SubRoom.h"
#include "geometry/WaitingArea.h"
#include "pedestrian/Pedestrian.h"
#include "routing/ff_router/mesh/RectGrid.h"

#include <stdexcept>
#include <libshared>

FFRouter::FFRouter(int id, RoutingStrategy s, bool hasSpecificGoals, Configuration * config) :
    Router(id, s)
{
    _config           = config;
    _building         = nullptr;
    _hasSpecificGoals = hasSpecificGoals;

    //depending on exit_strat 8 => false, depending on exit_strat 9 => true;
    _targetWithinSubroom = (_config->get_exit_strat() == 9);

    // get extra values depending on routing strategy s
    if(_strategy != ROUTING_FF_GLOBAL_SHORTEST && _strategy != ROUTING_FF_QUICKEST) {
        throw std::runtime_error("Wrong routing strategy for this router.");
    }

    if(_strategy == ROUTING_FF_QUICKEST) {
        _recalc_interval = _config->get_recalc_interval();
    }
}

FFRouter::~FFRouter()
{
    //delete localffs
    std::map<int, UnivFFviaFM *>::reverse_iterator delIter;
    for(delIter = _locffviafm.rbegin(); delIter != _locffviafm.rend(); ++delIter) {
        delete(*delIter).second;
    }
}

bool FFRouter::Init(Building * building)
{
    _building = building;

    CalculateFloorFields();

    if(_config->get_write_VTK_files()) {
        for(auto const & [roomID, localFF] : _locffviafm) {
            localFF->WriteFF(
                fmt::format(
                    FMT_STRING("ffrouterRoom_{:d}_t_{:.2f}.vtk"),
                    roomID,
                    Pedestrian::GetGlobalTime()),
                _allDoorUIDs);
        }
    }

    LOG_INFO("FF Router init done.");
    return true;
}

bool FFRouter::ReInit()
{
    CalculateFloorFields();
    _plzReInit = false;
    return true;
}

void FFRouter::CalculateFloorFields()
{
    //cleanse maps
    _distMatrix.clear();
    _pathsMatrix.clear();

    // clear all maps
    _allDoorUIDs.clear();
    _ExitsByUID.clear();
    _CroTrByUID.clear();

    //get all door UIDs
    std::vector<std::pair<int, int>> roomAndCroTrVector;

    for(const auto & [_, trans] : _building->GetAllTransitions()) {
        _allDoorUIDs.emplace_back(trans->GetUniqueID());
        _CroTrByUID.insert(std::make_pair(trans->GetUniqueID(), trans));
        if(trans->IsExit()) {
            _ExitsByUID.insert(std::make_pair(trans->GetUniqueID(), trans));
        }
        Room * room1 = trans->GetRoom1();
        if(room1) {
            roomAndCroTrVector.emplace_back(std::make_pair(room1->GetID(), trans->GetUniqueID()));
        }
        Room * room2 = trans->GetRoom2();
        if(room2) {
            roomAndCroTrVector.emplace_back(std::make_pair(room2->GetID(), trans->GetUniqueID()));
        }
    }

    for(const auto & [_, cross] : _building->GetAllCrossings()) {
        _allDoorUIDs.emplace_back(cross->GetUniqueID());
        _CroTrByUID.insert(std::make_pair(cross->GetUniqueID(), cross));
        Room * room1 = cross->GetRoom1();
        if(room1)
            roomAndCroTrVector.emplace_back(std::make_pair(room1->GetID(), cross->GetUniqueID()));
    }

    if(_hasSpecificGoals) {
        for(auto const & [goalID, goal] : _building->GetAllGoals()) {
            // TODO add handling for waiting areas
            // TODO add handling for doors with (almost) same distance
            //  ========      =========      =========
            //
            //       ------------------------------
            //       |           goal             |
            //       ------------------------------
            double minDist = std::numeric_limits<double>::max();
            int minID      = -1;

            for(auto const & [exitID, exit] : _ExitsByUID) {
                double dist = goal->GetDistance(exit->GetCentre());
                if(dist < minDist) {
                    minDist = dist;
                    minID   = exitID;
                }
            }
            _goalToLineUIDmap[goalID] = minID;
        }
    }

    //make unique
    std::sort(_allDoorUIDs.begin(), _allDoorUIDs.end());
    _allDoorUIDs.erase(std::unique(_allDoorUIDs.begin(), _allDoorUIDs.end()), _allDoorUIDs.end());

    //init, yet no distances, only Create map entries
    for(auto & id1 : _allDoorUIDs) {
        for(auto & id2 : _allDoorUIDs) {
            std::pair<int, int> key = std::make_pair(id1, id2);
            //distMatrix[i][j] = 0,   if i==j
            //distMatrix[i][j] = max, else
            double value = (id1 == id2) ? 0.0 : std::numeric_limits<double>::infinity();
            _distMatrix.insert(std::make_pair(key, value));

            //pathsMatrix[i][j] = i or j ? (follow wiki:path_reconstruction, it should be j)
            _pathsMatrix.insert(std::make_pair(key, id2));
        }
    }

    //prepare all room-floor-fields-objects (one room = one instance)
    _locffviafm.clear();
    for(const auto & [id, room] : _building->GetAllRooms()) {
        UnivFFviaFM * locffptr = new UnivFFviaFM(room.get(), _building, 0.125, 0.0, false);

        locffptr->SetUser(DISTANCE_MEASUREMENTS_ONLY);
        locffptr->SetMode(CENTERPOINT);
        locffptr->SetSpeedMode(FF_HOMO_SPEED);
        locffptr->AddAllTargetsParallel();
        LOG_INFO("Adding distances in Room {:d} to matrix.", id);
        _locffviafm.insert(std::make_pair(id, locffptr));
    }

    // nowait, because the parallel region ends directly afterwards
    //#pragma omp for nowait
    //@todo: @ar.graf: it would be easier to browse thru doors of each field directly after "AddAllTargetsParallel" as
    //                 we do only want doors of same subroom anyway. BUT the router would have to switch from room-scope
    //                 to subroom-scope. Nevertheless, we could omit the room info (used to access correct field), if we
    //                 do it like in "ReInit()".
    for(const auto & [roomID1, doorUID1] : roomAndCroTrVector) {
        // loop over upper triangular matrix (i,j) and write to (j,i) as well
        for(const auto & [roomID2, doorUID2] : roomAndCroTrVector) {
            if(roomID2 != roomID1)
                continue; // we only want doors with one room in common
            if(doorUID2 <= doorUID1)
                continue; // calculate every path only once
            // if we exclude otherDoor.second == rctIt->second, the program loops forever

            // if the two doors are not within the same subroom, do not consider (ar.graf)
            // should fix problems of oscillation caused by doorgaps in the distancegraph
            int thisUID1 = (_CroTrByUID.at(doorUID1)->GetSubRoom1()) ?
                               _CroTrByUID.at(doorUID1)->GetSubRoom1()->GetUID() :
                               -10;
            int thisUID2 = (_CroTrByUID.at(doorUID1)->GetSubRoom2()) ?
                               _CroTrByUID.at(doorUID1)->GetSubRoom2()->GetUID() :
                               -20;
            int otherUID1 = (_CroTrByUID.at(doorUID2)->GetSubRoom1()) ?
                                _CroTrByUID.at(doorUID2)->GetSubRoom1()->GetUID() :
                                -30;
            int otherUID2 = (_CroTrByUID.at(doorUID2)->GetSubRoom2()) ?
                                _CroTrByUID.at(doorUID2)->GetSubRoom2()->GetUID() :
                                -40;

            if((thisUID1 != otherUID1) && (thisUID1 != otherUID2) && (thisUID2 != otherUID1) &&
               (thisUID2 != otherUID2)) {
                continue;
            }

            UnivFFviaFM * locffptr = _locffviafm[roomID1];
            double tempDistance    = locffptr->GetDistanceBetweenDoors(doorUID1, doorUID2);

            if(tempDistance < locffptr->GetGrid()->Gethx()) {
                LOG_WARNING(
                    "Ignoring distance of doors {:d} and {:d} because it is too small: "
                    "{:.2f}.",
                    doorUID1,
                    doorUID2,
                    tempDistance);
                continue;
            }

            //
            std::pair<int, int> key_ij = std::make_pair(doorUID2, doorUID1);
            std::pair<int, int> key_ji = std::make_pair(doorUID1, doorUID2);

            if(_distMatrix.at(key_ij) > tempDistance) {
                _distMatrix.erase(key_ij);
                _distMatrix.erase(key_ji);
                _distMatrix.insert(std::make_pair(key_ij, tempDistance));
                _distMatrix.insert(std::make_pair(key_ji, tempDistance));
            }
        } // otherDoor
    }     // roomAndCroTrVector

    // penalize directional escalators
    std::vector<std::pair<int, int>> penaltyList;

    if(_config->get_has_directional_escalators()) {
        _directionalEscalatorsUID.clear();
        penaltyList.clear();
        for(const auto & room : _building->GetAllRooms()) {
            for(const auto & [_, subroom] : room.second->GetAllSubRooms()) {
                if((subroom->GetType() == "escalator_up") ||
                   (subroom->GetType() == "escalator_down")) {
                    _directionalEscalatorsUID.emplace_back(subroom->GetUID());
                }
            }
        }
        for(int subUID : _directionalEscalatorsUID) {
            auto * escalator          = (Escalator *) _building->GetSubRoomByUID(subUID);
            std::vector<int> lineUIDs = escalator->GetAllGoalIDs();
            assert(lineUIDs.size() == 2);
            if(escalator->IsEscalatorUp()) {
                if(_CroTrByUID[lineUIDs[0]]->IsInLineSegment(escalator->GetUp())) {
                    penaltyList.emplace_back(std::make_pair(lineUIDs[0], lineUIDs[1]));
                } else {
                    penaltyList.emplace_back(std::make_pair(lineUIDs[1], lineUIDs[0]));
                }
            } else { //IsEscalatorDown
                if(_CroTrByUID[lineUIDs[0]]->IsInLineSegment(escalator->GetUp())) {
                    penaltyList.emplace_back(std::make_pair(lineUIDs[1], lineUIDs[0]));
                } else {
                    penaltyList.emplace_back(std::make_pair(lineUIDs[0], lineUIDs[1]));
                }
            }
        }
    }

    //penalize closed doors
    for(auto doorID : _allDoorUIDs) {
        if(auto door = dynamic_cast<Crossing *>(_building->GetTransOrCrossByUID(doorID))) {
            if(door->IsClose()) {
                for(auto doorID2 : _allDoorUIDs) {
                    if(doorID == doorID2) {
                        continue;
                    }
                    std::pair<int, int> connection1 = std::make_pair(doorID, doorID2);
                    std::pair<int, int> connection2 = std::make_pair(doorID2, doorID);

                    if(_distMatrix.find(connection1) != _distMatrix.end()) {
                        penaltyList.emplace_back(connection1);
                        penaltyList.emplace_back(connection2);
                    }
                }
            }
        }
    }

    for(auto key : penaltyList) {
        _distMatrix.erase(key);
        _distMatrix.insert(std::make_pair(key, std::numeric_limits<double>::max()));
    }

    FloydWarshall();
}

int FFRouter::FindExit(Pedestrian * p)
{
    if(_strategy == ROUTING_FF_QUICKEST) {
        if(Pedestrian::GetGlobalTime() > _recalc_interval &&
           _building->GetRoom(p->GetRoomID())->GetSubRoom(p->GetSubRoomID())->IsInSubRoom(p) &&
           _locffviafm[p->GetRoomID()]->GetCostToDestination(p->GetExitIndex(), p->GetPos()) >
               3.0 &&
           p->GetExitIndex() != -1) {
            //delay possible
            if((int) Pedestrian::GetGlobalTime() % 10 != p->GetID() % 10) {
                return p->GetExitIndex(); //stay with old target
            }
        }
        //new version: recalc densityspeed every x seconds
        if((Pedestrian::GetGlobalTime() > _timeToRecalc) &&
           (Pedestrian::GetGlobalTime() > Pedestrian::GetMinPremovementTime() + _recalc_interval)) {
            _plzReInit = true;
        }
    }
    double minDist = std::numeric_limits<double>::infinity();
    int bestDoor   = -1;

    int goalID = p->GetFinalDestination();

    if(auto * wa = dynamic_cast<WaitingArea *>(_building->GetFinalGoal(goalID))) {
        bestDoor = wa->GetCentreCrossing()->GetUniqueID();
        p->SetExitIndex(bestDoor);
        p->SetExitLine(_CroTrByUID.at(bestDoor));
        return bestDoor;
    }

    std::vector<int> validFinalDoor; //UIDs of doors
    validFinalDoor.clear();
    if(goalID == -1) {
        for(auto & pairDoor : _ExitsByUID) {
            // we add all open/temp_close exits
            if(_building->GetTransitionByUID(pairDoor.first)->IsOpen() ||
               _building->GetTransitionByUID(pairDoor.first)->IsTempClose()) {
                validFinalDoor.emplace_back(pairDoor.first); //UID
            }
        }
    } else { //only one specific goal, goalToLineUIDmap gets
        //populated in Init()
        if((_goalToLineUIDmap.count(goalID) == 0) || (_goalToLineUIDmap[goalID] == -1)) {
            LOG_ERROR("ffRouter: unknown/unreachable goalID: {:d} in FindExit(Ped)", goalID);
        } else {
            validFinalDoor.emplace_back(_goalToLineUIDmap.at(goalID));
        }
    }

    std::vector<int> DoorUIDsOfRoom;

    if(!_targetWithinSubroom) {
        //candidates of current room (ID) (provided by Room)
        for(auto transUID : _building->GetRoom(p->GetRoomID())->GetAllTransitionsIDs()) {
            if(_CroTrByUID.count(transUID) != 0) {
                DoorUIDsOfRoom.emplace_back(transUID);
            }
        }
        for(auto & subIPair : _building->GetRoom(p->GetRoomID())->GetAllSubRooms()) {
            for(auto & crossI : subIPair.second->GetAllCrossings()) {
                DoorUIDsOfRoom.emplace_back(crossI->GetUniqueID());
            }
        }
    } else {
        //candidates of current subroom only
        for(auto & crossI :
            _building->GetRoom(p->GetRoomID())->GetSubRoom(p->GetSubRoomID())->GetAllCrossings()) {
            DoorUIDsOfRoom.emplace_back(crossI->GetUniqueID());
        }

        for(auto & transI : _building->GetRoom(p->GetRoomID())
                                ->GetSubRoom(p->GetSubRoomID())
                                ->GetAllTransitions()) {
            if(transI->IsOpen() || transI->IsTempClose()) {
                DoorUIDsOfRoom.emplace_back(transI->GetUniqueID());
            }
        }
    }

    int bestFinalDoor = -1; // to silence the compiler
    for(int finalDoor : validFinalDoor) {
        //with UIDs, we can ask for shortest path
        for(int doorUID : DoorUIDsOfRoom) {
            //double locDistToDoor = _locffviafm[p->GetRoomID()]->GetCostToDestination(doorUID, p->GetPos(), _mode);
            double locDistToDoor =
                _config->GetDirectionManager()->GetDirectionStrategy()->GetDistance2Target(
                    p, doorUID);

            if(locDistToDoor <
               -J_EPS) { //for old ff: //this can happen, if the point is not reachable and therefore has init val -7
                continue;
            }
            std::pair<int, int> key = std::make_pair(doorUID, finalDoor);
            //auto subroomDoors = _building->GetSubRoomByUID(p->GetSubRoomUID())->GetAllGoalIDs();
            //only consider, if paths exists
            if(_pathsMatrix.count(key) == 0) {
                LOG_ERROR("ffRouter: no key for {:d} {:d}", key.first, key.second);
                continue;
            }

            if((_distMatrix.count(key) != 0) &&
               (_distMatrix.at(key) != std::numeric_limits<double>::infinity())) {
                if((_distMatrix.at(key) + locDistToDoor) < minDist) {
                    minDist  = _distMatrix.at(key) + locDistToDoor;
                    bestDoor = key.first; //doorUID
                    auto subroomDoors =
                        _building->GetSubRoomByUID(p->GetSubRoomUID())->GetAllGoalIDs();
                    if(std::find(subroomDoors.begin(), subroomDoors.end(), _pathsMatrix[key]) !=
                       subroomDoors.end()) {
                        bestDoor = _pathsMatrix[key]; //@todo: @ar.graf: check this hack
                    }
                    bestFinalDoor = key.second;
                }
            }
        }
    }

    //at this point, bestDoor is either a crossing or a transition
    if((!_targetWithinSubroom) && (_CroTrByUID.count(bestDoor) != 0)) {
        while(!_CroTrByUID[bestDoor]->IsTransition()) {
            std::pair<int, int> key = std::make_pair(bestDoor, bestFinalDoor);
            bestDoor                = _pathsMatrix[key];
        }
    }

    if(_CroTrByUID.count(bestDoor)) {
        p->SetExitIndex(bestDoor);
        p->SetExitLine(_CroTrByUID.at(bestDoor));
    }
    return bestDoor; //-1 if no way was found, doorUID of best, if path found
}

void FFRouter::FloydWarshall()
{
    bool change  = false;
    int totalnum = _allDoorUIDs.size();
    for(int k = 0; k < totalnum; ++k) {
        for(int i = 0; i < totalnum; ++i) {
            for(int j = 0; j < totalnum; ++j) {
                std::pair<int, int> key_ij = std::make_pair(_allDoorUIDs[i], _allDoorUIDs[j]);
                std::pair<int, int> key_ik = std::make_pair(_allDoorUIDs[i], _allDoorUIDs[k]);
                std::pair<int, int> key_kj = std::make_pair(_allDoorUIDs[k], _allDoorUIDs[j]);
                if((_distMatrix[key_ik] < std::numeric_limits<double>::max()) &&
                   (_distMatrix[key_kj] < std::numeric_limits<double>::max()) &&
                   (_distMatrix[key_ik] + _distMatrix[key_kj] < _distMatrix[key_ij])) {
                    _distMatrix.erase(key_ij);
                    _distMatrix.insert(
                        std::make_pair(key_ij, _distMatrix[key_ik] + _distMatrix[key_kj]));
                    _pathsMatrix.erase(key_ij);
                    _pathsMatrix.insert(std::make_pair(key_ij, _pathsMatrix[key_ik]));
                    change = true;
                }
            }
        }
    }
    if(change) {
        FloydWarshall();
    } else {
        LOG_INFO("ffRouter: FloydWarshall done!");
    }
}

bool FFRouter::MustReInit()
{
    return _plzReInit;
}

void FFRouter::SetRecalc(double t)
{
    _timeToRecalc = t + _recalc_interval;
}

void FFRouter::Update()
{
    this->CalculateFloorFields();
    if(_config->get_write_VTK_files()) {
        for(auto const & [roomID, localFF] : _locffviafm) {
            localFF->WriteFF(
                fmt::format(
                    FMT_STRING("ffrouterRoom_{:d}_t_{:.2f}.vtk"),
                    roomID,
                    Pedestrian::GetGlobalTime()),
                _allDoorUIDs);
        }
    }
}
