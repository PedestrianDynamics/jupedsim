/*
 * Obstacle.h
 *
 *  Created on: Jul 31, 2012
 *      Author: Ulrich Kemloh
 */

#ifndef OBSTACLE_H_
#define OBSTACLE_H_

#include<string>
#include<vector>
using std::string;
using std::vector;

class Wall;
class Point;

class Obstacle {

private:
	double pClosed;
	double pHeight;
	int pID;
	string pCaption;
	vector<Wall> pWalls;
	vector<Point> pPoly;

public:
	Obstacle();
	virtual ~Obstacle();

	string GetCaption() const;
	void SetCaption(string caption);

	double GetClosed() const;
	void SetClosed(double closed);

	double GetHeight() const;
	void SetHeight(double height);

	int GetId() const;
	void SetId(int id);

	void AddWall(const Wall& w);
	const vector<Wall>& GetAllWalls() const;

	bool Contains(const Point& ped) const;

	void ConvertLineToPoly();

	int WhichQuad(const Point& vertex, const Point& hitPos) const;

	const vector<Point>&  GetPolygon() const;

	// x-Koordinate der Linie von einer Eccke zur nächsten
	double Xintercept(const Point& point1, const Point& point2,
			double hitY) const;
	string Write();
};

#endif /* OBSTACLE_H_ */
