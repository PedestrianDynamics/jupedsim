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
#include "../routing/Routing.h"
#include "../pedestrian/Pedestrian.h"
#include "../geometry/Transition.h"
#include "../mpi/LCGrid.h"




class Building {
private:
    string pCaption; // Name des Projekts
    Routing* pRouting;
    vector<Room*> pRooms; // Liste der Räume
    vector<Pedestrian*> pAllPedestians;
    LCGrid* pLinkedCellGrid;
    // this is only for the Hermes Project
    //bool pSubroomConnectionMap[16][130][16][130];

    // pedestrians pathway
    bool pSavePathway;
    ofstream PpathWayStream;

    // wird nur innerhalb von Building benötigt
    void StringExplode(string str, string separator, vector<string>* results);

public:
    // Konstruktor
    Building();
    Building(const Building& orig);
    virtual ~Building();

    // Setter -Funktionen
    void SetCaption(string s);
    void SetRouting(Routing* r);
    void SetAllRooms(const vector<Room*>& rooms);
    void SetRoom(Room* room, int index);
    /// delete the ped from the ped vector
    void DeletePedestrian(Pedestrian* ped);
    /// delete the ped from the simulation
    void DeletePedFromSim(Pedestrian* ped);
    void AddPedestrian(Pedestrian* ped);

    // Getter - Funktionen
    string GetCaption() const;
    Routing* GetRouting() const;
    const vector<Room*>& GetAllRooms() const;
    const vector<Pedestrian*>& GetAllPedestrians() const;
    Pedestrian* GetPedestrian( int pedID) const;
    int GetAnzRooms() const;
    Room* GetRoom(int index) const; // Gibt Raum der Nummer "index" zurueck
    Room* GetRoom(string caption)const;
    Transition* GetTransition(string caption) const;
    Crossing* GetGoal(string caption) const;
    int GetAnzPedestrians() const;
    LCGrid* GetGrid() const;

    // Sonstiges
    void InitGeometry();
    void InitGrid(double cellSize);
    //void InitRoomsAndSubroomsMap();
    void InitPhiAllPeds(); // Initialisiert die Ausrichtung der Ellipse
    void InitSavePedPathway(string filename);
    void AddRoom(Room* room);
    void Update();
    void UpdateGrid();
    void AddSurroundingRoom(); // add a final room (outside or world), that encompasses the complete geometry
    void  DumpSubRoomInRoom(int roomID, int subID);


    // Ein-Ausgabe
    void LoadBuilding(string filename); // Laedt Geometrie-Datei
    void LoadTrafficInfo(string filename);
    void LoadRoutingInfo(string filename);
    void WriteToErrorLog() const;

	void CleanUpTheScene();


	// saving computation
	//bool IsDirectlyConnected(int room1, int subroom1,int room2, int subroom2);

};

#endif	/* _BUILDING_H */

