/**
 * File:   Building.h
 *
 *
 * Created on 1. October 2010, 09:25
 *
 * @section LICENSE
 * This file is part of JuPedSim.
 *
 * JuPedSim is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * JuPedSim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with JuPedSim. If not, see <http://www.gnu.org/licenses/>.
 *
 * @section DESCRIPTION
 *
 *
 *
 */

#ifndef _BUILDING_H
#define	_BUILDING_H

#include <string>
#include <vector>
#include <fstream>
#include <cfloat>
#include <map>

#include "Room.h"
#include "NavLine.h"
#include "Transition.h"
#include "Hline.h"
#include "Obstacle.h"
#include "Goal.h"

class RoutingEngine;
class Pedestrian;
class Transition;
class LCGrid;


class Building {
private:
    std::string _caption;
    std::string _projectFilename;
    std::string _projectRootDir;
    RoutingEngine* _routingEngine;
    LCGrid* _linkedCellGrid;
    std::vector<Room*> _rooms;
    std::vector<Pedestrian*> _allPedestians;

    std::map<int, Crossing*> _crossings;
    std::map<int, Transition*> _transitions;
    std::map<int, Hline*> _hLines;
    std::map<int, Goal*>_goals;

    /// pedestrians pathway
    bool _savePathway;
    std::ofstream _pathWayStream;

public:
    /// constructor
    Building();
    /// destructor
    virtual ~Building();


    void SetCaption(const std::string& s);
    void SetRoutingEngine(RoutingEngine* r);
    void SetRoom(Room* room, int index);
    /// delete the ped from the ped vector
    void DeletePedestrian(Pedestrian* ped);
    /// delete the ped from the simulation
    void DeletePedFromSim(Pedestrian* ped);
    void AddPedestrian(Pedestrian* ped);


    std::string GetCaption() const;
    RoutingEngine* GetRoutingEngine() const;
    const std::vector<Room*>& GetAllRooms() const;
    const std::vector<Pedestrian*>& GetAllPedestrians() const;
    Pedestrian* GetPedestrian( int pedID) const;
    int GetNumberOfRooms() const;
    int GetNumberOfGoals()const;
    Room* GetRoom(int index) const;
    Room* GetRoom(std::string caption)const;

    //TODO: implement the same methods for Crossings and Hlines
    Transition* GetTransition(std::string caption) const;
    Transition* GetTransition(int id) ;

    /**
     * Not implemented
     */
    Crossing* GetCrossing(int id);

    /**
     * Not implemented
     */
    Hline* GetHline(int id);

    /**
     * @return true if the two segments are visible from each other.
     * Alls walls and transitions and crossings are used in this check.
     * The use of hlines is optional, because they are not real, can can be considered transparent
     */
    bool IsVisible(Line* l1, Line* l2, bool considerHlines=false);

    /**
     * @return true if the two points are visible from each other.
     * Alls walls and transitions and crossings are used in this check.
     * The use of hlines is optional, because they are not real, can be considered transparent
     */
    bool IsVisible(const Point& p1, const Point& p2, bool considerHlines=false);


    /**
     * @return a crossing or a transition matching the given caption.
     * Return NULL if none is found
     */
    Crossing* GetTransOrCrossByName(std::string caption) const;


    /**
     * @return a crossing or a transition matching the given id.
     * Return NULL if none is found
     */
    Crossing* GetTransOrCrossByID(int id) const;


    //TOD0: rename later to GetGoal
    Goal* GetFinalGoal(int id);

    int GetNumberOfPedestrians() const;

    /**
     * @return the linked-cell grid used for spatial query
     */
    LCGrid* GetGrid() const;

    // Sonstiges
    bool InitGeometry();
    void InitGrid(double cellSize);
    //void InitRoomsAndSubroomsMap();
    void InitPhiAllPeds(double pDt); // initialize the direction of the ellipses
    void InitSavePedPathway(const std::string &filename);
    void AddRoom(Room* room);
    void Update();
    void UpdateVerySlow();
    void UpdateGrid();
    void AddSurroundingRoom(); // add a final room (outside or world), that encompasses the complete geometry
    void DumpSubRoomInRoom(int roomID, int subID);

	const std::map<int, Crossing*>& GetAllCrossings() const;
	const std::map<int, Transition*>& GetAllTransitions() const;
	const std::map<int, Hline*>& GetAllHlines() const;
	const std::map<int, Goal*>& GetAllGoals() const;

	void AddCrossing(Crossing* line);
	void AddTransition(Transition* line);
	void AddHline(Hline* line);
	void AddGoal(Goal* goal);

	const std::string& GetProjectRootDir() const;
	const std::string& GetProjectFilename() const;
	void SetProjectFilename(const std::string &filename) ;
	void SetProjectRootDir(const std::string &filename);
    bool LoadBuildingFromFile(const std::string &filename="");
    void LoadTrafficInfo();
    void LoadRoutingInfo(const std::string &filename);
    void WriteToErrorLog() const;

	void CleanUpTheScene();

	/**
	 * Check the scenario for possible errors and
	 * output user specific informations.
	 */
	void SanityCheck();


private:
	void StringExplode(std::string str, std::string separator, std::vector<std::string>* results);

};

#endif	/* _BUILDING_H */

