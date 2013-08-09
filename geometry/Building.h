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


class RoutingEngine;
class Pedestrian;
class Transition;
class LCGrid;



class Building {
private:
    std::string _caption;
    RoutingEngine* _routingEngine;
    LCGrid* _linkedCellGrid;
    std::vector<Room*> _rooms;
    std::vector<Pedestrian*> _allPedestians;
    /// pedestrians pathway
    bool _savePathway;
    std::ofstream _pathWayStream;

    std::map<int, Crossing*> _crossings;
    std::map<int, Transition*> _transitions;
    std::map<int, Hline*> _hLines;


public:
    // Konstruktor
    Building();
    virtual ~Building();

    // Setter -Funktionen
    void SetCaption(std::string s);
    void SetRoutingEngine(RoutingEngine* r);
    void SetRoom(Room* room, int index);
    /// delete the ped from the ped vector
    void DeletePedestrian(Pedestrian* ped);
    /// delete the ped from the simulation
    void DeletePedFromSim(Pedestrian* ped);
    void AddPedestrian(Pedestrian* ped);

    // Getter - Funktionen
    std::string GetCaption() const;
    RoutingEngine* GetRoutingEngine() const;
    const std::vector<Room*>& GetAllRooms() const;
    const std::vector<Pedestrian*>& GetAllPedestrians() const;
    Pedestrian* GetPedestrian( int pedID) const;
    int GetNumberOfRooms() const;
    int GetNumberOfGoals()const;
    Room* GetRoom(int index) const;
    Room* GetRoom(std::string caption)const;
    Transition* GetTransition(std::string caption) const;
    Transition* GetTransition(int id) ;
    Crossing* GetGoal(std::string caption) const;

    //FIXME: obsolete should get rid of this method
    Crossing* GetGoal(int id);

    int GetNumberOfPedestrians() const;
    LCGrid* GetGrid() const;

    // Sonstiges
    void InitGeometry();
    void InitGrid(double cellSize);
    //void InitRoomsAndSubroomsMap();
    void InitPhiAllPeds(double pDt); // initialize the direction of the ellipses
    void InitSavePedPathway(std::string filename);
    void AddRoom(Room* room);
    void Update();
    void UpdateGrid();
    void AddSurroundingRoom(); // add a final room (outside or world), that encompasses the complete geometry
    void DumpSubRoomInRoom(int roomID, int subID);

	const std::map<int, Crossing*>& GetAllCrossings() const;
	const std::map<int, Transition*>& GetAllTransitions() const;
	const std::map<int, Hline*>& GetAllHlines() const;

	void AddCrossing(Crossing* line);
	void AddTransition(Transition* line);
	void AddHline(Hline* line);


    // Ein-Ausgabe
    void LoadBuilding(std::string filename); // Laedt Geometrie-Datei
    void LoadTrafficInfo(std::string filename);
    void LoadRoutingInfo(std::string filename);
    void WriteToErrorLog() const;

	void CleanUpTheScene();


	// saving computation
	//bool IsDirectlyConnected(int room1, int subroom1,int room2, int subroom2);

private:
	void StringExplode(std::string str, std::string separator, std::vector<std::string>* results);

};

#endif	/* _BUILDING_H */

