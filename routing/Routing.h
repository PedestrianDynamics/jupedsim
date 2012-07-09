/*
 * File:   Routing.h
 * Author: andrea
 *
 * Created on 11. November 2010, 12:55
 */

#ifndef _ROUTING_H
#define	_ROUTING_H

#include "../geometry/Crossing.h"
#include "../geometry/Transition.h"
#include "../pedestrian/Pedestrian.h"
#include "../geometry/Room.h"

class Building;

/************************************************************
 Routing
 ************************************************************/

class Routing {
private:
	vector<Crossing*> goals;
public:
	Routing();
	Routing(const Routing& orig);
	virtual ~Routing();

	// Getter-Funktionen
	const vector<Crossing*>& GetAllGoals() const;
	Crossing* GetGoal(int index) const;
	int GetAnzGoals() const;
	// Sonstiges
	void AddGoal(Crossing* line);
	void LoadCrossings(ifstream* buildingfile, int* i, Room* room);
	void LoadCrossing(ifstream* buildingfile, int* i, Room* room);
	void LoadTransitions(ifstream* buildingfile, int* i, vector<Room*> rooms);
	void LoadTransition(ifstream* buildingfile, int* i, vector<Room*> rooms);

	void LoadOrientationLines(ifstream* buildingfile, int* i, Room* room);
	void LoadOrientationLine(ifstream* buildingfile, int* i, Room* room);

	// Ausgabe
	void WriteToErrorLog() const;

	// virtuelle Funktionen
	virtual int FindExit(Pedestrian* p) = 0;
	virtual void Init(Building* b) = 0;

};

#endif	/* _ROUTING_H */

