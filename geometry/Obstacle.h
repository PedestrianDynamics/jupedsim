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
};

#endif /* OBSTACLE_H_ */
