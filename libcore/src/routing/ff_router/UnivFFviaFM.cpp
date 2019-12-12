//
// Created by arne on 5/9/17.
//
#include "UnivFFviaFM.h"

#include "general/Filesystem.h"
#include "general/Format.h"
#include "general/Logger.h"
#include "geometry/Building.h"
#include "geometry/Line.h"
#include "geometry/SubRoom.h"
#include "geometry/Wall.h"
#include "pedestrian/Pedestrian.h"
#include "routing/ff_router/mesh/RectGrid.h"

#include <unordered_set>

UnivFFviaFM::~UnivFFviaFM()
{
    delete _grid;
    //size_t speedsize = _speedFieldSelector.size();
    for(auto speedPtr : _speedFieldSelector) {
        delete[] speedPtr;
    }
    delete[] _gridCode;
    delete[] _subrooms;

    std::map<int, double *>::reverse_iterator delIterCost;

    for(delIterCost = _costFieldWithKey.rbegin(); delIterCost != _costFieldWithKey.rend();
        ++delIterCost) {
        delete[] delIterCost->second;
    }
    std::map<int, Point *>::reverse_iterator delIterDir;
    for(delIterDir = _directionFieldWithKey.rbegin(); delIterDir != _directionFieldWithKey.rend();
        ++delIterDir) {
        delete[] delIterDir->second;
    }
}

UnivFFviaFM::UnivFFviaFM(
    Room * r,
    Building * b,
    double hx,
    double wallAvoid,
    bool useWallDistances) :
    UnivFFviaFM(r, b->GetConfig(), hx, wallAvoid, useWallDistances)
{
    _building = b;
}

UnivFFviaFM::UnivFFviaFM(
    SubRoom * sr,
    Building * b,
    double hx,
    double wallAvoid,
    bool useWallDistances) :
    UnivFFviaFM(sr, b->GetConfig(), hx, wallAvoid, useWallDistances)
{
    _building = b;
}

UnivFFviaFM::UnivFFviaFM(
    Room * r,
    Configuration * const conf,
    double hx,
    double wallAvoid,
    bool useWallDistances) :
    UnivFFviaFM(r, conf, hx, wallAvoid, useWallDistances, std::vector<int>())
{
}

UnivFFviaFM::UnivFFviaFM(
    Room * roomArg,
    Configuration * const confArg,
    double hx,
    double wallAvoid,
    bool useWallDistances,
    const std::vector<int> & wantedDoors)
{
    //build the vector with walls(wall or obstacle), the map with <UID, Door(Cross or Trans)>, the vector with targets(UIDs)
    //then call other constructor including the mode
    _configuration = confArg;
    _room          = roomArg->GetID();
    std::vector<Line> walls;
    std::map<int, Line> doors;

    for(auto [_, subroom] : roomArg->GetAllSubRooms()) {
        for(auto wall : subroom->GetAllWalls()) {
            walls.emplace_back((Line) wall);
        }

        for(Obstacle * ptrObs : subroom->GetAllObstacles()) {
            for(auto owall : ptrObs->GetAllWalls()) {
                walls.emplace_back((Line) owall);
            }
        }

        for(auto const & cross : subroom->GetAllCrossings()) {
            int id = cross->GetUniqueID();
            if(doors.count(id) == 0) {
                doors.emplace(std::make_pair(id, Line{*cross}));
            }
        }
        for(auto const & trans : subroom->GetAllTransitions()) {
            int id = trans->GetUniqueID();
            if(doors.count(id) == 0) {
                doors.emplace(std::make_pair(id, Line{*trans}));
            }
        }

        //find insidePoint and save it, together with UID
        Point normalVec   = doors.begin()->second.NormalVec();
        Point midPoint    = doors.begin()->second.GetCentre();
        Point candidate01 = midPoint + (normalVec * 0.25);
        Point candidate02 = midPoint - (normalVec * 0.25);

        if(subroom->IsInSubRoom(candidate01)) {
            _subRoomPtrTOinsidePoint.emplace(std::make_pair(subroom.get(), candidate01));
        } else {
            //candidate = candidate - (normalVec * 0.25);
            if(subroom->IsInSubRoom(candidate02)) {
                _subRoomPtrTOinsidePoint.emplace(std::make_pair(subroom.get(), candidate02));
            } else {
                Logging::Error("In UnivFF InsidePoint Analysis.");
            }
        }
    }

    //this will interpret "useWallDistances" as best as possible. Users should clearify with "SetSpeedMode" before calling "AddTarget"
    if(useWallDistances) {
        Create(walls, doors, wantedDoors, FF_WALL_AVOID, hx, wallAvoid, useWallDistances);
    } else {
        Create(walls, doors, wantedDoors, FF_HOMO_SPEED, hx, wallAvoid, useWallDistances);
    }
}

UnivFFviaFM::UnivFFviaFM(
    SubRoom * sr,
    Configuration * const conf,
    double hx,
    double wallAvoid,
    bool useWallDistances) :
    UnivFFviaFM(sr, conf, hx, wallAvoid, useWallDistances, std::vector<int>())
{
}

UnivFFviaFM::UnivFFviaFM(
    SubRoom * subRoomArg,
    Configuration * const confArg,
    double hx,
    double wallAvoid,
    bool useWallDistances,
    const std::vector<int> & wantedDoors)
{
    //build the vector with walls(wall or obstacle), the map with <UID, Door(Cross or Trans)>, the vector with targets(UIDs)
    //then call other constructor including the mode
    _configuration = confArg;
    _room          = subRoomArg->GetRoomID();
    std::vector<Line> lines;
    std::map<int, Line> tmpDoors;

    std::vector<Wall> walls = std::vector<Wall>(subRoomArg->GetAllWalls());
    for(auto & wall : walls) {
        lines.emplace_back((Line) wall);
    }

    std::vector<Obstacle *> tmpObsPtrVec = subRoomArg->GetAllObstacles();
    for(Obstacle * ptrObs : tmpObsPtrVec) {
        const std::vector<Wall> obsWalls = ptrObs->GetAllWalls();
        for(auto & owall : obsWalls) {
            lines.emplace_back((Line) owall);
        }
    }

    const std::vector<Crossing *> tmpCross   = subRoomArg->GetAllCrossings();
    const std::vector<Transition *> tmpTrans = subRoomArg->GetAllTransitions();

    int uidNotConst = 0;
    bool isOpen     = false;
    for(auto & cross : tmpCross) {
        uidNotConst = cross->GetUniqueID();
        //TODO isOpen = cross->IsOpen();
        isOpen = !cross->IsClose();
        if(!isOpen) {
            lines.emplace_back((Line) *cross);
        } else {
            tmpDoors.emplace(std::make_pair(uidNotConst, (Line) *cross));
        }
    }
    for(auto & trans : tmpTrans) {
        uidNotConst = trans->GetUniqueID();
        //TODO  isOpen = trans->IsOpen();
        isOpen = !trans->IsClose();
        if(!isOpen) {
            lines.emplace_back((Line) *trans);
        } else {
            tmpDoors.emplace(std::make_pair(uidNotConst, (Line) *trans));
        }
    }

    //find insidePoint and save it, together with UID
    Line anyDoor      = Line{(--tmpDoors.end())->second};
    Point normalVec   = anyDoor.NormalVec();
    Point midPoint    = anyDoor.GetCentre();
    Point candidate01 = midPoint + (normalVec * 0.25);
    Point candidate02 = midPoint - (normalVec * 0.25);
    if(subRoomArg->IsInSubRoom(candidate01)) {
        _subRoomPtrTOinsidePoint.emplace(std::make_pair(subRoomArg, candidate01));
    } else {
        //candidate = candidate - (normalVec * 0.25);
        if(subRoomArg->IsInSubRoom(candidate02)) {
            _subRoomPtrTOinsidePoint.emplace(std::make_pair(subRoomArg, candidate02));
        } else {
            Logging::Error("In UnivFF InsidePoint Analysis.");
            bool a = subRoomArg->IsInSubRoom(candidate01);
            bool b = subRoomArg->IsInSubRoom(candidate02);
            a      = b && a;
        }
    }

    //_subroomUIDtoSubRoomPtr.emplace(std::make_pair(subRoomArg->GetUID(), subRoomArg));

    //this will interpret "useWallDistances" as best as possible. Users should clearify with "SetSpeedMode" before calling "AddTarget"
    if(useWallDistances) {
        Create(lines, tmpDoors, wantedDoors, FF_WALL_AVOID, hx, wallAvoid, useWallDistances);
    } else {
        Create(lines, tmpDoors, wantedDoors, FF_HOMO_SPEED, hx, wallAvoid, useWallDistances);
    }
}

void UnivFFviaFM::Create(
    std::vector<Line> & walls,
    std::map<int, Line> & doors,
    const std::vector<int> & targetUIDs,
    int mode,
    double spacing,
    double wallAvoidDist,
    bool useWallDistances)
{
    _wallAvoidDistance = wallAvoidDist;
    _useWallDistances  = useWallDistances;
    _speedmode         = mode;

    //find circumscribing rectangle (x_min/max, y_min/max) //Create RectGrid
    CreateRectGrid(walls, doors, spacing);
    _nPoints = _grid->GetnPoints();

    //allocate _gridCode and  _speedFieldSelector and initialize them ("draw" walls and doors)
    _gridCode = new int[_nPoints];
    ProcessGeometry(walls, doors);
    _speedFieldSelector.emplace(_speedFieldSelector.begin() + INITIAL_SPEED, new double[_nPoints]);
    std::fill(
        _speedFieldSelector[INITIAL_SPEED], _speedFieldSelector[INITIAL_SPEED] + _nPoints, 1.0);

    _speedFieldSelector.emplace(_speedFieldSelector.begin() + REDU_WALL_SPEED, nullptr);
    _speedFieldSelector.emplace(_speedFieldSelector.begin() + PED_SPEED, nullptr);

    //mark Inside areas (dont write outside areas)
    for(auto subRoomPointPair : _subRoomPtrTOinsidePoint) {
        MarkSubroom(subRoomPointPair.second, subRoomPointPair.first);
    }

    //allocate _modifiedSpeed
    if((_speedmode == FF_WALL_AVOID) || (useWallDistances)) {
        auto * cost_alias_walldistance      = new double[_nPoints];
        _costFieldWithKey[0]                = cost_alias_walldistance;
        auto * gradient_alias_walldirection = new Point[_nPoints];
        _directionFieldWithKey[0]           = gradient_alias_walldirection;

        //Create wall distance field
        //init costarray
        for(long i = 0; i < _nPoints; ++i) {
            if(_gridCode[i] == WALL) {
                cost_alias_walldistance[i] = magicnum(WALL_ON_COSTARRAY);
            } else {
                cost_alias_walldistance[i] = magicnum(UNKNOWN_COST);
            }
        }
        DrawLinesOnWall(walls, cost_alias_walldistance, magicnum(TARGET_REGION));
        CalcDF(
            cost_alias_walldistance,
            gradient_alias_walldirection,
            _speedFieldSelector[INITIAL_SPEED]);
        //_uids.emplace_back(0);

        //TODO warum wird das gemacht?
        auto * temp_reduWallSpeed = new double[_nPoints];
        if(_speedFieldSelector[REDU_WALL_SPEED]) { //free memory before overwriting
            delete[] _speedFieldSelector[REDU_WALL_SPEED];
        }
        _speedFieldSelector[REDU_WALL_SPEED] = temp_reduWallSpeed;
        //init _reducedWallSpeed by using distance field
        CreateReduWallSpeed(temp_reduWallSpeed);
    }

    // parallel call
    if(!targetUIDs.empty()) {
        AddTargetsParallel(targetUIDs);
    }
}

void UnivFFviaFM::CreateRectGrid(
    std::vector<Line> & walls,
    std::map<int, Line> & doors,
    double spacing)
{
    double x_min = std::numeric_limits<double>::max();
    double x_max = std::numeric_limits<double>::min();
    double y_min = std::numeric_limits<double>::max();
    double y_max = std::numeric_limits<double>::min();

    for(auto & wall : walls) {
        if(wall.GetPoint1()._x < x_min)
            x_min = wall.GetPoint1()._x;
        if(wall.GetPoint1()._y < y_min)
            y_min = wall.GetPoint1()._y;
        if(wall.GetPoint2()._x < x_min)
            x_min = wall.GetPoint2()._x;
        if(wall.GetPoint2()._y < y_min)
            y_min = wall.GetPoint2()._y;

        if(wall.GetPoint1()._x > x_max)
            x_max = wall.GetPoint1()._x;
        if(wall.GetPoint1()._y > y_max)
            y_max = wall.GetPoint1()._y;
        if(wall.GetPoint2()._x > x_max)
            x_max = wall.GetPoint2()._x;
        if(wall.GetPoint2()._y > y_max)
            y_max = wall.GetPoint2()._y;
    }

    for(auto & doorPair : doors) {
        Line & door = doorPair.second;
        if(door.GetPoint1()._x < x_min)
            x_min = door.GetPoint1()._x;
        if(door.GetPoint1()._y < y_min)
            y_min = door.GetPoint1()._y;
        if(door.GetPoint2()._x < x_min)
            x_min = door.GetPoint2()._x;
        if(door.GetPoint2()._y < y_min)
            y_min = door.GetPoint2()._y;

        if(door.GetPoint1()._x > x_max)
            x_max = door.GetPoint1()._x;
        if(door.GetPoint1()._y > y_max)
            y_max = door.GetPoint1()._y;
        if(door.GetPoint2()._x > x_max)
            x_max = door.GetPoint2()._x;
        if(door.GetPoint2()._y > y_max)
            y_max = door.GetPoint2()._y;
    }

    x_min -= 0.5;
    x_max += 0.5;
    y_min -= 0.5;
    y_max += 0.5;

    //Create Rect Grid
    _grid = new RectGrid();
    _grid->SetBoundaries(x_min, y_min, x_max, y_max);
    _grid->SetSpacing(spacing, spacing);
    _grid->CreateGrid();
}

void UnivFFviaFM::ProcessGeometry(std::vector<Line> & walls, std::map<int, Line> & doors)
{
    for(long i = 0; i < _nPoints; ++i) {
        _gridCode[i] = OUTSIDE;
    }

    for(auto mapentry : doors) {
        _doors.insert(mapentry);
    }
    //_doors = doors;

    DrawLinesOnGrid<int>(walls, _gridCode, WALL);
    DrawLinesOnGrid(doors, _gridCode); //UIDs of doors will be drawn on _gridCode
}

void UnivFFviaFM::MarkSubroom(const Point & insidePoint, SubRoom * value)
{
    //value must not be nullptr. it would lead to infinite loop
    if(!value)
        return;

    if(!_grid->IncludesPoint(insidePoint))
        return;

    //alloc mem if needed
    if(!_subrooms) {
        _subrooms = new SubRoom *[_nPoints];
        for(long i = 0; i < _nPoints; ++i) {
            _subrooms[i] = nullptr;
        }
    }

    //init start
    _subrooms[_grid->GetKeyAtPoint(insidePoint)] = value;
    _gridCode[_grid->GetKeyAtPoint(insidePoint)] = INSIDE;

    std::unordered_set<long> wavefront;
    wavefront.reserve(_nPoints);

    directNeighbor _neigh = _grid->GetNeighbors(_grid->GetKeyAtPoint(insidePoint));
    long aux              = _neigh.key[0];
    if((aux != -2) && (_gridCode[aux] == INSIDE || _gridCode[aux] == OUTSIDE)) {
        wavefront.insert(aux);
        _subrooms[aux] = value;
    }

    aux = _neigh.key[1];
    if((aux != -2) && (_gridCode[aux] == INSIDE || _gridCode[aux] == OUTSIDE)) {
        wavefront.insert(aux);
        _subrooms[aux] = value;
    }

    aux = _neigh.key[2];
    if((aux != -2) && (_gridCode[aux] == INSIDE || _gridCode[aux] == OUTSIDE)) {
        wavefront.insert(aux);
        _subrooms[aux] = value;
    }

    aux = _neigh.key[3];
    if((aux != -2) && (_gridCode[aux] == INSIDE || _gridCode[aux] == OUTSIDE)) {
        wavefront.insert(aux);
        _subrooms[aux] = value;
    }

    while(!wavefront.empty()) {
        long current = *(wavefront.begin());
        wavefront.erase(current);
        _gridCode[current] = INSIDE;

        _neigh = _grid->GetNeighbors(current);
        aux    = _neigh.key[0];
        if((aux != -2) && (_gridCode[aux] == INSIDE || _gridCode[aux] == OUTSIDE) &&
           _subrooms[aux] == nullptr) {
            wavefront.insert(aux);
            _subrooms[aux] = value;
        }

        aux = _neigh.key[1];
        if((aux != -2) && (_gridCode[aux] == INSIDE || _gridCode[aux] == OUTSIDE) &&
           _subrooms[aux] == nullptr) {
            wavefront.insert(aux);
            _subrooms[aux] = value;
        }

        aux = _neigh.key[2];
        if((aux != -2) && (_gridCode[aux] == INSIDE || _gridCode[aux] == OUTSIDE) &&
           _subrooms[aux] == nullptr) {
            wavefront.insert(aux);
            _subrooms[aux] = value;
        }

        aux = _neigh.key[3];
        if((aux != -2) && (_gridCode[aux] == INSIDE || _gridCode[aux] == OUTSIDE) &&
           _subrooms[aux] == nullptr) {
            wavefront.insert(aux);
            _subrooms[aux] = value;
        }
    }
}

void UnivFFviaFM::CreateReduWallSpeed(double * reduWallSpeed)
{
    double factor         = 1 / _wallAvoidDistance;
    double * wallDstAlias = _costFieldWithKey[0];

    for(long int i = 0; i < _nPoints; ++i) {
        if(wallDstAlias[i] > 0.) {
            reduWallSpeed[i] =
                (wallDstAlias[i] > _wallAvoidDistance) ? 1.0 : (factor * wallDstAlias[i]);
        }
    }
}

void UnivFFviaFM::FinalizeTargetLine(
    int uid,
    const Line & tempTargetLine,
    Point * newArrayPt,
    Point & passvector)
{
    // i~x; j~y;
    //http://stackoverflow.com/questions/10060046/drawing-lines-with-bresenhams-line-algorithm
    //src in answer of "Avi"; adapted to fit this application

    //grid handeling local vars:
    long int iMax = _grid->GetiMax();

    long int iStart, iEnd;
    long int jStart, jEnd;
    long int iDot, jDot;
    long int key;
    long int deltaX, deltaY, deltaX1, deltaY1, px, py, xe, ye, i; //Bresenham Algorithm

    //long int goodneighbor;
    //directNeighbor neigh;


    key    = _grid->GetKeyAtPoint(tempTargetLine.GetPoint1());
    iStart = (long) _grid->GetIFromKey(key);
    jStart = (long) _grid->GetJFromKey(key);

    key  = _grid->GetKeyAtPoint(tempTargetLine.GetPoint2());
    iEnd = (long) _grid->GetIFromKey(key);
    jEnd = (long) _grid->GetJFromKey(key);

    deltaX  = (int) (iEnd - iStart);
    deltaY  = (int) (jEnd - jStart);
    deltaX1 = abs((int) (iEnd - iStart));
    deltaY1 = abs((int) (jEnd - jStart));

    px = 2 * deltaY1 - deltaX1;
    py = 2 * deltaX1 - deltaY1;

    if(deltaY1 <= deltaX1) {
        if(deltaX >= 0) {
            iDot = iStart;
            jDot = jStart;
            xe   = iEnd;
        } else {
            iDot = iEnd;
            jDot = jEnd;
            xe   = iStart;
        }
        if(_gridCode[jDot * iMax + iDot] == uid) {
            /* //find a good neighborvalue
             neigh = _grid->GetNeighbors(jDot*_iMax + iDot);
             if ((neigh.key[0] >= 0) && (_gridCode[neigh.key[0]] == INSIDE)) {
                 goodneighbor = neigh.key[0];
             } else if ((neigh.key[1] >= 0) && (_gridCode[neigh.key[1]] == INSIDE)) {
                 goodneighbor = neigh.key[1];
             } else if ((neigh.key[2] >= 0) && (_gridCode[neigh.key[2]] == INSIDE)) {
                 goodneighbor = neigh.key[2];
             } else if ((neigh.key[3] >= 0) && (_gridCode[neigh.key[3]] == INSIDE)) {
                 goodneighbor = neigh.key[3];
             } else {
                 //ERROR - should have an inside neighbor
                 Log->Write("ERROR:\t In finalizeTargetLine");
             }
             //write the value on targetline
             if ((target[goodneighbor]._x == 0.) && (target[goodneighbor]._y == 0.)) {
                 //ERROR - should have a true vector
                 Log->Write("ERROR:\t (0;0) In FinalizeTargetLine");
             }*/
            newArrayPt[jDot * iMax + iDot] = passvector;
        } else if(_gridCode[jDot * iMax + iDot] == WALL) {
            //do nothing
        } else {
            newArrayPt[jDot * iMax + iDot] = passvector;
            //Log->Write("ERROR:\t in finalizingTargetLine");
        }
        for(i = 0; iDot < xe; ++i) {
            ++iDot;
            if(px < 0) {
                px += 2 * deltaY1;
            } else {
                if((deltaX < 0 && deltaY < 0) || (deltaX > 0 && deltaY > 0)) {
                    ++jDot;
                } else {
                    --jDot;
                }
                px += 2 * (deltaY1 - deltaX1);
            }
            if(_gridCode[jDot * iMax + iDot] == uid) {
                /*//find a good neighborvalue
                neigh = _grid->GetNeighbors(jDot*_iMax + iDot);
                if ((neigh.key[0] >= 0) && (_gridCode[neigh.key[0]] == INSIDE)) {
                    goodneighbor = neigh.key[0];
                } else if ((neigh.key[1] >= 0) && (_gridCode[neigh.key[1]] == INSIDE)) {
                    goodneighbor = neigh.key[1];
                } else if ((neigh.key[2] >= 0) && (_gridCode[neigh.key[2]] == INSIDE)) {
                    goodneighbor = neigh.key[2];
                } else if ((neigh.key[3] >= 0) && (_gridCode[neigh.key[3]] == INSIDE)) {
                    goodneighbor = neigh.key[3];
                } else {
                    //ERROR - should have an inside neighbor
                    Log->Write("ERROR:\t In finalizeTargetLine");
                }
                //write the value on targetline
                if ((target[goodneighbor]._x == 0.) && (target[goodneighbor]._y == 0.)) {
                    //ERROR - should have a true vector
                    Log->Write("ERROR:\t (0;0) In FinalizeTargetLine");
                }*/
                newArrayPt[jDot * iMax + iDot] = passvector;
            } else if(_gridCode[jDot * iMax + iDot] == WALL) {
                //do nothing
            } else {
                newArrayPt[jDot * iMax + iDot] = passvector;
                //Log->Write("ERROR:\t in finalizingTargetLine");
            }
        }
    } else {
        if(deltaY >= 0) {
            iDot = iStart;
            jDot = jStart;
            ye   = jEnd;
        } else {
            iDot = iEnd;
            jDot = jEnd;
            ye   = jStart;
        }
        if(_gridCode[jDot * iMax + iDot] == uid) {
            /*//find a good neighborvalue
            neigh = _grid->GetNeighbors(jDot*_iMax + iDot);
            if ((neigh.key[0] >= 0) && (_gridCode[neigh.key[0]] == INSIDE)) {
                goodneighbor = neigh.key[0];
            } else if ((neigh.key[1] >= 0) && (_gridCode[neigh.key[1]] == INSIDE)) {
                goodneighbor = neigh.key[1];
            } else if ((neigh.key[2] >= 0) && (_gridCode[neigh.key[2]] == INSIDE)) {
                goodneighbor = neigh.key[2];
            } else if ((neigh.key[3] >= 0) && (_gridCode[neigh.key[3]] == INSIDE)) {
                goodneighbor = neigh.key[3];
            } else {
                //ERROR - should have an inside neighbor
                Log->Write("ERROR:\t In finalizeTargetLine");
            }
            //write the value on targetline
            if ((target[goodneighbor]._x == 0.) && (target[goodneighbor]._y == 0.)) {
                //ERROR - should have a true vector
                Log->Write("ERROR:\t (0;0) In FinalizeTargetLine");
            }*/
            newArrayPt[jDot * iMax + iDot] = passvector;
        } else if(_gridCode[jDot * iMax + iDot] == WALL) {
            //do nothing
        } else {
            newArrayPt[jDot * iMax + iDot] = passvector;
            //Log->Write("ERROR:\t in finalizingTargetLine");
        }
        for(i = 0; jDot < ye; ++i) {
            ++jDot;
            if(py <= 0) {
                py += 2 * deltaX1;
            } else {
                if((deltaX < 0 && deltaY < 0) || (deltaX > 0 && deltaY > 0)) {
                    ++iDot;
                } else {
                    --iDot;
                }
                py += 2 * (deltaX1 - deltaY1);
            }
            if(_gridCode[jDot * iMax + iDot] == uid) {
                /*//find a good neighborvalue
                neigh = _grid->GetNeighbors(jDot*_iMax + iDot);
                if ((neigh.key[0] >= 0) && (_gridCode[neigh.key[0]] == INSIDE)) {
                    goodneighbor = neigh.key[0];
                } else if ((neigh.key[1] >= 0) && (_gridCode[neigh.key[1]] == INSIDE)) {
                    goodneighbor = neigh.key[1];
                } else if ((neigh.key[2] >= 0) && (_gridCode[neigh.key[2]] == INSIDE)) {
                    goodneighbor = neigh.key[2];
                } else if ((neigh.key[3] >= 0) && (_gridCode[neigh.key[3]] == INSIDE)) {
                    goodneighbor = neigh.key[3];
                } else {
                    //ERROR - should have an inside neighbor
                    Log->Write("ERROR:\t In finalizeTargetLine");
                }
                //write the value on targetline
                if ((target[goodneighbor]._x == 0.) && (target[goodneighbor]._y == 0.)) {
                    //ERROR - should have a true vector
                    Log->Write("ERROR:\t (0;0) In FinalizeTargetLine");
                }*/
                newArrayPt[jDot * iMax + iDot] = passvector;
            } else if(_gridCode[jDot * iMax + iDot] == WALL) {
                //do nothing
            } else {
                newArrayPt[jDot * iMax + iDot] = passvector;
                //Log->Write("ERROR:\t in finalizingTargetLine");
            }
        }
    }
}

void UnivFFviaFM::DrawLinesOnGrid(std::map<int, Line> & doors, int * grid)
{
    for(auto && doorPair : doors) {
        int tempUID       = doorPair.first;
        Line tempDoorLine = Line(doorPair.second);
        DrawLinesOnGrid(tempDoorLine, grid, tempUID);
    }
}

template <typename T>
void UnivFFviaFM::DrawLinesOnGrid(std::vector<Line> & wallArg, T * target, const T value)
{ //no init, plz init elsewhere

    for(auto & line : wallArg) {
        DrawLinesOnGrid(line, target, value);
    } //loop over all walls

} //DrawLinesOnGrid

template <typename T>
void UnivFFviaFM::DrawLinesOnGrid(Line & line, T * target, const T value)
{ //no init, plz init elsewhere
    // i~x; j~y;
    //http://stackoverflow.com/questions/10060046/drawing-lines-with-bresenhams-line-algorithm
    //src in answer of "Avi"; adapted to fit this application

    //grid handeling local vars:
    long int iMax = _grid->GetiMax();

    long int iStart, iEnd;
    long int jStart, jEnd;
    long int iDot, jDot;
    long int key;
    long int deltaX, deltaY, deltaX1, deltaY1, px, py, xe, ye, i; //Bresenham Algorithm


    key    = _grid->GetKeyAtPoint(line.GetPoint1());
    iStart = (long) _grid->GetIFromKey(key);
    jStart = (long) _grid->GetJFromKey(key);

    key  = _grid->GetKeyAtPoint(line.GetPoint2());
    iEnd = (long) _grid->GetIFromKey(key);
    jEnd = (long) _grid->GetJFromKey(key);

    deltaX  = (int) (iEnd - iStart);
    deltaY  = (int) (jEnd - jStart);
    deltaX1 = abs((int) (iEnd - iStart));
    deltaY1 = abs((int) (jEnd - jStart));

    px = 2 * deltaY1 - deltaX1;
    py = 2 * deltaX1 - deltaY1;

    if(deltaY1 <= deltaX1) {
        if(deltaX >= 0) {
            iDot = iStart;
            jDot = jStart;
            xe   = iEnd;
        } else {
            iDot = iEnd;
            jDot = jEnd;
            xe   = iStart;
        }
        if((_gridCode[jDot * iMax + iDot] != WALL) &&
           (_gridCode[jDot * iMax + iDot] != CLOSED_CROSSING) &&
           (_gridCode[jDot * iMax + iDot] != CLOSED_TRANSITION)) {
            target[jDot * iMax + iDot] = value;
        }
        for(i = 0; iDot < xe; ++i) {
            ++iDot;
            if(px < 0) {
                px += 2 * deltaY1;
            } else {
                if((deltaX < 0 && deltaY < 0) || (deltaX > 0 && deltaY > 0)) {
                    ++jDot;
                } else {
                    --jDot;
                }
                px += 2 * (deltaY1 - deltaX1);
            }
            if((_gridCode[jDot * iMax + iDot] != WALL) &&
               (_gridCode[jDot * iMax + iDot] != CLOSED_CROSSING) &&
               (_gridCode[jDot * iMax + iDot] != CLOSED_TRANSITION)) {
                target[jDot * iMax + iDot] = value;
            }
        }
    } else {
        if(deltaY >= 0) {
            iDot = iStart;
            jDot = jStart;
            ye   = jEnd;
        } else {
            iDot = iEnd;
            jDot = jEnd;
            ye   = jStart;
        }
        if((_gridCode[jDot * iMax + iDot] != WALL) &&
           (_gridCode[jDot * iMax + iDot] != CLOSED_CROSSING) &&
           (_gridCode[jDot * iMax + iDot] != CLOSED_TRANSITION)) {
            target[jDot * iMax + iDot] = value;
        }
        for(i = 0; jDot < ye; ++i) {
            ++jDot;
            if(py <= 0) {
                py += 2 * deltaX1;
            } else {
                if((deltaX < 0 && deltaY < 0) || (deltaX > 0 && deltaY > 0)) {
                    ++iDot;
                } else {
                    --iDot;
                }
                py += 2 * (deltaX1 - deltaY1);
            }
            if((_gridCode[jDot * iMax + iDot] != WALL) &&
               (_gridCode[jDot * iMax + iDot] != CLOSED_CROSSING) &&
               (_gridCode[jDot * iMax + iDot] != CLOSED_TRANSITION)) {
                target[jDot * iMax + iDot] = value;
            }
        }
    }
} //DrawLinesOnGrid

template <typename T>
void UnivFFviaFM::DrawLinesOnWall(std::vector<Line> & wallArg, T * target, const T value)
{ //no init, plz init elsewhere

    for(auto & line : wallArg) {
        DrawLinesOnWall(line, target, value);
    } //loop over all walls

} //DrawLinesOnWall

template <typename T>
void UnivFFviaFM::DrawLinesOnWall(Line & line, T * target, const T value)
{ //no init, plz init elsewhere
    // i~x; j~y;
    //http://stackoverflow.com/questions/10060046/drawing-lines-with-bresenhams-line-algorithm
    //src in answer of "Avi"; adapted to fit this application

    //grid handeling local vars:
    long int iMax = _grid->GetiMax();

    long int iStart, iEnd;
    long int jStart, jEnd;
    long int iDot, jDot;
    long int key;
    long int deltaX, deltaY, deltaX1, deltaY1, px, py, xe, ye, i; //Bresenham Algorithm


    key    = _grid->GetKeyAtPoint(line.GetPoint1());
    iStart = (long) _grid->GetIFromKey(key);
    jStart = (long) _grid->GetJFromKey(key);

    key  = _grid->GetKeyAtPoint(line.GetPoint2());
    iEnd = (long) _grid->GetIFromKey(key);
    jEnd = (long) _grid->GetJFromKey(key);

    deltaX  = (int) (iEnd - iStart);
    deltaY  = (int) (jEnd - jStart);
    deltaX1 = abs((int) (iEnd - iStart));
    deltaY1 = abs((int) (jEnd - jStart));

    px = 2 * deltaY1 - deltaX1;
    py = 2 * deltaX1 - deltaY1;

    if(deltaY1 <= deltaX1) {
        if(deltaX >= 0) {
            iDot = iStart;
            jDot = jStart;
            xe   = iEnd;
        } else {
            iDot = iEnd;
            jDot = jEnd;
            xe   = iStart;
        }
        if((_gridCode[jDot * iMax + iDot] != CLOSED_CROSSING) &&
           (_gridCode[jDot * iMax + iDot] != CLOSED_TRANSITION)) {
            target[jDot * iMax + iDot] = value;
        }
        for(i = 0; iDot < xe; ++i) {
            ++iDot;
            if(px < 0) {
                px += 2 * deltaY1;
            } else {
                if((deltaX < 0 && deltaY < 0) || (deltaX > 0 && deltaY > 0)) {
                    ++jDot;
                } else {
                    --jDot;
                }
                px += 2 * (deltaY1 - deltaX1);
            }
            if((_gridCode[jDot * iMax + iDot] != CLOSED_CROSSING) &&
               (_gridCode[jDot * iMax + iDot] != CLOSED_TRANSITION)) {
                target[jDot * iMax + iDot] = value;
            }
        }
    } else {
        if(deltaY >= 0) {
            iDot = iStart;
            jDot = jStart;
            ye   = jEnd;
        } else {
            iDot = iEnd;
            jDot = jEnd;
            ye   = jStart;
        }
        if((_gridCode[jDot * iMax + iDot] != CLOSED_CROSSING) &&
           (_gridCode[jDot * iMax + iDot] != CLOSED_TRANSITION)) {
            target[jDot * iMax + iDot] = value;
        }
        for(i = 0; jDot < ye; ++i) {
            ++jDot;
            if(py <= 0) {
                py += 2 * deltaX1;
            } else {
                if((deltaX < 0 && deltaY < 0) || (deltaX > 0 && deltaY > 0)) {
                    ++iDot;
                } else {
                    --iDot;
                }
                py += 2 * (deltaX1 - deltaY1);
            }
            if((_gridCode[jDot * iMax + iDot] != CLOSED_CROSSING) &&
               (_gridCode[jDot * iMax + iDot] != CLOSED_TRANSITION)) {
                target[jDot * iMax + iDot] = value;
            }
        }
    }
} //DrawLinesOnWall

void UnivFFviaFM::CalcFF(double * costOutput, Point * directionOutput, const double * const speed)
{
    //CompareCostTrips comp = CompareCostTrips(costOutput);
    std::priority_queue<long int, std::vector<long int>, CompareCostTrips> trialfield(
        costOutput); //pass the argument for the constr of CompareCostTrips
    //std::priority_queue<long int, std::vector<long int>, CompareCostTrips> trialfield2(comp);      //pass the CompareCostTrips object directly

    directNeighbor local_neighbor = _grid->GetNeighbors(0);
    long int aux                  = 0;
    //init trial field
    for(long int i = 0; i < _nPoints; ++i) {
        if(costOutput[i] == 0.0) {
            //check for negative neighbours, calc that ones and add to queue trialfield
            local_neighbor = _grid->GetNeighbors(i);

            //check for valid neigh
            aux = local_neighbor.key[0];
            if((aux != -2) && (_gridCode[aux] != WALL) && (_gridCode[aux] != OUTSIDE) &&
               (costOutput[aux] < 0.0)) {
                CalcCost(aux, costOutput, directionOutput, speed);
                trialfield.emplace(aux);
                //trialfield2.emplace(aux);
            }
            aux = local_neighbor.key[1];
            if((aux != -2) && (_gridCode[aux] != WALL) && (_gridCode[aux] != OUTSIDE) &&
               (costOutput[aux] < 0.0)) {
                CalcCost(aux, costOutput, directionOutput, speed);
                trialfield.emplace(aux);
                //trialfield2.emplace(aux);
            }
            aux = local_neighbor.key[2];
            if((aux != -2) && (_gridCode[aux] != WALL) && (_gridCode[aux] != OUTSIDE) &&
               (costOutput[aux] < 0.0)) {
                CalcCost(aux, costOutput, directionOutput, speed);
                trialfield.emplace(aux);
                //trialfield2.emplace(aux);
            }
            aux = local_neighbor.key[3];
            if((aux != -2) && (_gridCode[aux] != WALL) && (_gridCode[aux] != OUTSIDE) &&
               (costOutput[aux] < 0.0)) {
                CalcCost(aux, costOutput, directionOutput, speed);
                trialfield.emplace(aux);
                //trialfield2.emplace(aux);
            }
        }
    }

    while(!trialfield.empty()) {
        local_neighbor = _grid->GetNeighbors(trialfield.top());
        trialfield.pop();

        //check for valid neigh
        aux = local_neighbor.key[0];
        if((aux != -2) && (_gridCode[aux] != WALL) && (_gridCode[aux] != OUTSIDE) &&
           (costOutput[aux] < 0.0)) {
            CalcCost(aux, costOutput, directionOutput, speed);
            trialfield.emplace(aux);
            //trialfield2.emplace(aux);
        }
        aux = local_neighbor.key[1];
        if((aux != -2) && (_gridCode[aux] != WALL) && (_gridCode[aux] != OUTSIDE) &&
           (costOutput[aux] < 0.0)) {
            CalcCost(aux, costOutput, directionOutput, speed);
            trialfield.emplace(aux);
            //trialfield2.emplace(aux);
        }
        aux = local_neighbor.key[2];
        if((aux != -2) && (_gridCode[aux] != WALL) && (_gridCode[aux] != OUTSIDE) &&
           (costOutput[aux] < 0.0)) {
            CalcCost(aux, costOutput, directionOutput, speed);
            trialfield.emplace(aux);
            //trialfield2.emplace(aux);
        }
        aux = local_neighbor.key[3];
        if((aux != -2) && (_gridCode[aux] != WALL) && (_gridCode[aux] != OUTSIDE) &&
           (costOutput[aux] < 0.0)) {
            CalcCost(aux, costOutput, directionOutput, speed);
            trialfield.emplace(aux);
            //trialfield2.emplace(aux);
        }
    }
}

void UnivFFviaFM::CalcCost(long int key, double * cost, Point * dir, const double * speed)
{
    //adapt from calcFloorfield
    double row       = std::numeric_limits<double>::max();
    double col       = std::numeric_limits<double>::max();
    long int aux     = -1; //will be set below
    bool pointsUp    = false;
    bool pointsRight = false;

    directNeighbor dNeigh = _grid->GetNeighbors(key);

    aux = dNeigh.key[0];
    //hint: trialfield[i].cost = dist2Wall + i; <<< set in resetGoalAndCosts
    if((aux != -2) && //neighbor is a gridpoint
       (cost[aux] != magicnum(UNKNOWN_COST)) &&
       (cost[aux] != magicnum(UNKNOWN_DISTANCE)) && //gridpoint holds a calculated value
       (_gridCode[aux] != WALL))                    //gridpoint holds a calculated value
    {
        row         = cost[aux];
        pointsRight = true;
        if(row < 0) {
            Logging::Error(
                fmt::format(check_fmt("In CalcCost something went wrong {:.2f} {:d}"), row, aux));
            row = std::numeric_limits<double>::max();
        }
    }
    aux = dNeigh.key[2];
    if((aux != -2) && //neighbor is a gridpoint
       (cost[aux] != magicnum(UNKNOWN_COST)) &&
       (cost[aux] != magicnum(UNKNOWN_DISTANCE)) &&   //gridpoint holds a calculated value
       (_gridCode[aux] != WALL) && (cost[aux] < row)) //calculated value promises smaller cost
    {
        row         = cost[aux];
        pointsRight = false;
    }

    aux = dNeigh.key[1];
    //hint: trialfield[i].cost = dist2Wall + i; <<< set in parseBuilding after linescan call
    if((aux != -2) && //neighbor is a gridpoint
       (cost[aux] != magicnum(UNKNOWN_COST)) &&
       (cost[aux] != magicnum(UNKNOWN_DISTANCE)) && //gridpoint holds a calculated value
       (_gridCode[aux] != WALL)) {
        col      = cost[aux];
        pointsUp = true;
        if(col < 0) {
            Logging::Error(
                fmt::format(check_fmt("In CalcCost something went wrong {:.2f} {:d}"), row, aux));
            col = std::numeric_limits<double>::max();
        }
    }
    aux = dNeigh.key[3];
    if((aux != -2) && //neighbor is a gridpoint
       (cost[aux] != magicnum(UNKNOWN_COST)) &&
       (cost[aux] != magicnum(UNKNOWN_DISTANCE)) &&   //gridpoint holds a calculated value
       (_gridCode[aux] != WALL) && (cost[aux] < col)) //calculated value promises smaller cost
    {
        col      = cost[aux];
        pointsUp = false;
    }
    if(col == std::numeric_limits<double>::max()) { //one sided update with row
        cost[key] = OnesidedCalc(row, _grid->Gethx() / speed[key]);
        //flag[key] = FM_SINGLE;
        if(!dir) {
            return;
        }
        if(pointsRight) {
            dir[key]._x = (-(cost[key + 1] - cost[key]) / _grid->Gethx());
            dir[key]._y = (0.);
        } else {
            dir[key]._x = (-(cost[key] - cost[key - 1]) / _grid->Gethx());
            dir[key]._y = (0.);
        }
        dir[key] = dir[key].Normalized();
        return;
    }

    if(row == std::numeric_limits<double>::max()) { //one sided update with col
        cost[key] = OnesidedCalc(col, _grid->Gethy() / speed[key]);
        //flag[key] = FM_SINGLE;
        if(!dir) {
            return;
        }
        if(pointsUp) {
            dir[key]._x = (0.);
            dir[key]._y = (-(cost[key + (_grid->GetiMax())] - cost[key]) / _grid->Gethy());
        } else {
            dir[key]._x = (0.);
            dir[key]._y = (-(cost[key] - cost[key - (_grid->GetiMax())]) / _grid->Gethy());
        }
        dir[key] = dir[key].Normalized();
        return;
    }

    //two sided update
    double precheck = TwosidedCalc(row, col, _grid->Gethx() / speed[key]);
    if(precheck >= 0) {
        cost[key] = precheck;
        //flag[key] = FM_DOUBLE;
        if(!dir) {
            return;
        }
        if(pointsUp && pointsRight) {
            dir[key]._x = (-(cost[key + 1] - cost[key]) / _grid->Gethx());
            dir[key]._y = (-(cost[key + (_grid->GetiMax())] - cost[key]) / _grid->Gethy());
        }
        if(pointsUp && !pointsRight) {
            dir[key]._x = (-(cost[key] - cost[key - 1]) / _grid->Gethx());
            dir[key]._y = (-(cost[key + (_grid->GetiMax())] - cost[key]) / _grid->Gethy());
        }
        if(!pointsUp && pointsRight) {
            dir[key]._x = (-(cost[key + 1] - cost[key]) / _grid->Gethx());
            dir[key]._y = (-(cost[key] - cost[key - (_grid->GetiMax())]) / _grid->Gethy());
        }
        if(!pointsUp && !pointsRight) {
            dir[key]._x = (-(cost[key] - cost[key - 1]) / _grid->Gethx());
            dir[key]._y = (-(cost[key] - cost[key - (_grid->GetiMax())]) / _grid->Gethy());
        }
    } else {
        Logging::Error("else in twosided Dist");
    }
    dir[key] = dir[key].Normalized();
}

void UnivFFviaFM::CalcDF(double * costOutput, Point * directionOutput, const double * speed)
{
    //CompareCostTrips comp = CompareCostTrips(costOutput);
    std::priority_queue<long int, std::vector<long int>, CompareCostTrips> trialfield(
        costOutput); //pass the argument for the constr of CompareCostTrips
    //std::priority_queue<long int, std::vector<long int>, CompareCostTrips> trialfield2(comp);      //pass the CompareCostTrips object directly


    directNeighbor local_neighbor = _grid->GetNeighbors(0);
    long int aux                  = 0;
    //init trial field
    for(long int i = 0; i < _nPoints; ++i) {
        if(costOutput[i] == 0.0) {
            //check for negative neighbours, calc that ones and add to queue trialfield
            local_neighbor = _grid->GetNeighbors(i);

            //check for valid neigh
            aux = local_neighbor.key[0];
            if((aux != -2) && (_gridCode[aux] != WALL) && (_gridCode[aux] != OUTSIDE) &&
               (costOutput[aux] < 0.0)) {
                CalcDist(aux, costOutput, directionOutput, speed);
                trialfield.emplace(aux);
                //trialfield2.emplace(aux);
            }
            aux = local_neighbor.key[1];
            if((aux != -2) && (_gridCode[aux] != WALL) && (_gridCode[aux] != OUTSIDE) &&
               (costOutput[aux] < 0.0)) {
                CalcDist(aux, costOutput, directionOutput, speed);
                trialfield.emplace(aux);
                //trialfield2.emplace(aux);
            }
            aux = local_neighbor.key[2];
            if((aux != -2) && (_gridCode[aux] != WALL) && (_gridCode[aux] != OUTSIDE) &&
               (costOutput[aux] < 0.0)) {
                CalcDist(aux, costOutput, directionOutput, speed);
                trialfield.emplace(aux);
                //trialfield2.emplace(aux);
            }
            aux = local_neighbor.key[3];
            if((aux != -2) && (_gridCode[aux] != WALL) && (_gridCode[aux] != OUTSIDE) &&
               (costOutput[aux] < 0.0)) {
                CalcDist(aux, costOutput, directionOutput, speed);
                trialfield.emplace(aux);
                //trialfield2.emplace(aux);
            }
        }
    }

    while(!trialfield.empty()) {
        local_neighbor = _grid->GetNeighbors(trialfield.top());
        trialfield.pop();

        //check for valid neigh
        aux = local_neighbor.key[0];
        if((aux != -2) && (_gridCode[aux] != WALL) && (_gridCode[aux] != OUTSIDE) &&
           (costOutput[aux] < 0.0)) {
            CalcDist(aux, costOutput, directionOutput, speed);
            trialfield.emplace(aux);
            //trialfield2.emplace(aux);
        }
        aux = local_neighbor.key[1];
        if((aux != -2) && (_gridCode[aux] != WALL) && (_gridCode[aux] != OUTSIDE) &&
           (costOutput[aux] < 0.0)) {
            CalcDist(aux, costOutput, directionOutput, speed);
            trialfield.emplace(aux);
            //trialfield2.emplace(aux);
        }
        aux = local_neighbor.key[2];
        if((aux != -2) && (_gridCode[aux] != WALL) && (_gridCode[aux] != OUTSIDE) &&
           (costOutput[aux] < 0.0)) {
            CalcDist(aux, costOutput, directionOutput, speed);
            trialfield.emplace(aux);
            //trialfield2.emplace(aux);
        }
        aux = local_neighbor.key[3];
        if((aux != -2) && (_gridCode[aux] != WALL) && (_gridCode[aux] != OUTSIDE) &&
           (costOutput[aux] < 0.0)) {
            CalcDist(aux, costOutput, directionOutput, speed);
            trialfield.emplace(aux);
            //trialfield2.emplace(aux);
        }
    }
}

void UnivFFviaFM::CalcDist(long int key, double * cost, Point * dir, const double * speed)
{
    //adapt from calcFloorfield
    double row       = std::numeric_limits<double>::max();
    double col       = std::numeric_limits<double>::max();
    long int aux     = -1; //will be set below
    bool pointsUp    = false;
    bool pointsRight = false;

    directNeighbor dNeigh = _grid->GetNeighbors(key);

    aux = dNeigh.key[0];
    //hint: trialfield[i].cost = dist2Wall + i; <<< set in resetGoalAndCosts
    if((aux != -2) && //neighbor is a gridpoint
       (cost[aux] != magicnum(UNKNOWN_COST)) &&
       (cost[aux] != magicnum(UNKNOWN_DISTANCE)) //gridpoint holds a calculated value
       )                                         //gridpoint holds a calculated value
    {
        row         = cost[aux];
        pointsRight = true;
        if(row < 0) {
            Logging::Error(
                fmt::format(check_fmt("In CalcDist something went wrong {:.2f} {:d}"), row, aux));
            row = std::numeric_limits<double>::max();
        }
    }
    aux = dNeigh.key[2];
    if((aux != -2) && //neighbor is a gridpoint
       (cost[aux] != magicnum(UNKNOWN_COST)) &&
       (cost[aux] != magicnum(UNKNOWN_DISTANCE)) //gridpoint holds a calculated value
       && (cost[aux] < row))                     //calculated value promises smaller cost
    {
        row         = cost[aux];
        pointsRight = false;
    }

    aux = dNeigh.key[1];
    //hint: trialfield[i].cost = dist2Wall + i; <<< set in parseBuilding after linescan call
    if((aux != -2) && //neighbor is a gridpoint
       (cost[aux] != magicnum(UNKNOWN_COST)) &&
       (cost[aux] != magicnum(UNKNOWN_DISTANCE)) //gridpoint holds a calculated value
    ) {
        col      = cost[aux];
        pointsUp = true;
        if(col < 0) {
            Logging::Error(
                fmt::format(check_fmt("In CalcDist something went wrong {:.2f} {:d}"), row, aux));
            col = std::numeric_limits<double>::max();
        }
    }
    aux = dNeigh.key[3];
    if((aux != -2) && //neighbor is a gridpoint
       (cost[aux] != magicnum(UNKNOWN_COST)) &&
       (cost[aux] != magicnum(UNKNOWN_DISTANCE)) && //gridpoint holds a calculated value

       (cost[aux] < col)) //calculated value promises smaller cost
    {
        col      = cost[aux];
        pointsUp = false;
    }
    if(col == std::numeric_limits<double>::max()) { //one sided update with row
        cost[key] = OnesidedCalc(row, _grid->Gethx() / speed[key]);
        //flag[key] = FM_SINGLE;
        if(!dir) {
            return;
        }
        if(pointsRight) {
            dir[key]._x = (-(cost[key + 1] - cost[key]) / _grid->Gethx());
            dir[key]._y = (0.);
        } else {
            dir[key]._x = (-(cost[key] - cost[key - 1]) / _grid->Gethx());
            dir[key]._y = (0.);
        }
        dir[key] = dir[key].Normalized();
        return;
    }

    if(row == std::numeric_limits<double>::max()) { //one sided update with col
        cost[key] = OnesidedCalc(col, _grid->Gethy() / speed[key]);
        //flag[key] = FM_SINGLE;
        if(!dir) {
            return;
        }
        if(pointsUp) {
            dir[key]._x = (0.);
            dir[key]._y = (-(cost[key + (_grid->GetiMax())] - cost[key]) / _grid->Gethy());
        } else {
            dir[key]._x = (0.);
            dir[key]._y = (-(cost[key] - cost[key - (_grid->GetiMax())]) / _grid->Gethy());
        }
        dir[key] = dir[key].Normalized();
        return;
    }

    //two sided update
    double precheck = TwosidedCalc(row, col, _grid->Gethx() / speed[key]);
    if(precheck >= 0) {
        cost[key] = precheck;
        //flag[key] = FM_DOUBLE;
        if(!dir) {
            return;
        }
        if(pointsUp && pointsRight) {
            dir[key]._x = (-(cost[key + 1] - cost[key]) / _grid->Gethx());
            dir[key]._y = (-(cost[key + (_grid->GetiMax())] - cost[key]) / _grid->Gethy());
        }
        if(pointsUp && !pointsRight) {
            dir[key]._x = (-(cost[key] - cost[key - 1]) / _grid->Gethx());
            dir[key]._y = (-(cost[key + (_grid->GetiMax())] - cost[key]) / _grid->Gethy());
        }
        if(!pointsUp && pointsRight) {
            dir[key]._x = (-(cost[key + 1] - cost[key]) / _grid->Gethx());
            dir[key]._y = (-(cost[key] - cost[key - (_grid->GetiMax())]) / _grid->Gethy());
        }
        if(!pointsUp && !pointsRight) {
            dir[key]._x = (-(cost[key] - cost[key - 1]) / _grid->Gethx());
            dir[key]._y = (-(cost[key] - cost[key - (_grid->GetiMax())]) / _grid->Gethy());
        }
    } else {
        Logging::Error("else in twosided Dist");
    }
    dir[key] = dir[key].Normalized();
}

inline double UnivFFviaFM::OnesidedCalc(double xy, double hDivF)
{
    return xy + hDivF;
}

inline double UnivFFviaFM::TwosidedCalc(double x, double y, double hDivF)
{
    double determinante = (2 * hDivF * hDivF - (x - y) * (x - y));
    if(determinante >= 0) {
        return (x + y + sqrt(determinante)) / 2;
    } else {
        return (x < y) ? (x + hDivF) : (y + hDivF);
    }
}

void UnivFFviaFM::AddTarget(int uid, double * costarray, Point * gradarray)
{
    if(_doors.count(uid) == 0) {
        Logging::Error(
            fmt::format(check_fmt("Could not find door with uid {:d} in Room {:d}"), uid, _room));
        return;
    }
    Line tempTargetLine   = Line(_doors[uid]);
    Point tempCenterPoint = Point(tempTargetLine.GetCentre());
    if(_mode == LINESEGMENT) {
        if(tempTargetLine.GetLength() >
           0.6) { //shorten line from both Points to avoid targeting edges of real door
            const Point & p1 = tempTargetLine.GetPoint1();
            const Point & p2 = tempTargetLine.GetPoint2();
            double length    = tempTargetLine.GetLength();
            double u         = 0.2 / length;
            tempTargetLine   = Line(p1 + (p2 - p1) * u, p1 + (p2 - p1) * (1 - u), 0);
        } else if(tempTargetLine.GetLength() > 0.2) {
            const Point & p1 = tempTargetLine.GetPoint1();
            const Point & p2 = tempTargetLine.GetPoint2();
            double length    = tempTargetLine.GetLength();
            double u         = 0.05 / length;
            tempTargetLine   = Line(p1 + (p2 - p1) * u, p1 + (p2 - p1) * (1 - u), 0);
        }
    }

    //this allocation must be on shared heap! to be accessible by any thread later (should be shared in openmp)
    double * newArrayDBL = (costarray) ? costarray : new double[_nPoints];
    Point * newArrayPt   = nullptr;
    if(_user == DISTANCE_AND_DIRECTIONS_USED) {
        newArrayPt = (gradarray) ? gradarray : new Point[_nPoints];
    }

    if((_costFieldWithKey[uid]) && (_costFieldWithKey[uid] != costarray))
        delete[] _costFieldWithKey[uid];
    _costFieldWithKey[uid] = newArrayDBL;

    //init costarray
    for(int i = 0; i < _nPoints; ++i) {
        if(_gridCode[i] == WALL) {
            newArrayDBL[i] = magicnum(WALL_ON_COSTARRAY);
        } else {
            newArrayDBL[i] = magicnum(UNKNOWN_COST);
        }
    }

    if((_directionFieldWithKey[uid]) && (_directionFieldWithKey[uid] != gradarray))
        delete[] _directionFieldWithKey[uid];
    if(newArrayPt)
        _directionFieldWithKey[uid] = newArrayPt;

    //initialize start area
    if(_mode == LINESEGMENT) {
        DrawLinesOnGrid(tempTargetLine, newArrayDBL, magicnum(TARGET_REGION));
    }
    if(_mode == CENTERPOINT) {
        newArrayDBL[_grid->GetKeyAtPoint(tempCenterPoint)] = magicnum(TARGET_REGION);
    }

    if(_speedmode == FF_WALL_AVOID) {
        CalcFF(newArrayDBL, newArrayPt, _speedFieldSelector[REDU_WALL_SPEED]);
    } else if(_speedmode == FF_HOMO_SPEED) {
        CalcFF(newArrayDBL, newArrayPt, _speedFieldSelector[INITIAL_SPEED]);
    } else if(_speedmode == FF_PED_SPEED) {
        CalcFF(newArrayDBL, newArrayPt, _speedFieldSelector[PED_SPEED]);
    }

    //the rest of the door must be initialized if centerpoint was used. else ff_router will have probs getting localDist
    if(_mode == CENTERPOINT) {
        DrawLinesOnGrid(tempTargetLine, newArrayDBL, magicnum(TARGET_REGION));
    }
    //the directional field is yet undefined on the target line itself. we will use neighboring vector to help agents
    //to cross the line
    if(newArrayPt) {
        Point passvector = tempTargetLine.NormalVec();
        Point trial      = tempTargetLine.GetCentre() - passvector * 0.25;
        Point trial2     = tempTargetLine.GetCentre() + passvector * 0.25;
        if((_grid->IncludesPoint(trial)) && (_gridCode[_grid->GetKeyAtPoint(trial)] == INSIDE)) {
            FinalizeTargetLine(uid, _doors[uid], newArrayPt, passvector);
            FinalizeTargetLine(uid, tempTargetLine, newArrayPt, passvector);
        } else if(
            (_grid->IncludesPoint(trial2)) && (_gridCode[_grid->GetKeyAtPoint(trial2)] == INSIDE)) {
            passvector = passvector * -1.0;
            FinalizeTargetLine(uid, _doors[uid], newArrayPt, passvector);
            FinalizeTargetLine(uid, tempTargetLine, newArrayPt, passvector);

        } else {
            Logging::Error("in AddTarget: calling FinalizeTargetLine");
        }
        //         for (long int i = 0; i < _grid->GetnPoints(); ++i) {
        //             if ((_gridCode[i] != OUTSIDE) && (_gridCode[i] != WALL) && (newArrayPt[i] == Point(0.0, 0.0) )) {
        //                 Log->Write("Mist");
        //             }
        //         }
    }
#pragma omp critical(_uids)
    _uids.emplace_back(uid);
}

void UnivFFviaFM::AddAllTargetsParallel()
{
    //Reason: freeing and reallocating takes time. We do not use already allocated memory, because we do not know if it
    //        is shared memory. Maybe this is not neccessary - maybe reconsider. This way, it is safe. If this function
    //        is called from a parallel region, we all go to hell.
    //free old memory
    for(auto memoryDBL : _costFieldWithKey) {
        if(memoryDBL.first == 0)
            continue; //do not free distancemap
        if(memoryDBL.second)
            delete[](memoryDBL.second);
    }
    for(auto memoryPt : _directionFieldWithKey) {
        if(memoryPt.first == 0)
            continue; //do not free walldirectionmap
        if(memoryPt.second)
            delete[](memoryPt.second);
    }
    //allocate new memory
    for(const auto & uidmap : _doors) {
        _costFieldWithKey[uidmap.first] = new double[_nPoints];
        if(_user == DISTANCE_MEASUREMENTS_ONLY) {
            _directionFieldWithKey[uidmap.first] = nullptr;
        }
        if(_user == DISTANCE_AND_DIRECTIONS_USED) {
            _directionFieldWithKey[uidmap.first] = new Point[_nPoints];
        }
    }

    //parallel region
#pragma omp parallel default(none)
    {
#pragma omp for
        for(size_t i = 0; i < _doors.size(); ++i) {
            auto doorPair = _doors.begin();
            std::advance(doorPair, i);
            AddTarget(
                doorPair->first,
                _costFieldWithKey[doorPair->first],
                _directionFieldWithKey[doorPair->first]);
        }
    };
}

void UnivFFviaFM::AddTargetsParallel(std::vector<int> wantedDoors)
{
    //free old memory (but not the distancemap with key == 0)
    for(int targetUID : wantedDoors) {
        if((targetUID != 0) && _costFieldWithKey.count(targetUID) && _costFieldWithKey[targetUID]) {
            delete[] _costFieldWithKey[targetUID];
        }
        if((targetUID != 0) && _directionFieldWithKey.count(targetUID) &&
           _directionFieldWithKey[targetUID]) {
            delete[] _directionFieldWithKey[targetUID];
        }
    }
    //allocate new memory
    for(int targetUID : wantedDoors) {
        _costFieldWithKey[targetUID] = new double[_nPoints];
        if(_user == DISTANCE_MEASUREMENTS_ONLY) {
            _directionFieldWithKey[targetUID] = nullptr;
        }
        if(_user == DISTANCE_AND_DIRECTIONS_USED) {
            _directionFieldWithKey[targetUID] = new Point[_nPoints];
        }
    }

    //parallel region
#pragma omp parallel default(none) shared(wantedDoors)
    {
#pragma omp for
        for(size_t i = 0; i < wantedDoors.size(); ++i) {
            auto doorUID = wantedDoors.begin();
            std::advance(doorUID, i);
            AddTarget(*doorUID, _costFieldWithKey[*doorUID], _directionFieldWithKey[*doorUID]);
        }
    };
}

std::vector<int> UnivFFviaFM::GetKnownDoorUIDs()
{
    return _uids;
}

void UnivFFviaFM::SetUser(int user)
{
    _user = user;
}

void UnivFFviaFM::SetMode(int mode)
{
    _mode = mode;
}

void UnivFFviaFM::SetSpeedMode(int speedMode)
{
    _speedmode = speedMode;
    if(_speedmode == FF_PED_SPEED && !_speedFieldSelector[PED_SPEED]) {
        _speedFieldSelector[PED_SPEED] = new double[_nPoints];
    }
}


void UnivFFviaFM::WriteFF(const fs::path & filename, std::vector<int> targetID)
{
    fs::create_directory(_configuration->GetProjectRootDir() / "ff_vtk_files");
    auto floorfieldFile = _configuration->GetProjectRootDir() / "ff_vtk_files" / filename;
    Logging::Info(fmt::format(
        check_fmt("Write Floorfield to file: {:s} with {:d} targets."),
        floorfieldFile.string().c_str(),
        targetID.size()));
    std::ofstream file;

    file.open(floorfieldFile.string());

    file << "# vtk DataFile Version 3.0" << std::endl;
    file << "Testdata: Fast Marching: Test: " << std::endl;
    file << "ASCII" << std::endl;
    file << "DATASET STRUCTURED_POINTS" << std::endl;
    file << "DIMENSIONS " << std::to_string(_grid->GetiMax()) << " "
         << std::to_string(_grid->GetjMax()) << " 1" << std::endl;
    file << "ORIGIN " << _grid->GetxMin() << " " << _grid->GetyMin() << " 0" << std::endl;
    file << "SPACING " << std::to_string(_grid->Gethx()) << " " << std::to_string(_grid->Gethy())
         << " 1" << std::endl;
    file << "POINT_DATA " << std::to_string(_grid->GetnPoints()) << std::endl;
    file << "SCALARS GCode float 1" << std::endl;
    file << "LOOKUP_TABLE default" << std::endl;
    if(!_gridCode) {
        return;
    }
    for(long int i = 0; i < _grid->GetnPoints(); ++i) {
        file << _gridCode[i] << std::endl;
    }

    if(_directionFieldWithKey[0]) {
        file << "VECTORS Dir2Wall float" << std::endl;
        for(long int i = 0; i < _grid->GetnPoints(); ++i) {
            file << _directionFieldWithKey[0][i]._x << " " << _directionFieldWithKey[0][i]._y
                 << " 0.0" << std::endl;
        }

        file << "SCALARS Dist2Wall float 1" << std::endl;
        file << "LOOKUP_TABLE default" << std::endl;
        for(long int i = 0; i < _grid->GetnPoints(); ++i) {
            file << _costFieldWithKey[0][i] << std::endl; //@todo: change target to all dist2wall
        }
    }

    if(_subrooms) {
        file << "SCALARS SubroomPtr float 1" << std::endl;
        file << "LOOKUP_TABLE default" << std::endl;
        for(long int i = 0; i < _grid->GetnPoints(); ++i) {
            if(_subrooms[i]) {
                file << _subrooms[i]->GetUID() << std::endl;
            } else {
                file << 0.0 << std::endl;
            }
        }
    }

    if(!targetID.empty()) {
        for(unsigned int iTarget = 0; iTarget < targetID.size(); ++iTarget) {
            if(_costFieldWithKey.count(targetID[iTarget]) == 0) {
                continue;
            }
            double * costarray = _costFieldWithKey[targetID[iTarget]];

            std::string name = _building->GetTransOrCrossByUID(targetID[iTarget])->GetCaption() +
                               "-" + std::to_string(targetID[iTarget]);
            std::replace(name.begin(), name.end(), ' ', '_');

            if(!costarray) {
                continue;
            }

            file << "SCALARS CostTarget" << name << " float 1" << std::endl;
            file << "LOOKUP_TABLE default" << std::endl;
            for(long int i = 0; i < _grid->GetnPoints(); ++i) {
                file << costarray[i] << std::endl;
            }

            if(_directionFieldWithKey.count(targetID[iTarget]) == 0) {
                continue;
            }

            Point * gradarray = _directionFieldWithKey[targetID[iTarget]];
            if(gradarray == nullptr) {
                continue;
            }


            file << "VECTORS GradientTarget" << name << " float" << std::endl;
            for(int i = 0; i < _grid->GetnPoints(); ++i) {
                file << gradarray[i]._x << " " << gradarray[i]._y << " 0.0" << std::endl;
            }
        }
    }
    file.close();
}

//mode is argument, which should not be needed, the info is stored in members like speedmode, ...
double UnivFFviaFM::GetCostToDestination(int destID, const Point & position, int mode)
{
    assert(_grid->IncludesPoint(position));
    long int key = _grid->GetKeyAtPoint(position);
    if((_gridCode[key] == OUTSIDE) || (_gridCode[key] == WALL)) {
        //bresenham line (treppenstruktur) at middle and calculated centre of line are on different gridpoints
        //find a key that belongs domain (must be one left or right and second one below or above)
        if((key + 1 <= _grid->GetnPoints()) && (_gridCode[key + 1] != OUTSIDE) &&
           (_gridCode[key + 1] != WALL)) {
            key = key + 1;
        } else if(
            (key - 1 >= 0) && (_gridCode[key - 1] != OUTSIDE) && (_gridCode[key - 1] != WALL)) {
            key = key - 1;
        } else if(
            (key >= _grid->GetiMax()) && (_gridCode[key - _grid->GetiMax()] != OUTSIDE) &&
            (_gridCode[key - _grid->GetiMax()] != WALL)) {
            key = key - _grid->GetiMax();
        } else if(
            (key < _grid->GetnPoints() - _grid->GetiMax()) &&
            (_gridCode[key + _grid->GetiMax()] != OUTSIDE) &&
            (_gridCode[key + _grid->GetiMax()] != WALL)) {
            key = key + _grid->GetiMax();
        } else {
            // Log->Write("ERROR:\t In GetCostToDestination(3 args)");
        }
    }
    if(_costFieldWithKey.count(destID) == 1 && _costFieldWithKey[destID]) {
        return _costFieldWithKey[destID][key];
    } else if(_doors.count(destID) > 0) {
        _costFieldWithKey[destID] = new double[_nPoints];
        if(_user == DISTANCE_AND_DIRECTIONS_USED) {
            _directionFieldWithKey[destID] = new Point[_nPoints];
        } else {
            _directionFieldWithKey[destID] = nullptr;
        }

        AddTarget(destID, _costFieldWithKey[destID], _directionFieldWithKey[destID]);
        return GetCostToDestination(destID, position, mode);
    }
    return std::numeric_limits<double>::max();
}

double UnivFFviaFM::GetCostToDestination(int destID, const Point & position)
{
    assert(_grid->IncludesPoint(position));
    long int key = _grid->GetKeyAtPoint(position);
    if((_gridCode[key] == OUTSIDE) || (_gridCode[key] == WALL)) {
        //bresenham line (treppenstruktur) GetKeyAtPoint yields gridpoint next to edge, although position is on edge
        //find a key that belongs domain (must be one left or right and second one below or above)
        if((key + 1 <= _grid->GetnPoints()) && (_gridCode[key + 1] != OUTSIDE) &&
           (_gridCode[key + 1] != WALL)) {
            key = key + 1;
        } else if(
            (key - 1 >= 0) && (_gridCode[key - 1] != OUTSIDE) && (_gridCode[key - 1] != WALL)) {
            key = key - 1;
        } else if(
            (key >= _grid->GetiMax()) && (_gridCode[key - _grid->GetiMax()] != OUTSIDE) &&
            (_gridCode[key - _grid->GetiMax()] != WALL)) {
            key = key - _grid->GetiMax();
        } else if(
            (key < _grid->GetnPoints() - _grid->GetiMax()) &&
            (_gridCode[key + _grid->GetiMax()] != OUTSIDE) &&
            (_gridCode[key + _grid->GetiMax()] != WALL)) {
            key = key + _grid->GetiMax();
        } else {
            // Log->Write("ERROR:\t In GetCostToDestination(2 args)");
        }
    }
    if(_costFieldWithKey.count(destID) == 1 && _costFieldWithKey[destID]) {
        return _costFieldWithKey[destID][key];
    } else if(_doors.count(destID) > 0) {
        _costFieldWithKey[destID] = new double[_nPoints];
        if(_user == DISTANCE_AND_DIRECTIONS_USED) {
            _directionFieldWithKey[destID] = new Point[_nPoints];
        } else {
            _directionFieldWithKey[destID] = nullptr;
        }

        AddTarget(destID, _costFieldWithKey[destID], _directionFieldWithKey[destID]);
        return GetCostToDestination(destID, position);
    }
    return std::numeric_limits<double>::max();
}

double UnivFFviaFM::GetDistanceBetweenDoors(const int door1_ID, const int door2_ID)
{
    assert(_doors.count(door1_ID) != 0);
    assert(_doors.count(door2_ID) != 0);

    if(_costFieldWithKey.count(door1_ID) == 1 && _costFieldWithKey[door1_ID]) {
        long int key = _grid->GetKeyAtPoint(_doors.at(door2_ID).GetCentre());
        if(_gridCode[key] != door2_ID) {
            //bresenham line (treppenstruktur) GetKeyAtPoint yields gridpoint next to edge, although position is on edge
            //find a key that belongs to door (must be one left or right and second one below or above)
            if(_gridCode[key + 1] == door2_ID) {
                key = key + 1;
            } else if(_gridCode[key - 1] == door2_ID) {
                key = key - 1;
            } else {
                Logging::Error("In DistanceBetweenDoors.");
            }
        }
        return _costFieldWithKey[door1_ID][key];
    } else if(_doors.count(door1_ID) > 0) {
        _costFieldWithKey[door1_ID] = new double[_nPoints];
        if(_user == DISTANCE_AND_DIRECTIONS_USED) {
            _directionFieldWithKey[door1_ID] = new Point[_nPoints];
        } else {
            _directionFieldWithKey[door1_ID] = nullptr;
        }

        AddTarget(door1_ID, _costFieldWithKey[door1_ID], _directionFieldWithKey[door1_ID]);
        return GetDistanceBetweenDoors(door1_ID, door2_ID);
    }
    return std::numeric_limits<double>::max();
}

RectGrid * UnivFFviaFM::GetGrid()
{
    return _grid;
}

void UnivFFviaFM::GetDirectionToUID(int destID, long int key, Point & direction, int mode)
{
    assert(key > 0 && key < _nPoints);
    if((_gridCode[key] == OUTSIDE) || (_gridCode[key] == WALL)) {
        //bresenham line (treppenstruktur) GetKeyAtPoint yields gridpoint next to edge, although position is on edge
        //find a key that belongs domain (must be one left or right and second one below or above)
        if((key + 1 <= _grid->GetnPoints()) && (_gridCode[key + 1] != OUTSIDE) &&
           (_gridCode[key + 1] != WALL)) {
            key = key + 1;
        } else if(
            (key - 1 >= 0) && (_gridCode[key - 1] != OUTSIDE) && (_gridCode[key - 1] != WALL)) {
            key = key - 1;
        } else if(
            (key >= _grid->GetiMax()) && (_gridCode[key - _grid->GetiMax()] != OUTSIDE) &&
            (_gridCode[key - _grid->GetiMax()] != WALL)) {
            key = key - _grid->GetiMax();
        } else if(
            (key < _grid->GetnPoints() - _grid->GetiMax()) &&
            (_gridCode[key + _grid->GetiMax()] != OUTSIDE) &&
            (_gridCode[key + _grid->GetiMax()] != WALL)) {
            key = key + _grid->GetiMax();
        } else {
            Logging::Error("In GetDirectionToUID (4 args)");
        }
    }
    if(_directionFieldWithKey.count(destID) == 1 && _directionFieldWithKey[destID]) {
        direction = _directionFieldWithKey[destID][key];
    } else if(_doors.count(destID) > 0) {
        //free memory if needed
        if(_costFieldWithKey.count(destID) == 1 && _costFieldWithKey[destID]) {
            delete[] _costFieldWithKey[destID];
        }
        //allocate memory
        _costFieldWithKey[destID] = new double[_nPoints];
        if(_user == DISTANCE_AND_DIRECTIONS_USED) {
            _directionFieldWithKey[destID] = new Point[_nPoints];
        } else {
            _directionFieldWithKey[destID] = nullptr;
        }

        //calculate destID's fields and call function
        AddTarget(destID, _costFieldWithKey[destID], _directionFieldWithKey[destID]);
        GetDirectionToUID(destID, key, direction, mode);
    }
}

void UnivFFviaFM::GetDirectionToUID(int destID, long int key, Point & direction)
{
    //assert(key > 0 && key < _nPoints);
    if(key <= 0 || key >= _nPoints) {
        direction._x = 0.;
        direction._y = 0.;
        return;
    }
    if((_gridCode[key] == OUTSIDE) || (_gridCode[key] == WALL)) {
        //bresenham line (treppenstruktur) GetKeyAtPoint yields gridpoint next to edge, although position is on edge
        //find a key that belongs domain (must be one left or right and second one below or above)
        if((key + 1 <= _grid->GetnPoints()) && (_gridCode[key + 1] != OUTSIDE) &&
           (_gridCode[key + 1] != WALL)) {
            key = key + 1;
        } else if(
            (key - 1 >= 0) && (_gridCode[key - 1] != OUTSIDE) && (_gridCode[key - 1] != WALL)) {
            key = key - 1;
        } else if(
            (key >= _grid->GetiMax()) && (_gridCode[key - _grid->GetiMax()] != OUTSIDE) &&
            (_gridCode[key - _grid->GetiMax()] != WALL)) {
            key = key - _grid->GetiMax();
        } else if(
            (key < _grid->GetnPoints() - _grid->GetiMax()) &&
            (_gridCode[key + _grid->GetiMax()] != OUTSIDE) &&
            (_gridCode[key + _grid->GetiMax()] != WALL)) {
            key = key + _grid->GetiMax();
        } else {
            // Log->Write("ERROR:\t In GetDirectionToUID (3 args)");
        }
    }
    if(_directionFieldWithKey.count(destID) == 1 && _directionFieldWithKey[destID]) {
        direction = _directionFieldWithKey[destID][key];
    } else if(_doors.count(destID) > 0) {
        //free memory if needed
        if(_costFieldWithKey.count(destID) == 1 && _costFieldWithKey[destID]) {
            delete[] _costFieldWithKey[destID];
        }
        //allocate memory
        _costFieldWithKey[destID] = new double[_nPoints];
        if(_user == DISTANCE_AND_DIRECTIONS_USED) {
            _directionFieldWithKey[destID] = new Point[_nPoints];
        } else {
            _directionFieldWithKey[destID] = nullptr;
        }

        //calculate destID's fields and call function
        AddTarget(destID, _costFieldWithKey[destID], _directionFieldWithKey[destID]);
        GetDirectionToUID(destID, key, direction);
    }
}

void UnivFFviaFM::GetDirectionToUID(int destID, const Point & pos, Point & direction)
{
    GetDirectionToUID(destID, _grid->GetKeyAtPoint(pos), direction);
}

double UnivFFviaFM::GetDistance2WallAt(const Point & pos)
{
    if(_useWallDistances || (_speedmode == FF_WALL_AVOID)) {
        if(_costFieldWithKey[0]) {
            return _costFieldWithKey[0][_grid->GetKeyAtPoint(pos)];
        }
    }
    return std::numeric_limits<double>::max();
}

void UnivFFviaFM::GetDir2WallAt(const Point & pos, Point & p)
{
    if(_useWallDistances || (_speedmode == FF_WALL_AVOID)) {
        if(_directionFieldWithKey[0]) {
            p = _directionFieldWithKey[0][_grid->GetKeyAtPoint(pos)];
        }
    } else {
        p = Point(0.0, 0.0);
    }
}

/* Log:
 * todo:
 *   - implement error treatment: extend fctns to throw errors and handle them
 *   - error treatment will be advantageous, if calculation of FFs can be postponed
 *                                           to be done in Simulation::RunBody, where
 *                                           all cores are available
 *   - (WIP) fill subroom* array with correct values
 *   */
