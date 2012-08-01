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
#include "../geometry/Hline.h"
#include "../pedestrian/Pedestrian.h"
#include "../geometry/Room.h"

class Building;

/************************************************************
 Routing
 ************************************************************/

class Routing {
private:
	vector<Crossing*> pCrossings;
	vector<Crossing*> goals;
	vector<Transition*> pTransitions;
	vector<Hline*> pHlines;

public:
	Routing();
	Routing(const Routing& orig);
	virtual ~Routing();

	// Getter-Funktionen
	const vector<Crossing*>& GetAllGoals() const;
	Crossing* GetGoal(int index) const;
	int GetAnzGoals() const;

	// Sonstiges
	void AddCrossing(Crossing* line);
	void AddTransition(Transition* line);
	void AddHline(Hline* line);

	const vector<Crossing*>& GetAllCrossings() const;
	const vector<Transition*>& GetAllTransitions() const;
	const vector<Hline*>& GetAllHlines() const;

	// Ausgabe
	void WriteToErrorLog() const;

	// virtuelle Funktionen
	virtual int FindExit(Pedestrian* p) = 0;
	virtual void Init(Building* b) = 0;


	//FIXME: to be deleted
	void  AddGoal(Crossing* line);
};

#endif	/* _ROUTING_H */

