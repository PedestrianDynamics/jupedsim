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
#include <omp.h>

#include "Room.h"
#include "../general/Macros.h"


class Building {
private:
    string pCaption; // Name des Projekts
    vector<Room*> pRooms; // Liste der Räume

    /// pedestrians pathway
    bool pSavePathway;
    ofstream PpathWayStream;

	map<int, Crossing*> _crossings;
	map<int, Transition*> _transitions;
	map<int, Hline*> _hLines;


public:
    // Konstruktor
    Building();
    virtual ~Building();

    // Setter -Funktionen
    void SetCaption(string s);
    void SetRoutingEngine(RoutingEngine* r);
    void SetAllRooms(const vector<Room*>& rooms);
    void SetRoom(Room* room, int index);
    /// delete the ped from the ped vector
    void DeletePedestrian(Pedestrian* ped);
    /// delete the ped from the simulation
    void DeletePedFromSim(Pedestrian* ped);
    void AddPedestrian(Pedestrian* ped);

    // Getter - Funktionen
    string GetCaption() const;
    RoutingEngine* GetRoutingEngine() const;
    const vector<Room*>& GetAllRooms() const;
    const vector<Pedestrian*>& GetAllPedestrians() const;
    Pedestrian* GetPedestrian( int pedID) const;
    int GetAnzRooms() const;
    int GetGoalsCount()const;
    Room* GetRoom(int index) const; // Gibt Raum der Nummer "index" zurueck
    Room* GetRoom(string caption)const;
    Transition* GetTransition(string caption) const;
    Transition* GetTransition(int id) ;
    Crossing* GetGoal(string caption) const;

    //FIXME: obsolete shold get rid of this method
    Crossing* GetGoal(int id);

    int GetAnzPedestrians() const;
    LCGrid* GetGrid() const;

    // Sonstiges
    void InitGeometry();
    void InitGrid(double cellSize);
    //void InitRoomsAndSubroomsMap();
    void InitPhiAllPeds(double pDt); // initialize the direction of the ellipses
    void InitSavePedPathway(string filename);
    void AddRoom(Room* room);
    void Update();
    void UpdateGrid();
    void AddSurroundingRoom(); // add a final room (outside or world), that encompasses the complete geometry
    void  DumpSubRoomInRoom(int roomID, int subID);

	const map<int, Crossing*>& GetAllCrossings() const;
	const map<int, Transition*>& GetAllTransitions() const;
	const map<int, Hline*>& GetAllHlines() const;

	void AddCrossing(Crossing* line);
	void AddTransition(Transition* line);
	void AddHline(Hline* line);



    // Ein-Ausgabe
    void LoadBuilding(string filename); // Laedt Geometrie-Datei
    void LoadTrafficInfo(string filename);
    void LoadRoutingInfo(string filename);
    void WriteToErrorLog() const;

	void CleanUpTheScene();



private:
	// wird nur innerhalb von Building benötigt
	void StringExplode(string str, string separator, vector<string>* results);

};

#endif	/* _BUILDING_H */

