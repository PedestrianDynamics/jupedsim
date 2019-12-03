/**
 * \file        Building.h
 * \date        Oct 1, 2010
 * \version     v0.7
 * \copyright   <2009-2015> Forschungszentrum Jülich GmbH. All rights reserved.
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
 *
 *
 **/


#ifndef _BUILDING_H
#define _BUILDING_H

#include "Goal.h"
#include "Hline.h"
#include "NavLine.h"
#include "Obstacle.h"
#include "Room.h"
#include "Transition.h"

#include <cfloat>
#include <fstream>
#include <map>
#include <memory>
#include <string>
#include <vector>

class RoutingEngine;
class Pedestrian;
class Transition;
class LCGrid;
class ForceModel;
class PedDistributor;

class Building
{
private:
    std::string _caption;
    std::string _projectFilename;
    std::string _projectRootDir;
    std::string _geometryFilename;
    RoutingEngine * _routingEngine;
    LCGrid * _linkedCellGrid;
    std::vector<Pedestrian *> _allPedestians;

    std::map<int, std::unique_ptr<Room>> _rooms;
    std::map<int, Crossing *> _crossings;
    std::map<int, Transition *> _transitions;
    std::map<int, Hline *> _hLines;
    std::map<int, Goal *> _goals;

    /// pedestrians pathway
    bool _savePathway;
    std::ofstream _pathWayStream;

public:
    /// constructor
    Building();
    Building(const std::string &, const std::string &, RoutingEngine &, PedDistributor &, double);
    /// destructor
    virtual ~Building();

    void SetCaption(const std::string & s);
    void SetRoutingEngine(RoutingEngine * r);

    /// delete the ped from the ped vector
    void DeletePedestrian(Pedestrian *& ped);
    /// delete the ped from the simulation
    void AddPedestrian(Pedestrian * ped);
    void GetPedestrians(int room, int subroom, std::vector<Pedestrian *> & peds) const;

    std::string GetCaption() const;
    RoutingEngine * GetRoutingEngine() const;
    const std::map<int, std::unique_ptr<Room>> & GetAllRooms() const;
    const std::vector<Pedestrian *> & GetAllPedestrians() const;
    Pedestrian * GetPedestrian(int pedID) const;
    int GetNumberOfRooms() const;
    int GetNumberOfGoals() const;
    Room * GetRoom(int index) const;
    Room * GetRoom(std::string caption) const;

    Transition * GetTransition(std::string caption) const;
    Transition * GetTransition(int id);

    /**
      * Not implemented
      */
    Crossing * GetCrossing(int id);

    /**
      * Not implemented
      */
    Hline * GetHline(int id);

    /**
      * return the subroom with the corresponding unique identifier
      * @param uid ,the unique identifier
      * @return NULL if no exists with that identifier.
      */
    SubRoom * GetSubRoomByUID(int uid);

    /**
      * @return true if the two segments are visible from each other.
      * Alls walls and transitions and crossings are used in this check.
      * The use of hlines is optional, because they are not real, can can be considered transparent
      */
    //bool IsVisible(Line* l1, Line* l2, bool considerHlines=false);

    /**
      * @return true if the two points are visible from each other.
      * Alls walls and transitions and crossings are used in this check.
      * The use of hlines is optional, because they are not real, can be considered transparent
      */
    bool IsVisible(
        const Point & p1,
        const Point & p2,
        const std::vector<SubRoom *> & subrooms,
        bool considerHlines = false);

    /**
      * @return a crossing or a transition matching the given caption.
      * Return NULL if none is found
      */
    Crossing * GetTransOrCrossByName(std::string caption) const;

    /**
      * @return a crossing or a transition or a hline matching the given uid.
      * Return NULL if none is found
      */
    Hline * GetTransOrCrossByUID(int uid) const;

    /**
      * @return the transition matching the uid
      */
    Transition * GetTransitionByUID(int uid) const;

    //TOD0: rename later to GetGoal
    Goal * GetFinalGoal(int id);

    /**
      * @return the linked-cell grid used for spatial query
      */
    LCGrid * GetGrid() const;

    // convenience methods
    bool InitGeometry();
    void InitGrid(double cellSize);
    //void InitRoomsAndSubroomsMap();
    void InitSavePedPathway(const std::string & filename);
    void AddRoom(Room * room);
    void UpdateGrid();
    void
    AddSurroundingRoom(); // add a final room (outside or world), that encompasses the complete geometry

    const std::map<int, Crossing *> & GetAllCrossings() const;
    const std::map<int, Transition *> & GetAllTransitions() const;
    const std::map<int, Hline *> & GetAllHlines() const;
    const std::map<int, Goal *> & GetAllGoals() const;

    void AddCrossing(Crossing * line);
    void AddTransition(Transition * line);
    void AddHline(Hline * line);
    void AddGoal(Goal * goal);

    const std::string & GetProjectRootDir() const;
    const std::string & GetProjectFilename() const;
    const std::string & GetGeometryFilename() const;
    void SetProjectFilename(const std::string & filename);
    void SetProjectRootDir(const std::string & filename);

    /**
      * Load and parse the geometry file into the building object.
      * If no geometry file is provided, one is searched in the the project file
      *
      * @param filename, the geometry file
      */
    bool LoadGeometry(const std::string & geometryfile = "");

    /**
      * Write the geometry to the given file.
      * That will be useful in the geometry editor.
      * @param filename the relative location of the file
      * @return true if everything went fine.
      */
    bool SaveGeometry(const std::string & filename);

    bool LoadTrafficInfo();
    bool LoadRoutingInfo(const std::string & filename);
    void WriteToErrorLog() const;

    /**
      * Check the scenario for possible errors and
      * output user specific informations.
      */
    bool SanityCheck();
    // ---
    double _xMin;
    double _xMax;
    double _yMin;
    double _yMax;


private:
    void StringExplode(std::string str, std::string separator, std::vector<std::string> * results);
};

#endif /* _BUILDING_H */
