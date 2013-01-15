/**
 * File:   SubRoom.h
 *
 * Created on 8. October 2010, 10:56
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

#ifndef _SUBROOM_H
#define	_SUBROOM_H


#include "Line.h"
#include "Wall.h"
#include "NavLine.h"
#include "Crossing.h"
#include "../general/Macros.h"
#include "Obstacle.h"


class Transition;
class Hline;

/************************************************************
 SubRoom
 ************************************************************/

class SubRoom {
private:
	int pID;
	int pRoomID;
	vector<Obstacle*> pObstacles; // obstacles
	vector<int> pGoalIDs; // all navigation lines contained in this subroom
	double pArea;
	double pClosed;

	//different types of navigation lines
	vector<Crossing*> pCrossings;
	vector<Transition*> pTransitions;
	vector<Hline*> pHlines;

	static int UID;
	int pUID;

	//defined by: Z = Ax + By + C
	double pPlanEquation[3];

protected:
	vector<Wall> pWalls;
	vector<Point> pPoly; // Polygon representation of the subroom

public:

	// constructors
	SubRoom();
	SubRoom(const SubRoom& orig);
	virtual ~SubRoom();

	// Set-methods
	void SetSubRoomID(int ID);
	void SetRoomID(int ID);
	void SetAllWalls(const vector<Wall>& walls);
	void SetWall(const Wall& wall, int index);
	void SetPolygon(const vector<Point>& poly);
	void SetArea(double a);
	void SetClosed(double c);
	void SetPlanEquation(double A, double B, double C);

	// Get-methods
	int GetSubRoomID() const;
	int GetAnzWalls() const;
	const vector<Wall>& GetAllWalls() const;
	const Wall GetWall(int index) const;
	const vector<Point>& GetPolygon() const;
	int GetAnzPedestrians() const;
	const vector<Obstacle*>& GetAllObstacles() const;
	int GetAnzGoalIDs() const;
	const vector<int>& GetAllGoalIDs() const;
	int GetGoalID(int index) const;
	int GetRoomID() const;
	int GetUID() const; // unique identifier for this subroom
	double GetClosed() const ;
	double GetArea() const;
	Point GetCentroid() const;
	const double * GetPlanEquation () const;
	double GetElevation(const Point & p1);

	//navigation
	void AddCrossing(Crossing* line);
	void AddTransition(Transition* line);
	void AddHline(Hline* line);

	const vector<Crossing*>& GetAllCrossings() const;
	const vector<Transition*>& GetAllTransitions() const;
	const vector<Hline*>& GetAllHlines() const;
	const Crossing* GetCrossing(int i) const;
	const Transition* GetTransition(int i) const;
	const Hline* GetHline(int i) const;


	// Misc
	void AddWall(const Wall& w);
	void AddObstacle(Obstacle* obs);
	void DeleteWall(int index);
	void DeletePedestrian(int index);
	void AddGoalID(int ID);
	void RemoveGoalID(int ID);
	void CalculateArea();
	bool IsDirectlyConnectedWith(const SubRoom* sub) const;
	bool IsVisible(Line* l1, Line* l2, bool considerHlines=false);
	bool IsVisible(const Point& p1, const Point& p2, bool considerHlines=false);

	/// @see LoadNormalSubRooom()  @see  LoadStair()
	void LoadWall(string line);

	// virtual functions
	virtual string WriteSubRoom() const = 0;
	virtual void WriteToErrorLog() const = 0;
	virtual string WritePolyLine() const=0;

	/// convert all walls and transitions(doors) into a polygon representing the subroom
	virtual void ConvertLineToPoly(vector<Line*> goals) = 0;

	///check whether the pedestrians is still in the subroom
	virtual bool IsInSubRoom(const Point& ped) const = 0;


	// MPI:
	void ClearAllPedestrians();
};

/************************************************************
 NormalSubroom
 ************************************************************/

class NormalSubRoom : public SubRoom {
private:

	///@see IsInSubRoom
	int WhichQuad(const Point& vertex, const Point& hitPos) const;
	double Xintercept(const Point& point1, const Point& point2, double hitY) const;

public:
	NormalSubRoom();
	NormalSubRoom(const NormalSubRoom& orig);
	virtual ~NormalSubRoom();

	string WriteSubRoom() const;
	string WritePolyLine() const;

	void WriteToErrorLog() const;
	void ConvertLineToPoly(vector<Line*> goals);
	bool IsInSubRoom(const Point& ped) const;
};

/************************************************************
 Stair
 ************************************************************/

class Stair : public NormalSubRoom {
private:
	Point pUp; /// Punkt der den oberen Bereich der Treppe markiert
	Point pDown; /// Punkt der den unteren Bereich der Treppe markiert

	const Point* CheckCorner(const Point** otherPoint, const Point** aktPoint, const Point* nextPoint);
public:
	Stair();
	Stair(const Stair& orig);
	virtual ~Stair();

	// Setter-Funktionen
	void SetUp(const Point& p);
	void SetDown(const Point& p);

	// Getter-Funktionen
	const Point& GetUp() const;
	const Point& GetDown() const;

	/// pedestrians are going the stairs downwards
	bool IsUpStairs() const;
	/// pedestrians are going the stairs upwards
	bool IsDownStair() const;

	string WriteSubRoom() const;
	string WritePolyLine() const;
	virtual void WriteToErrorLog() const;
	virtual void ConvertLineToPoly(vector<Line*> goals);
	bool IsInSubRoom(const Point& ped) const;
};

#endif	/* _SUBROOM_H */

