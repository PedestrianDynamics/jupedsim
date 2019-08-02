/**
 * \file        FloorfieldViaFM.cpp
 * \date        Mar 05, 2015
 * \version     N/A (v0.6)
 * \copyright   <2009-2014> Forschungszentrum Jülich GmbH. All rights reserved.
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
 * Implementation of classes for ...
 *
 *
 **/
#define TESTING
#define GEO_UP_SCALE 1
#include "FloorfieldViaFM.h"

#include "general/OpenMP.h"

FloorfieldViaFM::FloorfieldViaFM()
{
    //ctor (very ugly)
    //std::cerr << "The defaultconsturctor FloorfieldViaFM should not be called!" << std::endl;
}

FloorfieldViaFM::~FloorfieldViaFM()
{
    //dtor
    delete _grid;
    if (_gcode) delete[] _gcode;
    if (_subrooms) delete[] _subrooms;
    if (_dist2Wall) delete[] _dist2Wall;
    if (_speedInitial) delete[] _speedInitial;
    if (_modifiedspeed) delete[] _modifiedspeed;
    if (_densityspeed) delete[] _densityspeed;
    //if (cost) delete[] cost;
    //if (neggrad) delete[] neggrad;
    if (_dirToWall) delete[] _dirToWall;
    //if (trialfield) delete[] trialfield;
    for ( const auto& goalid : _goalcostmap) {
        if (goalid.second) delete[] goalid.second;
    }
    for ( const auto& id : _costmap) {
        //if (id.first == -1) continue;
        if (id.second) delete[] id.second;
        if (_neggradmap.at(id.first)) delete[] _neggradmap.at(id.first);
        //map will be deleted by itself
    }

}

FloorfieldViaFM::FloorfieldViaFM(const Building* const buildingArg, const double hxArg, const double hyArg,
                                 const double wallAvoidDistance, const bool useDistancefield, const bool onlyRoomsWithExits) {
    //ctor
    //_threshold = -1; //negative value means: ignore threshold
    _threshold = wallAvoidDistance;
    _building = buildingArg;
    _useDistanceToWall = useDistancefield;

    if (hxArg != hyArg) {
        //std::cerr << "ERROR: hx != hy <=========";
        Log->Write("WARNING: \tFloor field: stepsize hx differs from hy! Taking hx = %d for both.", hxArg);
    }
    //parse building and create list of walls/obstacles (find xmin xmax, ymin, ymax, and add border?)
    //Log->Write("INFO: \tStart Parsing: Building");
    if (onlyRoomsWithExits) {
        parseBuildingForExits(buildingArg, hxArg, hyArg);
    } else {
        parseBuilding(buildingArg, hxArg, hyArg);
    }
    //Log->Write("INFO: \tFinished Parsing: Building");
    //testoutput("AALineScan.vtk", "AALineScan.txt", dist2Wall);

    prepareForDistanceFieldCalculation(onlyRoomsWithExits);
    //Log->Write("INFO: \tGrid initialized: Walls");

    calculateDistanceField(-1.); //negative threshold is ignored, so all distances get calculated. this is important since distances is used for slowdown/redirect
    //Log->Write("INFO: \tGrid initialized: Walldistances");

    setSpeed(useDistancefield); //use distance2Wall
    //Log->Write("INFO: \tGrid initialized: Speed");

    calculateFloorfield(_exitsFromScope, _cost, _neggrad);
    //writing FF-file disabled, we will not revive it ( @todo: argraf )
}

FloorfieldViaFM::FloorfieldViaFM(const FloorfieldViaFM &other) :
_building(other._building)

{
    _grid = other.getGrid();
    long int otherNumOfPoints = _grid->GetnPoints();

    _wall.clear();
    _wall.reserve(other._wall.size());
    std::copy(other._wall.begin(), other._wall.end(), _wall.begin());

    _exitsFromScope.clear();
    _exitsFromScope.reserve(other._exitsFromScope.size());
    std::copy(other._exitsFromScope.begin(), other._exitsFromScope.end(), _exitsFromScope.begin());

    _numOfExits = other._numOfExits;

    _gcode = new int[otherNumOfPoints];
    std::copy(other._gcode, other._gcode + otherNumOfPoints, _gcode);

    _dist2Wall = new double[otherNumOfPoints];
    std::copy(other._dist2Wall, other._dist2Wall + otherNumOfPoints, _dist2Wall);

    _speedInitial = new double[otherNumOfPoints];
    std::copy(other._speedInitial, other._speedInitial + otherNumOfPoints, _speedInitial);

    _modifiedspeed = new double[otherNumOfPoints];
    std::copy(other._modifiedspeed, other._modifiedspeed + otherNumOfPoints, _modifiedspeed);

    _densityspeed = new double[otherNumOfPoints];
    std::copy(other._densityspeed, other._densityspeed + otherNumOfPoints, _densityspeed);

    _cost = new double[otherNumOfPoints];
    std::copy(other._cost, other._cost + otherNumOfPoints, _cost);

    _neggrad = new Point[otherNumOfPoints]; //gradients
    std::copy(other._neggrad, other._neggrad + otherNumOfPoints, _neggrad);

    _dirToWall = new Point[otherNumOfPoints];
    std::copy(other._dirToWall, other._dirToWall + otherNumOfPoints, _dirToWall);

    _threshold = other._threshold;
    _useDistanceToWall = other._useDistanceToWall;
}

FloorfieldViaFM::FloorfieldViaFM(const std::string& filename) {


    Log->Write("ERROR: \tReading  FF from file not supported!!");
    Log->Write(filename);

}

//void FloorfieldViaFM::getDirectionAt(const Point& position, Point& direction){
//    long int key = grid->getKeyAtPoint(position);
//    direction._x = (neggrad[key]._x);
//    direction._y = (neggrad[key]._y);
//}

void FloorfieldViaFM::getDirectionToDestination(Pedestrian* ped, Point& direction){
    const Point& position = ped->GetPos();
    int destID = ped->GetExitIndex();
    long int key = _grid->getKeyAtPoint(position);
    getDirectionToUID(destID, key, direction);
    if (direction._x == DBL_MAX && direction._y == DBL_MAX) {
        // This can be interpreted differently by the different operational models.
        // The inaccuracy introduced by this is negligible. --f.mack
         direction._x = 0;
         direction._y = 0;
    }
}

void FloorfieldViaFM::getDirectionToUID(int destID, const long int key, Point &direction) {
    getDirectionToUID(destID, key, direction, global_shortest);
}

void FloorfieldViaFM::getDirectionToUID(int destID, const long int key, Point& direction, int mode) {
    //what if goal == -1, meaning closest exit... is GetExitIndex then -1? NO... ExitIndex is UID, given by router
    //if (ped->GetFinalDestination() == -1) /*go to closest exit*/ destID != -1;

    if ((key < 0) || (key >= _grid->GetnPoints())) { // @todo: ar.graf: this check in a #ifdef-block?
        Log->Write("ERROR: \t Floorfield tried to access a key out of grid!");
        direction._x = 0.;
        direction._y = 0.;
        return;
    }
    Point* localneggradptr = nullptr;
    double* localcostptr = nullptr;
    {
        if (_neggradmap.count(destID) == 0) {
            //Log->Write("FF for destID %d does not exist (key is %d)", destID, key);
            //check, if distID is in this grid
            Hline* destLine = _building->GetTransOrCrossByUID(destID);
            Point A = destLine->GetPoint1();
            Point B = destLine->GetPoint2();
            if (!(_grid->includesPoint(A)) || !(_grid->includesPoint(B))) {
                Log->Write("ERROR: \t Destination ID %d is not in grid!", destID);
                direction._x = direction._y = 0.;
                //return;
            }
        }
        localneggradptr = (_neggradmap.count(destID) == 0) ? nullptr : _neggradmap.at(destID);
        localcostptr = (_costmap.count(destID) == 0) ? nullptr : _costmap.at(destID);
        if (localneggradptr == nullptr) {
            bool isBeingCalculated;
#pragma omp critical(floorfieldsBeingCalculated)
            {
                if (!(isBeingCalculated = _floorfieldsBeingCalculated.count(destID) > 0)) {
                    _floorfieldsBeingCalculated.insert(destID);
                }
            }
            if (isBeingCalculated) {
                // we do not want to wait until the other calculation has finished, so we return immediately
                // the values are corrected in getDirectionToDestination(), and getCostToDestination doesn't care about the direction
                direction._x = DBL_MAX;
                direction._y = DBL_MAX;
                return;
            }

            //create floorfield (remove mapentry with nullptr, allocate memory, add mapentry, create ff)
            localcostptr =    new double[_grid->GetnPoints()];
            localneggradptr = new Point[_grid->GetnPoints()];
#pragma omp critical(neggradmap)
            _neggradmap.erase(destID);
#pragma omp critical(neggradmap)
            _neggradmap.emplace(destID, localneggradptr);
#pragma omp critical(costmap)
            _costmap.erase(destID);
#pragma omp critical(costmap)
            _costmap.emplace(destID, localcostptr);

                //create ff (prepare Trial-mechanic, then calc)
//                for (long int i = 0; i < grid->GetnPoints(); ++i) {
//                    //set Trialptr to fieldelements
//                    trialfield[i].cost = localcostptr + i;
//                    trialfield[i].neggrad = localneggradptr + i;
//                    trialfield[i].father = nullptr;
//                    trialfield[i].child = nullptr;
//                }
//                clearAndPrepareForFloorfieldReCalc(localcostptr);
            std::vector<Line> localline = {Line((Line) *(_building->GetTransOrCrossByUID(destID)))};
//                setNewGoalAfterTheClear(localcostptr, localline);
                //Log->Write("Starting FF for UID %d (ID %d)", destID, dynamic_cast<Crossing*>(building->GetTransOrCrossByUID(destID))->GetID());
                //std::cerr << "\rW\tO\tR\tK\tI\tN\tG";
            if (mode == quickest) {
                calculateFloorfield(localline, localcostptr, localneggradptr, _densityspeed);
            } else {
                calculateFloorfield(localline, localcostptr, localneggradptr, _modifiedspeed);
            }
#pragma omp critical(floorfieldsBeingCalculated)
            {
                if (_floorfieldsBeingCalculated.count(destID) != 1) {
                    Log->Write("ERROR: FloorfieldViaFM::getDirectionToUID: key %d was calculating FF for destID %d, but it was removed from floorfieldsBeingCalculated meanwhile", key, destID);
                }
                _floorfieldsBeingCalculated.erase(destID);
            }
                //Log->Write("Ending   FF for UID %d", destID);
                //std::cerr << "\r W\t O\t R\t K\t I\t N\t G";
        }
    }
    direction._x = (localneggradptr[key]._x);
    direction._y = (localneggradptr[key]._y);
}

void FloorfieldViaFM::createMapEntryInLineToGoalID(const int goalID)
{
    Point* localneggradptr;
    double* localcostptr;
    if (goalID < 0) {
        Log->Write("WARNING: \t goalID was negative in FloorfieldViaFM::createMapEntryInLineToGoalID");
        return;
    }
    if (!_building->GetFinalGoal(goalID)) {
        Log->Write("WARNING: \t goalID was unknown in FloorfieldViaFM::createMapEntryInLineToGoalID");
        return;
    }

    // The scope of this critical section can probably be reduced (maybe use a GoalsBeingCalculated similar to FloorfieldViaFM::getDirectionToUID)
#pragma omp critical(FloorfieldViaFM_maps)
    {
        if (_goalcostmap.count(goalID) == 0) { //no entry for goalcostmap, so we need to calc FF
            _goalcostmap.emplace(goalID, nullptr);
            _goalneggradmap.emplace(goalID, nullptr);
            _goalToLineUIDmap.emplace(goalID, -1);
            _goalToLineUIDmap2.emplace(goalID, -1);
            _goalToLineUIDmap3.emplace(goalID, -1);
        }
        localneggradptr = _goalneggradmap.at(goalID);
        localcostptr = _goalcostmap.at(goalID);
        if (localneggradptr == nullptr) {
            //create floorfield (remove mapentry with nullptr, allocate memory, add mapentry, create ff)
            localcostptr =    new double[_grid->GetnPoints()];
            localneggradptr = new Point[_grid->GetnPoints()];
            _goalneggradmap.erase(goalID);
            //goalneggradmap.emplace(goalID, localneggradptr);
            _goalcostmap.erase(goalID);
            //goalcostmap.emplace(goalID, localcostptr);
            //create ff (prepare Trial-mechanic, then calc)
//            for (long int i = 0; i < grid->GetnPoints(); ++i) {
//                //set Trialptr to fieldelements
//                trialfield[i].cost = localcostptr + i;
//                trialfield[i].neggrad = localneggradptr + i;
//                trialfield[i].father = nullptr;
//                trialfield[i].child = nullptr;
//            }
//            clearAndPrepareForFloorfieldReCalc(localcostptr);

            //get all lines/walls of goalID
            std::vector<Line> localline;
            const std::map<int, Goal*>& allgoals = _building->GetAllGoals();
            std::vector<Wall> localwalls = allgoals.at(goalID)->GetAllWalls();

            double xMin = _grid->GetxMin();
            double xMax = _grid->GetxMax();

            double yMin = _grid->GetyMin();
            double yMax = _grid->GetyMax();

            for (const auto& iwall:localwalls) {
                const Point& a = iwall.GetPoint1();
                const Point& b = iwall.GetPoint2();
                if (
                      (a._x >= xMin) && (a._x <= xMax)
                    &&(a._y >= yMin) && (a._y <= yMax)
                    &&(b._x >= xMin) && (b._x <= xMax)
                    &&(b._y >= yMin) && (b._y <= yMax)
                      )
                {
                    localline.emplace_back( Line( (Line) iwall ) );
                } else {
                    std::cerr << "GOAL " << goalID << " includes point out of grid!" << std::endl;
                    std::cerr << "Point: " << a._x << ", " << a._y << std::endl;
                    std::cerr << "Point: " << b._x << ", " << b._y << std::endl;
                }
            }

//            setNewGoalAfterTheClear(localcostptr, localline);

            //performance-measurement:
            //auto start = std::chrono::steady_clock::now();

            calculateFloorfield(localline, localcostptr, localneggradptr);

            //performance-measurement:
            //auto end = std::chrono::steady_clock::now();
            //auto diff = end - start;
            //std::cerr << std::chrono::duration_cast<std::chrono::seconds>(end - start).count() << std::endl;
            //std::cerr << "new GOALfield " << goalID << "   :    " << localline[0].GetPoint1().GetX() << " " << localline[0].GetPoint1().GetY() << " " << localline[0].GetPoint2().GetX() << " " << localline[0].GetPoint2().GetY() << std::endl;
            //Log->Write("new GOALfield " + std::to_string(goalID) + "  :   " + std::to_string(localline[0].GetPoint1().GetX()));
            //Log->Write("new GOALfield " + std::to_string(goalID) + "  :   " + std::to_string( std::chrono::duration_cast<std::chrono::seconds>(end - start).count() ) + " " + std::to_string(localline.size()) );
            //find closest door and add to cheatmap "goalToLineUID" map

            const std::map<int, Transition*>& transitions = _building->GetAllTransitions();
            int UID_of_MIN = -1;
            int UID_of_MIN2 = -1;
            int UID_of_MIN3 = -1;
            double cost_of_MIN = DBL_MAX;
            double cost_of_MIN2 = DBL_MAX;
            double cost_of_MIN3 = DBL_MAX;
            long int dummykey;
            for (const auto& loctrans : transitions) {
                //TODO if (!loctrans.second->IsExit() || !loctrans.second->IsOpen()) {
                if (!loctrans.second->IsExit() || loctrans.second->IsClose()) {
                    continue;
                }
                dummykey = _grid->getKeyAtPoint(loctrans.second->GetCentre());
                if ((cost_of_MIN > localcostptr[dummykey]) && (localcostptr[dummykey] >= 0.)) {
                    UID_of_MIN3 = UID_of_MIN2;
                    cost_of_MIN3 = cost_of_MIN2;

                    UID_of_MIN2 = UID_of_MIN;
                    cost_of_MIN2 = cost_of_MIN;

                    UID_of_MIN = loctrans.second->GetUniqueID();
                    cost_of_MIN = localcostptr[dummykey];
                    //std::cerr << std::endl << "Closer Line found: " << UID_of_MIN ;
                    continue;
                }
                if ((cost_of_MIN2 > localcostptr[dummykey]) && (localcostptr[dummykey] >= 0.)) {
                    UID_of_MIN3 = UID_of_MIN2;
                    cost_of_MIN3 = cost_of_MIN2;

                    UID_of_MIN2 = loctrans.second->GetUniqueID();
                    cost_of_MIN2 = localcostptr[dummykey];
                    continue;
                }
                if ((cost_of_MIN3 > localcostptr[dummykey]) && (localcostptr[dummykey] >= 0.)) {
                    UID_of_MIN3 = loctrans.second->GetUniqueID();
                    cost_of_MIN3 = localcostptr[dummykey];
                    continue;
                }
            }
            _goalToLineUIDmap.erase(goalID);
            _goalToLineUIDmap.emplace(goalID, UID_of_MIN);
            _goalToLineUIDmap2.erase(goalID);
            _goalToLineUIDmap2.emplace(goalID, UID_of_MIN2);
            _goalToLineUIDmap3.erase(goalID);
            _goalToLineUIDmap3.emplace(goalID, UID_of_MIN3);
            delete[] localcostptr;
            delete[] localneggradptr;
        }
    }
}

double FloorfieldViaFM::getCostToDestination(const int destID, const Point& position) {
    return getCostToDestination(destID, position, global_shortest);
}

double FloorfieldViaFM::getCostToDestination(const int destID, const Point& position, int mode) {
    if ((_costmap.count(destID) == 0) || (_costmap.at(destID) == nullptr)) {
        Point dummy;
        getDirectionToUID(destID, 0, dummy, mode);         //this call induces the floorfieldcalculation
    }
    if ((_costmap.count(destID) == 0) || (_costmap.at(destID) == nullptr)) {
        Log->Write("ERROR: \tDestinationUID %d is invalid / out of grid.", destID);
        return DBL_MAX;
    }
    if (_grid->getKeyAtPoint(position) == -1) {  //position is out of grid
        return -7;
    }
    return (_costmap.at(destID))[_grid->getKeyAtPoint(position)];
}

void FloorfieldViaFM::getDir2WallAt(const Point& position, Point& direction){
    long int key = _grid->getKeyAtPoint(position);
    //debug assert
    if (key < 0) {
        Log->Write("ERROR: \tgetDir2WallAt error");
    } else {
        direction._x = (_dirToWall[key]._x);
        direction._y = (_dirToWall[key]._y);
    }
}

double FloorfieldViaFM::getDistance2WallAt(const Point& position) {
    long int key = _grid->getKeyAtPoint(position);
    //debug assert
    if (key < 0) {
        Log->Write("ERROR: \tgetDistance2WallAt error");
        return 1.;
    } else {
        return _dist2Wall[key];
    }
}

//int  FloorfieldViaFM::getSubroomUIDAt(const Point &position) {
//    long int key = grid->getKeyAtPoint(position);
//    return subroomUID[key];
//}

/*!
 * \brief Parsing geo-info but conflicts in multi-floor-buildings OBSOLETE
 *
 * When parsing a building with multiple floors, the projection of all floors onto the x-/y- plane leads to wrong
 * results.
 *
 * \param[in] buildingArg provides the handle to all information
 * \param[out] stepSizeX/-Y discretization of the grid, which will be created here
 * \return member attributes of FloorfieldViaFM class are allocated and initialized
 * \sa Macros.h, RectGrid.h
 * \note
 * \warning
 */
void FloorfieldViaFM::parseBuilding(const Building* const buildingArg, const double stepSizeX, const double stepSizeY) {
    _building = buildingArg;
    //init min/max before parsing
    double xMin = DBL_MAX;
    double xMax = -DBL_MAX;
    double yMin = xMin;
    double yMax = xMax;

    if (stepSizeX != stepSizeY) Log->Write("ERROR: \tStepsizes in x- and y-direction must be identical!");

    _costmap.clear();
    _neggradmap.clear();
    _wall.clear();
    _exitsFromScope.clear();

    //create a list of walls
    const std::map<int, Transition*>& allTransitions = buildingArg->GetAllTransitions();
    for (auto& trans : allTransitions) {
        //TODO if (trans.second->IsExit() && trans.second->IsOpen())
        if (trans.second->IsExit() && !trans.second->IsClose())
        {
            _exitsFromScope.emplace_back(Line ( (Line) *(trans.second)));
        }
        //populate both maps: costmap, neggradmap. These are the lookup maps for floorfields to specific transitions
        _costmap.emplace(trans.second->GetUniqueID(), nullptr);
        _neggradmap.emplace(trans.second->GetUniqueID(), nullptr);
    }
    _numOfExits = (unsigned int) _exitsFromScope.size();
    for (auto& trans : allTransitions) {
        //TODO if (!trans.second->IsOpen()) {
        if (!trans.second->IsOpen()) {
            _wall.emplace_back(Line ( (Line) *(trans.second)));
        }

    }
    for (const auto& itRoom : buildingArg->GetAllRooms()) {
        for (const auto& itSubroom : itRoom.second->GetAllSubRooms()) {
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

            const std::vector<Crossing*>& allCrossings = itSubroom.second->GetAllCrossings();
            for (Crossing* crossPtr : allCrossings) {
                //TODO if (!crossPtr->IsOpen()) {
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
    }

    //all goals
    const std::map<int, Goal*>& allgoals = buildingArg->GetAllGoals();
    for (auto eachgoal:allgoals) {
        for (auto& eachwall:eachgoal.second->GetAllWalls() ) {
            if (eachwall.GetPoint1()._x < xMin) xMin = eachwall.GetPoint1()._x;
            if (eachwall.GetPoint2()._x < xMin) xMin = eachwall.GetPoint2()._x;
            if (eachwall.GetPoint1()._x > xMax) xMax = eachwall.GetPoint1()._x;
            if (eachwall.GetPoint2()._x > xMax) xMax = eachwall.GetPoint2()._x;

            if (eachwall.GetPoint1()._y < yMin) yMin = eachwall.GetPoint1()._y;
            if (eachwall.GetPoint2()._y < yMin) yMin = eachwall.GetPoint2()._y;
            if (eachwall.GetPoint1()._y > yMax) yMax = eachwall.GetPoint1()._y;
            if (eachwall.GetPoint2()._y > yMax) yMax = eachwall.GetPoint2()._y;
        }
        _goalcostmap.emplace(eachgoal.second->GetId(), nullptr);
        _goalneggradmap.emplace(eachgoal.second->GetId(), nullptr);
    }

    //create Rect Grid
    _grid = new RectGrid();
    _grid->setBoundaries(xMin, yMin, xMax, yMax);
    _grid->setSpacing(stepSizeX, stepSizeX);
    _grid->createGrid();

    //create arrays
    _subrooms = new SubRoom*[_grid->GetnPoints()]();
    _gcode = new int[_grid->GetnPoints()];                  //gcode:
                                                            //    enum GridCode { //used in floor fields
                                                            //         WALL = 0,
                                                            //         INSIDE,
                                                            //         OUTSIDE,
                                                            //         OPEN_CROSSING,
                                                            //         OPEN_TRANSITION,
                                                            //         CLOSED_CROSSING,
                                                            //         CLOSED_TRANSITION
                                                            //    };
    _dist2Wall = new double[_grid->GetnPoints()];
    _speedInitial = new double[_grid->GetnPoints()];
    _modifiedspeed = new double[_grid->GetnPoints()];
    _densityspeed = new double[_grid->GetnPoints()];
    _cost = new double[_grid->GetnPoints()];
    _neggrad = new Point[_grid->GetnPoints()];
    _dirToWall = new Point[_grid->GetnPoints()];

    _costmap.emplace(-1 , _cost);                         // enable default ff (closest exit)
    _neggradmap.emplace(-1, _neggrad);

    //init grid with -3 as unknown distance to any wall
    for(long int i = 0; i < _grid->GetnPoints(); ++i) {
        _dist2Wall[i]    = -3.;
        _cost[i]         = -2.;
        _gcode[i]        = OUTSIDE;
        //flag[i] = FM_UNKNOWN;            //unknown
    }
    drawLinesOnGrid<double>(_wall, _dist2Wall, 0.);
    drawLinesOnGrid<double>(_wall, _cost, -7.);
    drawLinesOnGrid<int>(_wall, _gcode, WALL);
    drawLinesOnGrid<int>(_exitsFromScope, _gcode, OPEN_TRANSITION);
}

/*!
 * \brief Parsing geo-info ONLY considering rooms with EXITS to the outside to avoid conflicts in multi-floor-buildings
 *
 * When parsing a building with multiple floors, the projection of all floors onto the x-/y- plane leads to wrong
 * results. We then decided, to consider each floor separately and each staircase separately.
 * We still need to match the ouside goals to an exit (transition), so that the router can guide agents to that goals.
 * Next problem was in buildings, where exits to the outside would now be on different rooms. This is why we create this
 * function. We want to create one floorfield for all rooms, that lead outside. Reason: We want the router to lead agents
 * to the maybe second or third best exit-door, which might be in a different room.
 *
 * \param[in] buildingArg provides the handle to all information
 * \param[out] stepSizeX/-Y discretization of the grid, which will be created here
 * \return member attributes of FloorfieldViaFM class are allocated and initialized
 * \sa Macros.h, RectGrid.h
 * \note
 * \warning
 */
void FloorfieldViaFM::parseBuildingForExits(const Building* const buildingArg, const double stepSizeX, const double stepSizeY) {
    _building = buildingArg;
    //init min/max before parsing
    double xMin = DBL_MAX;
    double xMax = -DBL_MAX;
    double yMin = xMin;
    double yMax = xMax;

    if (stepSizeX != stepSizeY) Log->Write("ERROR: \tStepsizes in x- and y-direction must be identical!");

    _costmap.clear();
    _neggradmap.clear();
    _wall.clear();
    _exitsFromScope.clear();

    std::vector<int> exitRoomIDs;
    exitRoomIDs.clear();

    //create a list of walls
    const std::map<int, Transition*>& allTransitions = buildingArg->GetAllTransitions();
    for (auto& trans : allTransitions) {
        //TODO if (trans.second->IsExit() && trans.second->IsOpen())
        if (trans.second->IsExit() && !trans.second->IsClose())
        {
            _exitsFromScope.emplace_back(Line ( (Line) *(trans.second)));
            int roomID = -1;
            if (trans.second->GetRoom1()) {
                roomID = trans.second->GetRoom1()->GetID();
            }
            if (trans.second->GetRoom2()) {
                roomID = trans.second->GetRoom2()->GetID();
            }
            if (std::find(exitRoomIDs.begin(), exitRoomIDs.end(), roomID) == exitRoomIDs.end()) {
                exitRoomIDs.emplace_back(roomID);
            }
        }
        //populate both maps: costmap, neggradmap. These are the lookup maps for floorfields to specific transitions
        _costmap.emplace(trans.second->GetUniqueID(), nullptr);
        _neggradmap.emplace(trans.second->GetUniqueID(), nullptr);
    }
    _numOfExits = (unsigned int) _exitsFromScope.size();
    for (auto& trans : allTransitions) {
        //TODO if (!trans.second->IsOpen()) {
        if (!trans.second->IsOpen()) {
            _wall.emplace_back(Line ( (Line) *(trans.second)));
        }

    }
    for (const auto& itRoom : buildingArg->GetAllRooms()) {
        if (std::find(exitRoomIDs.begin(), exitRoomIDs.end(), itRoom.second->GetID()) == exitRoomIDs.end()) { //room with no exit
            continue;
        }
        for (const auto& itSubroom : itRoom.second->GetAllSubRooms()) {
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
            const std::vector<Crossing*>& allCrossings = itSubroom.second->GetAllCrossings();
            for (Crossing* crossPtr : allCrossings) {
                //TODO if (!crossPtr->IsOpen()) {
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
    }

    //all goals
    const std::map<int, Goal*>& allgoals = buildingArg->GetAllGoals();
    for (auto eachgoal:allgoals) {
        for (auto& eachwall:eachgoal.second->GetAllWalls() ) {
            if (eachwall.GetPoint1()._x < xMin) xMin = eachwall.GetPoint1()._x;
            if (eachwall.GetPoint2()._x < xMin) xMin = eachwall.GetPoint2()._x;
            if (eachwall.GetPoint1()._x > xMax) xMax = eachwall.GetPoint1()._x;
            if (eachwall.GetPoint2()._x > xMax) xMax = eachwall.GetPoint2()._x;

            if (eachwall.GetPoint1()._y < yMin) yMin = eachwall.GetPoint1()._y;
            if (eachwall.GetPoint2()._y < yMin) yMin = eachwall.GetPoint2()._y;
            if (eachwall.GetPoint1()._y > yMax) yMax = eachwall.GetPoint1()._y;
            if (eachwall.GetPoint2()._y > yMax) yMax = eachwall.GetPoint2()._y;
        }
        //goalcostmap.emplace(eachgoal.second->GetId(), nullptr);
        //goalneggradmap.emplace(eachgoal.second->GetId(), nullptr);
    }

    //create Rect Grid
    _grid = new RectGrid();
    _grid->setBoundaries(xMin, yMin, xMax, yMax);
    _grid->setSpacing(stepSizeX, stepSizeX);
    _grid->createGrid();

    //create arrays
    _gcode = new int[_grid->GetnPoints()];                  //see Macros.h: enum GridCode {...}
    _subrooms = new SubRoom*[_grid->GetnPoints()]();
    _dist2Wall = new double[_grid->GetnPoints()];
    _speedInitial = new double[_grid->GetnPoints()];
    _modifiedspeed = new double[_grid->GetnPoints()];
    _cost = new double[_grid->GetnPoints()];
    _neggrad = new Point[_grid->GetnPoints()];
    _dirToWall = new Point[_grid->GetnPoints()];

    _costmap.emplace(-1 , _cost);                         // enable default ff (closest exit)
    _neggradmap.emplace(-1, _neggrad);

    //init grid with -3 as unknown distance to any wall
    for(long int i = 0; i < _grid->GetnPoints(); ++i) {
        _dist2Wall[i] = -3.;
        _cost[i] = -2.;
        _gcode[i] = OUTSIDE;            //unknown
        _subrooms[i] = nullptr;
    }
    drawLinesOnGrid<double>(_wall, _dist2Wall, 0.);
    drawLinesOnGrid<double>(_wall, _cost, -7.);
    drawLinesOnGrid<int>(_wall, _gcode, WALL);
    drawLinesOnGrid<int>(_exitsFromScope, _gcode, OPEN_TRANSITION);
}

//this function must only be used BEFORE calculateDistanceField(), because we set trialfield[].cost = dist2Wall AND we init dist2Wall with "-3"
void FloorfieldViaFM::prepareForDistanceFieldCalculation(const bool onlyRoomsWithExits) { //onlyRoomsWithExits means, outside points must be considered
    for (long int i = 0; i < _grid->GetnPoints(); ++i) {

        switch (_gcode[i]) {
            //wall, closed_cross/_trans are all coded with "WALL" after prasing building
            case WALL:
                _speedInitial[i] = .001;
                _cost[i]         = -7.;
                _dist2Wall[i]    = 0.;
                break;
            case CLOSED_CROSSING:
                _speedInitial[i] = .001;
                _cost[i]         = -7.;
                _dist2Wall[i]    = 0.;
                break;
            case CLOSED_TRANSITION:
                _speedInitial[i] = .001;
                _cost[i]         = -7.;
                _dist2Wall[i]    = 0.;
                break;
            //open transitions are marked
            case OPEN_TRANSITION:
                _speedInitial[i] = 1.;
                _cost[i]         = 0.;
                _neggrad[i]._x = (0.);        //must be changed to costarray/neggradarray?
                _neggrad[i]._y = (0.);        //we can leave it, if we agree on cost/neggrad being
                _dirToWall[i]._x = (0.);      //default floorfield using all exits and have the
                _dirToWall[i]._y = (0.);      //array mechanic on top
                break;
            //open crossings are not marked at all after parsing building
            case OPEN_CROSSING:
                _speedInitial[i] = 1.;
                _cost[i]         = -2.;
                break;
            //after parsing, none is INSIDE, but for style reasons, I want switch cases to show all values
            case INSIDE:
                _speedInitial[i] = 1.;
                _cost[i]         = -2.;
                break;
            //this is the main thing in this loop, we want to find and mark inside points (and it is costly!!)
            case OUTSIDE:
            {
                SubRoom* subroom = isInside(i);
                if (subroom || onlyRoomsWithExits) {
                    _speedInitial[i] = 1.;
                    _cost[i] = -2.;
                    _gcode[i] = INSIDE;
                    _subrooms[i] = subroom;
                }
                break;
            }
        } //switch
    } //for loop (points)
    // drawLinesOnGrid(exits, cost, 0.); //already mark targets/exits in cost array (for floorfieldcalc and crossout (LocalFF))
}

void FloorfieldViaFM::deleteAllFFs() {
    for (size_t i = 0; i < _costmap.size(); ++i) {
        auto costIter = _costmap.begin();
        auto negIter  = _neggradmap.begin();
        std::advance(costIter, (_costmap.size() - (i+1)));
        std::advance(negIter,  (_neggradmap.size() - (i+1)));

        if (costIter->second) delete[] costIter->second;
        if (negIter->second) delete[] negIter->second;

        costIter->second = nullptr;
        negIter->second = nullptr;
    }
}

template <typename T>
void FloorfieldViaFM::drawLinesOnGrid(std::vector<Line>& wallArg, T* const target, const T value) { //no init, plz init elsewhere
// i~x; j~y;
//http://stackoverflow.com/questions/10060046/drawing-lines-with-bresenhams-line-algorithm
//src in answer of "Avi"; adapted to fit this application

//    //init with inside value:
//    long int indexMax = grid->GetnPoints();
//    for (long int i = 0; i < indexMax; ++i) {
//        target[i] = inside;
//    }

    //grid handeling local vars:
    long int iMax  = _grid->GetiMax();

    long int iStart, iEnd;
    long int jStart, jEnd;
    long int iDot, jDot;
    long int key;
    long int deltaX, deltaY, deltaX1, deltaY1, px, py, xe, ye, i; //Bresenham Algorithm

    for (auto& line : wallArg) {
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
            if ((_gcode[jDot*iMax + iDot] != WALL) && (_gcode[jDot*iMax + iDot] != CLOSED_CROSSING) && (_gcode[jDot*iMax + iDot] != CLOSED_TRANSITION)) {
                target[jDot * iMax + iDot] = value;
            }
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
                if ((_gcode[jDot*iMax + iDot] != WALL) && (_gcode[jDot*iMax + iDot] != CLOSED_CROSSING) && (_gcode[jDot*iMax + iDot] != CLOSED_TRANSITION)) {
                    target[jDot * iMax + iDot] = value;
                }
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
            if ((_gcode[jDot*iMax + iDot] != WALL) && (_gcode[jDot*iMax + iDot] != CLOSED_CROSSING) && (_gcode[jDot*iMax + iDot] != CLOSED_TRANSITION)) {
                target[jDot * iMax + iDot] = value;
            }
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
                if ((_gcode[jDot*iMax + iDot] != WALL) && (_gcode[jDot*iMax + iDot] != CLOSED_CROSSING) && (_gcode[jDot*iMax + iDot] != CLOSED_TRANSITION)) {
                    target[jDot * iMax + iDot] = value;
                }
            }
        }
    } //loop over all walls

} //drawLinesOnGrid

void FloorfieldViaFM::setSpeed(bool useDistance2WallArg) {
    if (useDistance2WallArg && (_threshold > 0)) {
        double temp;            //needed to only slowdown band of threshold. outside of band modifiedspeed should be 1
        for (long int i = 0; i < _grid->GetnPoints(); ++i) {
            temp = (_dist2Wall[i] < _threshold) ? _dist2Wall[i] : _threshold;
            _modifiedspeed[i] = 0.001 + 0.999 * (temp/_threshold); //linear ramp from wall (0.001) to thresholddistance (1.000)
        }
    } else {
        if (useDistance2WallArg) {     //favor middle of hallways/rooms
            for (long int i = 0; i < _grid->GetnPoints(); ++i) {
                if (_threshold == 0.) {  //special case if user set (use_wall_avoidance = true, wall_avoid_distance = 0.0) and thus wants a plain floorfield
                    _modifiedspeed[i] = _speedInitial[i];
                } else {                //this is the regular case for "favor middle of hallways/rooms
                    _modifiedspeed[i] = 0.001 + 0.999 * (_dist2Wall[i]/10); // @todo: ar.graf  (10 ist ein hardgecodeter wert.. sollte ggf. angepasst werden)
                }
            }
        } else {                    //do not use Distance2Wall
            for (long int i = 0; i < _grid->GetnPoints(); ++i) {
                _modifiedspeed[i] = _speedInitial[i];
            }
        }
    }
    if (_densityspeed) {
        std::copy(_modifiedspeed, _modifiedspeed + _grid->GetnPoints(), _densityspeed);
    }
}

void FloorfieldViaFM::setSpeedThruPeds(Pedestrian* const * pedsArg, int nsize, int modechoice, double radius) {

    long int delta = radius / _grid->Gethx();
    long int posIndex = 0;
    long int pos_i = 0;
    long int pos_j = 0;
    long int i_start = 0;
    long int j_start = 0;
    long int i_end = 0;
    long int j_end = 0;
    double indexDistance = 0.0;

    if (nsize == 0) {
        Log->Write("WARNING: \tSetSpeedThruPeds: nsize is ZERO");
    } else {
        Log->Write("INFO: \t\tNumber of Peds used in setSpeedThruPeds: %d",nsize);
    }

    if ((modechoice == quickest) && (!_densityspeed)) {
        _densityspeed = new double[_grid->GetnPoints()];
    }
    //std::copy(modifiedspeed, modifiedspeed+(grid->GetnPoints()), densityspeed);
    for (long int i = 0; i < _grid->GetnPoints(); ++i) {
        _densityspeed[i] = _speedInitial[i];
    }

    for (int i = 0; i < nsize; ++i) {
        //the following check is not 3D proof, we require the caller of this function to provide a list with "valid"
        //pedestrian-pointer
        if (!_grid->includesPoint(pedsArg[i]->GetPos())) {
            continue;
        }
                                                    /*this value defines the jam-speed threshold*/
//        if (pedsArg[i]->GetEllipse().GetV().Norm() >  0.8*pedsArg[i]->GetEllipse().GetV0()) {
//            continue;
//        }
        posIndex = _grid->getKeyAtPoint(pedsArg[i]->GetPos());
        pos_i = _grid->get_i_fromKey(posIndex);
        pos_j = _grid->get_j_fromKey(posIndex);

        i_start = ((pos_i - delta) < 0)               ? 0               : (pos_i - delta);
        i_end   = ((pos_i + delta) >= _grid->GetiMax()) ? _grid->GetiMax()-1 : (pos_i + delta);

        j_start = ((pos_j - delta) < 0)               ? 0               : (pos_j - delta);
        j_end   = ((pos_j + delta) >= _grid->GetjMax()) ? _grid->GetjMax()-1 : (pos_j + delta);

        for     (long int curr_i = i_start; curr_i < i_end; ++curr_i) {
            for (long int curr_j = j_start; curr_j < j_end; ++curr_j) {
                //indexDistance holds the square
                indexDistance = ( (curr_i - pos_i)*(curr_i - pos_i) + (curr_j - pos_j)*(curr_j - pos_j) );
                //now using indexDistance to store the (speed) reduction value
                //indexDistance = (delta*delta) - (indexDistance);
                //if (indexDistance < 0) { indexDistance = 0.;}
                //scale to [0 .. 1]
                //indexDistance = indexDistance/(delta*delta);

                //densityspeed[curr_j*grid->GetiMax() + curr_i] = (indexDistance > (delta*delta)) ? densityspeed[curr_j*grid->GetiMax() + curr_i] : .001;
                if (indexDistance < (delta*delta)) {
                    //std::cout << "c h a n g i n g   ";
                    _densityspeed[curr_j*_grid->GetiMax() + curr_i] = 0.07;
                }
            }
        }
    }

}

void FloorfieldViaFM::calculateFloorfield(std::vector<Line>& targetlines, double* costarray, Point* neggradarray) {
    calculateFloorfield(targetlines, costarray, neggradarray, _modifiedspeed);
}

void FloorfieldViaFM::calculateFloorfield(std::vector<Line>& targetlines, double* costarray, Point* neggradarray, double* speedarray) {

    std::priority_queue<TrialP, std::vector<TrialP>, std::greater<TrialP>> trialqueue;
    int* flag = new int[_grid->GetnPoints()];

    //re-init memory
    for (long int i = 0; i < _grid->GetnPoints(); ++i) {
        if ((_gcode[i] == INSIDE) || (_gcode[i] == OPEN_TRANSITION) || (_gcode[i] == OPEN_CROSSING)) {
            flag[i]         = FM_UNKNOWN;   //inside
        } else {
            flag[i]         = FM_OUTSIDE;  //wall, outside or closed
        }

        costarray[i] = -7.;
    }
    drawLinesOnGrid<double>(targetlines, costarray, 0.);
    drawLinesOnGrid<int>(targetlines, flag, FM_FINAL);

    //init narrowband
    for (long int i = 0; i < _grid->GetnPoints(); ++i) {
        if (flag[i] == FM_FINAL) {
            TrialP newTrialP = TrialP(i, costarray, speedarray, flag, neggradarray);
            checkNeighborsAndAddToNarrowband(trialqueue, newTrialP,
                                             [&] (TrialP trialPArg) {this->calcFloorfield(trialPArg);});
        }
    }

    // initial narrowband done, now loop (while not empty: remove smallest, add neighbors of removed)
    while (!trialqueue.empty()) {
        long int keyOfSmallest = trialqueue.top().key;
        flag[keyOfSmallest] = FM_FINAL;
        trialqueue.pop();
        TrialP newTrialP = TrialP(keyOfSmallest, costarray, speedarray, flag, neggradarray);
        checkNeighborsAndAddToNarrowband(trialqueue,  newTrialP,
                                         [&] (TrialP trialPArg) { this->calcFloorfield(trialPArg);});
    }
    delete[] flag;
}

void FloorfieldViaFM::calculateDistanceField(const double thresholdArg) {  //if threshold negative, then ignore it

    std::priority_queue<TrialP, std::vector<TrialP>, std::greater<TrialP>> trialqueue;
    int* flag = new int[_grid->GetnPoints()];
#ifdef TESTING
    //sanity check (fields <> 0)
    if (flag == nullptr) return;
    if (_dist2Wall == 0) return;
    if (_speedInitial == 0) return;
    if (_cost == 0) return;
    if (_neggrad == 0) return;
#endif //TESTING

    //re-init memory
    for (long int i = 0; i < _grid->GetnPoints(); ++i) {
        if ((_gcode[i] == INSIDE) || (_gcode[i] == OPEN_TRANSITION) || (_gcode[i] == OPEN_CROSSING)) {
            flag[i]         = FM_UNKNOWN;   //inside
        } else if (_gcode[i] == OUTSIDE) {
            flag[i]         = FM_OUTSIDE;  //outside
        } else {
            flag[i]         = FM_FINAL;   //wall or closed
        }
    }

    for (long int i = 0; i < _grid->GetnPoints(); ++i) {
        //if ((dist2Wall[i] == 0.) && (flag[i] == -7.)) {
        if ((_gcode[i] == WALL) || (_gcode[i] == CLOSED_TRANSITION) || (_gcode[i] == CLOSED_CROSSING)) {
            TrialP newP = TrialP(i, _dist2Wall, _speedInitial, flag, _dirToWall);
            checkNeighborsAndAddToNarrowband(trialqueue, newP,
                                             [&] (TrialP trialPArg) { this->calcDist2Wall(trialPArg);} );
        }
    }

    while (!trialqueue.empty()) {
        long int keyOfSmallest = trialqueue.top().key;
        flag[keyOfSmallest] = FM_FINAL;
        trialqueue.pop();
        if ((thresholdArg > 0) && (_dist2Wall[keyOfSmallest] > thresholdArg)) {    //set rest of nearfield and rest of unknown to this max value:

            //rest of nearfield
            while (!trialqueue.empty()) {
                long int currKey = trialqueue.top().key;
                _dist2Wall[currKey] = _dist2Wall[keyOfSmallest];
                flag[currKey] = FM_FINAL;
                _dirToWall[currKey]._x = (0.);
                _dirToWall[currKey]._y = (0.);
                trialqueue.pop();
            }

            //rest of unknown
            for (long int i = 0; i < _grid->GetnPoints(); ++i) {
                if (flag[i] == FM_UNKNOWN) {
                    flag[i] = FM_FINAL;
                    _dist2Wall[i] = _dist2Wall[keyOfSmallest];
                    _dirToWall[i]._x = (0.);
                    _dirToWall[i]._y = (0.);
                }
            }
        } else {
            //Log->Write(std::to_string(debugcounter++) + " " + std::to_string(grid->GetnPoints()));
            TrialP smallestP = TrialP(keyOfSmallest, _dist2Wall, _speedInitial, flag, _dirToWall);
            checkNeighborsAndAddToNarrowband(trialqueue, smallestP,
                                             [&] (TrialP trialPArg) { this->calcDist2Wall(trialPArg);} );
        }
    }
    delete[] flag;
} //calculateDistancField

void FloorfieldViaFM::checkNeighborsAndAddToNarrowband(std::priority_queue<TrialP, std::vector<TrialP>, std::greater<TrialP>>& trialqueue, TrialP keyP,
                                                       std::function<void (TrialP)> calc) {
    int* flag = keyP.flag;

    directNeighbor dNeigh = _grid->getNeighbors(keyP.key);

    //check for valid neigh
    long int aux = dNeigh.key[0];
    //hint: trialfield[i].cost = dist2Wall + i; <<< set in prepareForDistanceFieldCalc
    if ((aux != -2) && (flag[aux] == FM_UNKNOWN)) {
        flag[aux] = FM_ADDED;      // added to trial but not calculated
        TrialP currNeighP = TrialP(aux, keyP.cost, keyP.speed, flag, keyP.neggrad);
        calc(currNeighP);
        trialqueue.emplace(currNeighP);
    }
    aux = dNeigh.key[1];
    if ((aux != -2) && (flag[aux] == FM_UNKNOWN)) {
        flag[aux] = FM_ADDED;      // added to trial but not calculated
        TrialP currNeighP = TrialP(aux, keyP.cost, keyP.speed, flag, keyP.neggrad);
        calc(currNeighP);
        trialqueue.emplace(currNeighP);
    }
    aux = dNeigh.key[2];
    if ((aux != -2) && (flag[aux] == FM_UNKNOWN)) {
        flag[aux] = FM_ADDED;      // added to trial but not calculated
        TrialP currNeighP = TrialP(aux, keyP.cost, keyP.speed, flag, keyP.neggrad);
        calc(currNeighP);
        trialqueue.emplace(currNeighP);
    }
    aux = dNeigh.key[3];
    if ((aux != -2) && (flag[aux] == FM_UNKNOWN)) {
        flag[aux] = FM_ADDED;      // added to trial but not calculated
        TrialP currNeighP = TrialP(aux, keyP.cost, keyP.speed, flag, keyP.neggrad);
        calc(currNeighP);
        trialqueue.emplace(currNeighP);
    }
}

void FloorfieldViaFM::calcDist2Wall(TrialP newPoint) {
    double row;
    double col;
    long int aux;
    bool pointsUp;
    bool pointsRight;

    int* flag = newPoint.flag;
    double* cost = newPoint.cost;
    long int key = newPoint.key;

    row = 100000.;
    col = 100000.;
    //aux = -1;

    directNeighbor dNeigh = _grid->getNeighbors(key);

    aux = dNeigh.key[0];
    //hint: trialfield[i].cost = dist2Wall + i; <<< set in resetGoalAndCosts
    if  ((aux != -2) &&                                                         //neighbor is a gridpoint
         (flag[aux] != FM_UNKNOWN) &&                                                    //gridpoint holds a calculated value
         (_gcode[aux] != OUTSIDE))                                                     //gridpoint holds a calculated value
    {
        row = cost[aux];
        pointsRight = true;
        if (row < 0) {
            std::cerr << "hier ist was schief " << row << " " << aux << " " << flag[aux] << std::endl;
            row = 100000;
        }
    }
    aux = dNeigh.key[2];
    if  ((aux != -2) &&                                                         //neighbor is a gridpoint
         (flag[aux] != FM_UNKNOWN) &&                                                    //gridpoint holds a calculated value
         (_gcode[aux] != OUTSIDE) &&
         (cost[aux] < row))                                       //calculated value promises smaller cost
    {
        row = cost[aux];
        pointsRight = false;
    }

    aux = dNeigh.key[1];
    //hint: trialfield[i].cost = dist2Wall + i; <<< set in parseBuilding after linescan call
    if  ((aux != -2) &&                                                         //neighbor is a gridpoint
         (flag[aux] != FM_UNKNOWN) &&                                                    //gridpoint holds a calculated value
         (_gcode[aux] != OUTSIDE))
    {
        col = cost[aux];
        pointsUp = true;
        if (col < 0) {
            std::cerr << "hier ist was schief " << col << " " << aux << " " << flag[aux] << std::endl;
            col = 100000;
        }
    }
    aux = dNeigh.key[3];
    if  ((aux != -2) &&                                                         //neighbor is a gridpoint
         (flag[aux] != FM_UNKNOWN) &&                                                    //gridpoint holds a calculated value
         (_gcode[aux] != OUTSIDE) &&
         (cost[aux] < col))                                       //calculated value promises smaller cost
    {
        col = cost[aux];
        pointsUp = false;
    }
    if (col == 100000.) { //one sided update with row
        cost[key] = onesidedCalc(row, _grid->Gethx());
        flag[key] = FM_SINGLE;
        if (pointsRight) {
            _dirToWall[key]._x = (-(cost[key+1]-cost[key])/_grid->Gethx());
            _dirToWall[key]._y = (0.);
        } else {
            _dirToWall[key]._x = (-(cost[key]-cost[key-1])/_grid->Gethx());
            _dirToWall[key]._y = (0.);
        }
        _dirToWall[key] = _dirToWall[key].Normalized(); //@todo: ar.graf: what yields better performance? scale every point here or scale each read value? more points or more calls to any element of dir2Wall
        return;
    }

    if (row == 100000.) { //one sided update with col
        cost[key] = onesidedCalc(col, _grid->Gethy());
        flag[key] = FM_SINGLE;
        if (pointsUp) {
            _dirToWall[key]._x = (0.);
            _dirToWall[key]._y = (-(cost[key+(_grid->GetiMax())]-cost[key])/_grid->Gethy());
        } else {
            _dirToWall[key]._x = (0.);
            _dirToWall[key]._y = (-(cost[key]-cost[key-(_grid->GetiMax())])/_grid->Gethy());
        }
        _dirToWall[key] = _dirToWall[key].Normalized();
        return;
    }

    //two sided update
    double precheck = twosidedCalc(row, col, _grid->Gethx());
    if (precheck >= 0) {
        cost[key] = precheck;
        flag[key] = FM_DOUBLE;
        if (pointsUp && pointsRight) {
            _dirToWall[key]._x = (-(cost[key+1]-cost[key])/_grid->Gethx());
            _dirToWall[key]._y = (-(cost[key+(_grid->GetiMax())]-cost[key])/_grid->Gethy());
        }
        if (pointsUp && !pointsRight) {
            _dirToWall[key]._x = (-(cost[key]-cost[key-1])/_grid->Gethx());
            _dirToWall[key]._y = (-(cost[key+(_grid->GetiMax())]-cost[key])/_grid->Gethy());
        }
        if (!pointsUp && pointsRight) {
            _dirToWall[key]._x = (-(cost[key+1]-cost[key])/_grid->Gethx());
            _dirToWall[key]._y = (-(cost[key]-cost[key-(_grid->GetiMax())])/_grid->Gethy());
        }
        if (!pointsUp && !pointsRight) {
            _dirToWall[key]._x = (-(cost[key]-cost[key-1])/_grid->Gethx());
            _dirToWall[key]._y = (-(cost[key]-cost[key-(_grid->GetiMax())])/_grid->Gethy());
        }
    } else {
        std::cerr << "else in twosided Dist " << std::endl;
    }
    _dirToWall[key] = _dirToWall[key].Normalized();
}

void FloorfieldViaFM::calcFloorfield(TrialP newPoint) {
    double row;
    double col;
    long int aux;
    bool pointsUp = false;
    bool pointsRight = false;

    int* flag = newPoint.flag;
    double* cost = newPoint.cost;
    long int key = newPoint.key;
    double* speed = newPoint.speed;
    Point* neggrad = newPoint.neggrad;

    row = DBL_MAX;
    col = DBL_MAX;
    //aux = -1;

    directNeighbor dNeigh = _grid->getNeighbors(key);

    aux = dNeigh.key[0];
    //hint: trialfield[i].cost = costarray + i; <<< set in calculateFloorfield
    if  ((aux != -2) &&                                                         //neighbor is a gridpoint
         (flag[aux] != FM_UNKNOWN) &&
         //((gcode[aux] == INSIDE) || (gcode[aux] == OPEN_CROSSING) || (gcode[aux] == OPEN_TRANSITION)) )    //gridpoint holds a calculated value
         (flag[aux] != FM_OUTSIDE))
    {
        row = cost[aux];
        pointsRight = true;
    }
    aux = dNeigh.key[2];
    if  ((aux != -2) &&                                                         //neighbor is a gridpoint
         (flag[aux] != FM_UNKNOWN) &&
         (flag[aux] != FM_OUTSIDE) &&                                                 //gridpoint holds a calculated value
         (cost[aux] < row))                                       //calculated value promises smaller cost
    {
        row = cost[aux];
        pointsRight = false;
    }

    aux = dNeigh.key[1];
    //hint: trialfield[i].cost = cost + i; <<< set in calculateFloorfield
    if  ((aux != -2) &&                                                         //neighbor is a gridpoint
         (flag[aux] != FM_UNKNOWN) &&
         (flag[aux] != FM_OUTSIDE))                                                      //gridpoint holds a calculated value
    {
        col = cost[aux];
        pointsUp = true;
    }
    aux = dNeigh.key[3];
    if  ((aux != -2) &&                                                         //neighbor is a gridpoint
         (flag[aux] != FM_UNKNOWN) &&
         (flag[aux] != FM_OUTSIDE) &&                                                  //gridpoint holds a calculated value
         (cost[aux] < col))                                       //calculated value promises smaller cost
    {
        col = cost[aux];
        pointsUp = false;
    }
    if ((col == DBL_MAX) && (row == DBL_MAX)) {
        std::cerr << "Issue 175 in FloorfieldViaFM: invalid combination of row,col (both on max)" <<std::endl;
        return;
    }
    if (col == DBL_MAX) { //one sided update with row
        cost[key] = onesidedCalc(row, _grid->Gethx()/speed[key]);
        flag[key] = FM_SINGLE;
        if (pointsRight && (dNeigh.key[0] != -2)) {
            neggrad[key]._x = (-(cost[key+1]-cost[key])/_grid->Gethx());
            neggrad[key]._y = (0.);
        } else if (dNeigh.key[2] != -2) {
            neggrad[key]._x = (-(cost[key]-cost[key-1])/_grid->Gethx());
            neggrad[key]._y = (0.);
        }
        return;
    }

    if (row == DBL_MAX) { //one sided update with col
        cost[key] = onesidedCalc(col, _grid->Gethy()/speed[key]);
        flag[key] = FM_SINGLE;
        if ((pointsUp) && (dNeigh.key[1] != -2)) {
            neggrad[key]._x = (0.);
            neggrad[key]._y = (-(cost[key+(_grid->GetiMax())]-cost[key])/_grid->Gethy());
        } else if (dNeigh.key[3] != -2){
            neggrad[key]._x = (0.);
            neggrad[key]._y = (-(cost[key]-cost[key-(_grid->GetiMax())])/_grid->Gethy());
        }
        return;
    }

    //two sided update
    double precheck = twosidedCalc(row, col, _grid->Gethx()/speed[key]);
//    if (precheck > 10000) {
//        Log->Write("ERROR \t\t\t is in twosided");
//    }
    if (precheck >= 0) {
        cost[key] = precheck;
        flag[key] = FM_DOUBLE;
        if (pointsUp && pointsRight) {
            neggrad[key]._x = (-(cost[key+1]-cost[key])/_grid->Gethx());
            neggrad[key]._y = (-(cost[key+(_grid->GetiMax())]-cost[key])/_grid->Gethy());
        }
        if (pointsUp && !pointsRight) {
            neggrad[key]._x = (-(cost[key]-cost[key-1])/_grid->Gethx());
            neggrad[key]._y = (-(cost[key+(_grid->GetiMax())]-cost[key])/_grid->Gethy());
        }
        if (!pointsUp && pointsRight) {
            neggrad[key]._x = (-(cost[key+1]-cost[key])/_grid->Gethx());
            neggrad[key]._y = (-(cost[key]-cost[key-(_grid->GetiMax())])/_grid->Gethy());
        }
        if (!pointsUp && !pointsRight) {
            neggrad[key]._x = (-(cost[key]-cost[key-1])/_grid->Gethx());
            neggrad[key]._y = (-(cost[key]-cost[key-(_grid->GetiMax())])/_grid->Gethy());
        }
    } else {
        std::cerr << "else in twosided Floor " << precheck << " " << row << " " << col << std::endl;
    }
}

inline double FloorfieldViaFM::onesidedCalc(double xy, double hDivF) {
    //if ( (xy+hDivF) > 10000) std::cerr << "error in onesided " << xy << std::endl;
    return xy + hDivF;
}

inline double FloorfieldViaFM::twosidedCalc(double x, double y, double hDivF) { //on error return -2
    double determinante = (2*hDivF*hDivF - (x-y)*(x-y));
    if (determinante >= 0) {
        return (x + y + sqrt(determinante))/2;
    } else {
        return (x < y) ? (x + hDivF) : (y + hDivF);
    }
    std::cerr << "error in two-sided 2!!!!!!!!!!!!!!!!!!!!!!! o_O??" << std::endl;
    return -2.; //this line should never execute
} //twosidedCalc

void FloorfieldViaFM::testoutput(const char* filename1, const char* filename2, const double* target) {
//security debug check
    std::ofstream file;
    std::ofstream file2;
    int numX = (int) ((_grid->GetxMax()-_grid->GetxMin())/_grid->Gethx());
    int numY = (int) ((_grid->GetyMax()-_grid->GetyMin())/_grid->Gethy());
    int numTotal = numX * numY;
    //std::cerr << numTotal << " numTotal" << std::endl;
    //std::cerr << grid->GetnPoints() << " grid" << std::endl;
    file.open(filename1);
    file2.open(filename2);
    file << "# vtk DataFile Version 3.0" << std::endl;
    file << "Testdata: Fast Marching: Test: " << std::endl;
    file << "ASCII" << std::endl;
    file << "DATASET STRUCTURED_POINTS" << std::endl;
    file << "DIMENSIONS " <<
                                std::to_string(_grid->GetiMax()) <<
                                " " <<
                                std::to_string(_grid->GetjMax()) <<
                                " 1" << std::endl;
    file << "ORIGIN " << _grid->GetxMin() << " " << _grid->GetyMin() << " 0" << std::endl;
    file << "SPACING " << std::to_string(_grid->Gethx()) << " " << std::to_string(_grid->Gethy()) << " 1" << std::endl;
    file << "POINT_DATA " << std::to_string(numTotal) << std::endl;
    file << "SCALARS Cost float 1" << std::endl;
    file << "LOOKUP_TABLE default" << std::endl;
    for (long int i = 0; i < _grid->GetnPoints(); ++i) {
        file << target[i] << std::endl;
        Point iPoint = _grid->getPointFromKey(i);
        file2 << iPoint._x /*- grid->GetxMin()*/ << " " << iPoint._y /*- grid->GetyMin()*/ << " " << target[i] << std::endl;
    }

    if (target == _cost) {
        file << "VECTORS Gradient float" << std::endl;
        for (int i = 0; i < _grid->GetnPoints(); ++i) {
            file << _neggrad[i]._x << " " << _neggrad[i]._y << " 0.0" << std::endl;
        }
    }

    file.close();
    file2.close();

    //std::cerr << "INFO: \tFile closed: " << filename1 << std::endl;
}

void FloorfieldViaFM::writeFF(const std::string& filename, std::vector<int> targetID) {
    Log->Write("INFO: \tWrite Floorfield to file");
    Log->Write(filename);
    std::ofstream file;

    Log->Write("FloorfieldViaFM::writeFF(): writing to file %s: There are %d targets.", filename.c_str(), targetID.size());

    int numX = (int) ((_grid->GetxMax()-_grid->GetxMin())/_grid->Gethx());
    int numY = (int) ((_grid->GetyMax()-_grid->GetyMin())/_grid->Gethy());
    int numTotal = numX * numY;
    //std::cerr << numTotal << " numTotal" << std::endl;
    //std::cerr << grid->GetnPoints() << " grid" << std::endl;
    file.open(filename);

    file << "# vtk DataFile Version 3.0" << std::endl;
    file << "Testdata: Fast Marching: Test: " << std::endl;
    file << "ASCII" << std::endl;
    file << "DATASET STRUCTURED_POINTS" << std::endl;
    file << "DIMENSIONS " <<
                                std::to_string(_grid->GetiMax()) <<
                                " " <<
                                std::to_string(_grid->GetjMax()) <<
                                " 1" << std::endl;
    file << "ORIGIN " << _grid->GetxMin() << " " << _grid->GetyMin() << " 0" << std::endl;
    file << "SPACING " << std::to_string(_grid->Gethx()) << " " << std::to_string(_grid->Gethy()) << " 1" << std::endl;
    file << "POINT_DATA " << std::to_string(numTotal) << std::endl;
    file << "SCALARS Dist2Wall float 1" << std::endl;
    file << "LOOKUP_TABLE default" << std::endl;
    for (long int i = 0; i < _grid->GetnPoints(); ++i) {
        file << _dist2Wall[i] << std::endl; //@todo: change target to all dist2wall
        //Point iPoint = grid->getPointFromKey(i);
        //file2 << iPoint._x /*- grid->GetxMin()*/ << " " << iPoint._y /*- grid->GetyMin()*/ << " " << target[i] << std::endl;
    }

    file << "VECTORS Dir2Wall float" << std::endl;
    for (long int i = 0; i < _grid->GetnPoints(); ++i) {
        file << _dirToWall[i]._x << " " << _dirToWall[i]._y << " 0.0" << std::endl;
    }

    file << "SCALARS SubroomPtr float 1" << std::endl;
    file << "LOOKUP_TABLE default" << std::endl;
    for (long int i = 0; i < _grid->GetnPoints(); ++i) {
        if (_subrooms[i]) {
            file << _subrooms[i]->GetUID() << std::endl;
        } else {
            file << 0.0 << std::endl;
        }
    }

    for (unsigned int iTarget = 0; iTarget < targetID.size(); ++iTarget) {
        Log->Write("%s: target number %d: UID %d", filename.c_str(), iTarget, targetID[iTarget]);
        if (_neggradmap.count(targetID[iTarget]) == 0) {
            continue;
        }

        Point *gradarray = _neggradmap[targetID[iTarget]];
        if (gradarray == nullptr) {
            continue;
        }

        std::string name = _building->GetTransOrCrossByUID(targetID[iTarget])->GetCaption() + "-" + std::to_string(targetID[iTarget]);
        std::replace(name.begin(), name.end(), ' ', '_');
        file << "VECTORS GradientTarget" << name << " float" << std::endl;
        for (int i = 0; i < _grid->GetnPoints(); ++i) {
            file << gradarray[i]._x << " " << gradarray[i]._y << " 0.0" << std::endl;
        }

        double *costarray = _costmap[targetID[iTarget]];
        file << "SCALARS CostTarget" << name << " float 1" << std::endl;
        file << "LOOKUP_TABLE default" << std::endl;
        for (long int i = 0; i < _grid->GetnPoints(); ++i) {
            file << costarray[i] << std::endl;
        }
    }
    file << "SCALARS GCode float 1" << std::endl;
    file << "LOOKUP_TABLE default" << std::endl;
    for (long int i = 0; i < _grid->GetnPoints(); ++i) {
        file << _gcode[i] << std::endl;
    }
    file.close();
}

void FloorfieldViaFM::writeGoalFF(const std::string& filename, std::vector<int> targetID) {
    Log->Write("INFO: \tWrite Floorfield to file");
    Log->Write(filename);
    std::ofstream file;

    int numX = (int) ((_grid->GetxMax()-_grid->GetxMin())/_grid->Gethx());
    int numY = (int) ((_grid->GetyMax()-_grid->GetyMin())/_grid->Gethy());
    int numTotal = numX * numY;
    //std::cerr << numTotal << " numTotal" << std::endl;
    //std::cerr << grid->GetnPoints() << " grid" << std::endl;
    file.open(filename);

    file << "# vtk DataFile Version 3.0" << std::endl;
    file << "Testdata: Fast Marching: Test: " << std::endl;
    file << "ASCII" << std::endl;
    file << "DATASET STRUCTURED_POINTS" << std::endl;
    file << "DIMENSIONS " <<
    std::to_string(_grid->GetiMax()) <<
    " " <<
    std::to_string(_grid->GetjMax()) <<
    " 1" << std::endl;
    file << "ORIGIN " << _grid->GetxMin() << " " << _grid->GetyMin() << " 0" << std::endl;
    file << "SPACING " << std::to_string(_grid->Gethx()) << " " << std::to_string(_grid->Gethy()) << " 1" << std::endl;
    file << "POINT_DATA " << std::to_string(numTotal) << std::endl;
    //file << "SCALARS Dist2Wall float 1" << std::endl;
    //file << "LOOKUP_TABLE default" << std::endl;
    //for (long int i = 0; i < grid->GetnPoints(); ++i) {
    //    file << dist2Wall[i] << std::endl; //@todo: change target to all dist2wall
        //Point iPoint = grid->getPointFromKey(i);
        //file2 << iPoint._x /*- grid->GetxMin()*/ << " " << iPoint._y /*- grid->GetyMin()*/ << " " << target[i] << std::endl;
    //}

    //file << "VECTORS Dir2Wall float" << std::endl;
    //for (long int i = 0; i < grid->GetnPoints(); ++i) {
    //    file << dirToWall[i]._x << " " << dirToWall[i]._y << " 0.0" << std::endl;
    //}

    for (unsigned int iTarget = 0; iTarget < targetID.size(); ++iTarget) {
        if (_goalneggradmap.count(targetID[iTarget]) == 0) {
            continue;
        }

        Point *gradarray = _goalneggradmap[targetID[iTarget]];
        if (gradarray == nullptr) {
            continue;
        }

        file << "VECTORS GradientTarget" << targetID[iTarget] << " float" << std::endl;
        for (int i = 0; i < _grid->GetnPoints(); ++i) {
            file << gradarray[i]._x << " " << gradarray[i]._y << " 0.0" << std::endl;
        }

        double *costarray = _goalcostmap[targetID[iTarget]];
        file << "SCALARS CostTarget" << targetID[iTarget] << " float 1" << std::endl;
        file << "LOOKUP_TABLE default" << std::endl;
        for (long int i = 0; i < _grid->GetnPoints(); ++i) {
            file << costarray[i] << std::endl;
        }
    }
    file << "SCALARS GCode float 1" << std::endl;
    file << "LOOKUP_TABLE default" << std::endl;
    for (long int i = 0; i < _grid->GetnPoints(); ++i) {
        file << _gcode[i] << std::endl;
    }
    file.close();
}

SubRoom* FloorfieldViaFM::isInside(const long int /*key*/) {
//    Point probe = _grid->getPointFromKey(key);

//    const std::map<int, std::shared_ptr<Room>>& roomMap = _building->GetAllRooms();
//
//    for (auto& roomPair : roomMap) {
//
//        Room* roomPtr = roomPair.second.get();
//        const std::map<int, std::shared_ptr<SubRoom>>& subRoomMap = roomPtr->GetAllSubRooms();
//
//        for (auto& subRoomPair : subRoomMap) {
//
//            SubRoom* subRoomPtr = subRoomPair.second.get();
//
//            if (subRoomPtr->IsInSubRoom(probe)) {
//                return subRoomPtr;
//            }
//        }

    //FloorfieldViaFM does not support isInside because of unresolved problem of ambiguous projection of xy plane in multi-
    //storage buildings

    return nullptr;
}

void CentrePointFFViaFM::getDirectionToUID(int destID, const long int key, Point& direction, int mode) {
    //what if goal == -1, meaning closest exit... is GetExitIndex then -1? NO... ExitIndex is UID, given by router
    //if (ped->GetFinalDestination() == -1) /*go to closest exit*/ destID != -1;

    if ((key < 0) || (key >= _grid->GetnPoints())) { // @todo: ar.graf: this check in a #ifdef-block?
        Log->Write("ERROR: \t Floorfield tried to access a key out of grid!");
        direction._x = 0.;
        direction._y = 0.;
        return;
    }
    Point* localneggradptr = nullptr;
    double* localcostptr = nullptr;
    {
        if (_neggradmap.count(destID) == 0) {
            //Log->Write("FF for destID %d does not exist (key is %d)", destID, key);
            //check, if distID is in this grid
            Hline* destLine = _building->GetTransOrCrossByUID(destID);
            Point A = destLine->GetCentre();
            //Point B = destLine->GetPoint2();
            if (!(_grid->includesPoint(A))) {
                Log->Write("ERROR: \t Destination ID %d is not in grid!", destID);
                direction._x = direction._y = 0.;
                //return;
            }
        }
        localneggradptr = (_neggradmap.count(destID) == 0) ? nullptr : _neggradmap.at(destID);
        localcostptr = (_costmap.count(destID) == 0) ? nullptr : _costmap.at(destID);
        if (localneggradptr == nullptr) {
            bool isBeingCalculated;
#pragma omp critical(floorfieldsBeingCalculated)
            {
                if (!(isBeingCalculated = _floorfieldsBeingCalculated.count(destID) > 0)) {
                    _floorfieldsBeingCalculated.insert(destID);
                }
            }
            if (isBeingCalculated) {
                // we do not want to wait until the other calculation has finished, so we return immediately
                // the values are corrected in getDirectionToDestination(), and getCostToDestination doesn't care about the direction
                direction._x = DBL_MAX;
                direction._y = DBL_MAX;
                return;
            }

            //create floorfield (remove mapentry with nullptr, allocate memory, add mapentry, create ff)
            localcostptr =    new double[_grid->GetnPoints()];
            localneggradptr = new Point[_grid->GetnPoints()];
#pragma omp critical(neggradmap)
            _neggradmap.erase(destID);
#pragma omp critical(neggradmap)
            _neggradmap.emplace(destID, localneggradptr);
#pragma omp critical(costmap)
            _costmap.erase(destID);
#pragma omp critical(costmap)
            _costmap.emplace(destID, localcostptr);


            //create ff (prepare Trial-mechanic, then calc)
//                for (long int i = 0; i < grid->GetnPoints(); ++i) {
//                    //set Trialptr to fieldelements
//                    trialfield[i].cost = localcostptr + i;
//                    trialfield[i].neggrad = localneggradptr + i;
//                    trialfield[i].father = nullptr;
//                    trialfield[i].child = nullptr;
//                }
//                clearAndPrepareForFloorfieldReCalc(localcostptr);
            //std::vector<Line> localline = {Line((Line) *(building->GetTransOrCrossByUID(destID)))};
            Point centre = _building->GetTransOrCrossByUID(destID)->GetCentre();
            std::vector<Line> localline = {Line(centre, centre, 0)}; // only one point
//                setNewGoalAfterTheClear(localcostptr, localline);
            //Log->Write("Starting FF for UID %d (ID %d)", destID, dynamic_cast<Crossing*>(building->GetTransOrCrossByUID(destID))->GetID());
            //std::cerr << "\rW\tO\tR\tK\tI\tN\tG";
            if (mode == quickest) {
                calculateFloorfield(localline, localcostptr, localneggradptr, _densityspeed);
            } else {
                calculateFloorfield(localline, localcostptr, localneggradptr, _modifiedspeed);
            }
             //when using CentralPoint, the rest of destID Line would not be calculated. We set that line to zero in the lines below.
             //this way, the ffrouter understands, if a pedestrian reached the line, but is next to central point.
             Point a = _building->GetTransOrCrossByUID(destID)->GetPoint1();
             Point b = _building->GetTransOrCrossByUID(destID)->GetPoint2();
             localline.emplace_back(Line(a, b, 0));
             drawLinesOnGrid<double>(localline, localcostptr, 0.);
#pragma omp critical(floorfieldsBeingCalculated)
            {
                if (_floorfieldsBeingCalculated.count(destID) != 1) {
                    Log->Write("ERROR: FloorfieldViaFM::getDirectionToUID: key %d was calculating FF for destID %d, but it was removed from floorfieldsBeingCalculated meanwhile", key, destID);
                }
                _floorfieldsBeingCalculated.erase(destID);
            }
            //Log->Write("Ending   FF for UID %d", destID);
            //std::cerr << "\r W\t O\t R\t K\t I\t N\t G";
        }
    }
    direction._x = (localneggradptr[key]._x);
    direction._y = (localneggradptr[key]._y);
}

SubRoom* FloorfieldViaFM::GetSubroom(Pedestrian* ped) {
    long int key = _grid->getKeyAtPoint(ped->GetPos());
    assert(key >= 0 && key <= _grid->GetnPoints());
    return _subrooms[key];
}