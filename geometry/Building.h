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

class Crossing;
class Transition;
class Hline;
class Room;

class Building {
private:
    std::string pCaption; // Name des Projekts
    std::vector<Room*> pRooms; // Liste der Räume


	std::map<int, Crossing*> _crossings;
	std::map<int, Transition*> _transitions;
	std::map<int, Hline*> _hLines;

	std::vector <std::vector<Point>> obstpoly; //Obstacle polygons
	std::vector <std::vector<Point>> geopoly; // geometry polygons
public:
    // Konstruktor
    Building();
    virtual ~Building();

    // Setter -Funktionen
    void SetCaption(std::string s);
    void SetAllRooms(const std::vector<Room*>& rooms);
    void SetRoom(Room* room, int index);

    // Getter - Funktionen
    std::string GetCaption() const;
    const std::vector<Room*>& GetAllRooms() const;
    int GetAnzRooms() const;
    int GetGoalsCount()const;
    Room* GetRoom(int index) const;
    Room* GetRoom(std::string caption)const;
    Transition* GetTransition(std::string caption) const;
    Transition* GetTransition(int id);
    Crossing* GetGoal(std::string caption) const;

    //FIXME: obsolete shold get rid of this method
    Crossing* GetGoal(int id);


    // Sonstiges
    void InitGeometry();
    void AddRoom(Room* room);
    void AddSurroundingRoom(); // add a final room (outside or world), that encompasses the complete geometry
    void  DumpSubRoomInRoom(int roomID, int subID);

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



private:
	// wird nur innerhalb von Building benötigt
	void StringExplode(std::string str, std::string separator, std::vector<std::string>* results);

};

#endif	/* _BUILDING_H */

