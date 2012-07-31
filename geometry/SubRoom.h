/* 
 * File:   SubRoom.h
 * Author: andrea
 *
 * Created on 8. Oktober 2010, 10:56
 */

#ifndef _SUBROOM_H
#define	_SUBROOM_H

#include "Wall.h"
#include "../pedestrian/Pedestrian.h"
#include "../general/Macros.h"

class Obstacle;

/************************************************************
 SubRoom
 ************************************************************/

class SubRoom {
private:
	int pID; // gleich Index nur innerhalb des Rooms eindeutig
	int pRoomID;
	vector<Pedestrian*> pPeds; // Fussgänger
	vector<Obstacle*> pObstacles; // obstacles
	vector<int> pGoalIDs; // Indizes aller gaols (Crossings und Transitions)
	double pArea; // Fläche des SubRooms
	double pClosed;

protected:
	vector<Wall> pWalls; // Wände
	vector<Point> pPoly; // Alle Eckpunkte des Unter-Raums als Polygon
public:
	// Konstruktoren
	SubRoom();
	SubRoom(const SubRoom& orig);
	virtual ~SubRoom();

	// Setter -Funktionen
	void SetSubRoomID(int ID);
	void SetRoomID(int ID);
	void SetAllWalls(const vector<Wall>& walls);
	void SetWall(const Wall& wall, int index);
	void SetPolygon(const vector<Point>& poly);
	void SetAllPedestrians(const vector<Pedestrian*>& peds);
	void SetPedestrian(Pedestrian* ped, int index);
	void SetArea(double a);
	void SetClosed(double c);

	// Getter - Funktionen
	int GetSubRoomID() const;
	int GetAnzWalls() const;
	const vector<Wall>& GetAllWalls() const;
	const Wall GetWall(int index) const;
	const vector<Point>& GetPolygon() const;
	int GetAnzPedestrians() const;
	const vector<Pedestrian*>& GetAllPedestrians() const;
	const vector<Obstacle*>& GetAllObstacles() const;
	Pedestrian* GetPedestrian(int index) const;
	int GetAnzGoalIDs() const;
	const vector<int>& GetAllGoalIDs() const;
	int GetGoalID(int index) const;
	int GetRoomID() const;
	int GetUID() const; // unique identifier for this subroom
	double GetClosed() const ;
	double GetArea() const;


	// Sonstiges
	void AddWall(const Wall& w);
	void AddObstacle(Obstacle* obs);
	void DeleteWall(int index);
	void AddPedestrian(Pedestrian* ped);
	void DeletePedestrian(int index);
	void AddGoalID(int ID);
	void RemoveGoalID(int ID);
	void CalculateArea();
	bool IsDirectlyConnectedWith(const SubRoom* sub) const;

	// Ein-Ausgabe
	void LoadWall(string line); // wird in LoadNormalSubRooom() bzw LoadStair() benötigt
	virtual string WriteSubRoom() const = 0;

	// virtuelle Funktionen
	virtual void WriteToErrorLog() const = 0; // Testausgabe nach Log verschieden für Stair und normal
	virtual void ConvertLineToPoly(vector<Line*> goals) = 0; // Erstellt einen Polygonzug aus Walls und Übergängen
	virtual bool IsInSubRoom(Pedestrian* ped) const; // prüft, ob Fußgänger noch im SubRoom
	virtual bool IsInSubRoom(const Point& ped) const = 0;


	// MPI:
	void ClearAllPedestrians();
};

/************************************************************
 NormalSubroom
 ************************************************************/

class NormalSubRoom : public SubRoom {
private:

	int WhichQuad(const Point& vertex, const Point& hitPos) const; // wird in IsInSubRoom benötigt
	double Xintercept(const Point& point1, const Point& point2, double hitY) const;
public:
	NormalSubRoom();
	NormalSubRoom(const NormalSubRoom& orig);
	virtual ~NormalSubRoom();

	string WriteSubRoom() const;
	void WriteToErrorLog() const;
	void ConvertLineToPoly(vector<Line*> goals);
	bool IsInSubRoom(const Point& ped) const;
};

/************************************************************
 Stair
 ************************************************************/

class Stair : public SubRoom {
private:
	Point pUp; // Punkt der den oberen Bereich der Treppe markiert
	Point pDown; // Punkt der den unteren Bereich der Treppe markiert

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

	// Sonstiges
	bool IsUpStairs() const; // Fußgänger bewegt sich die Treppe runter
	bool IsDownStair() const; // Fußgänger bewegt sich die Treppe rauf

	string WriteSubRoom() const;
	virtual void WriteToErrorLog() const;
	virtual void ConvertLineToPoly(vector<Line*> goals);
	bool IsInSubRoom(const Point& ped) const;
};

#endif	/* _SUBROOM_H */

