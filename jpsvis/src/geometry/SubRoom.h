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

#include <vector>
#include <string>

class Transition;
class Hline;
class Obstacle;
class Crossing;

#ifdef _SIMULATOR
	class Pedestrian;
#endif


/************************************************************
 SubRoom
 ************************************************************/

class SubRoom {
private:
	/// the id set using the SetID method
	int _id;
	/// the unique id resulting from the count of all subrooms in the system
	int _uid;
	int _roomID;
	std::vector<int> _goalIDs; // all navigation lines contained in this subroom
	double _area;
	double _closed;
	//defined by: Z = Ax + By + C
	double _planeEquation[3];
	double _cosAngleWithHorizontalPlane;
	std::string _type;

	std::vector<Obstacle*> _obstacles; // obstacles

	//different types of navigation lines
	std::vector<Crossing*> _crossings;
	std::vector<Transition*> _transitions;
	std::vector<Hline*> _hlines;

	/// storing and incrementing the total number of subrooms
	static int _static_uid;


#ifdef _SIMULATOR
	std::vector<Pedestrian*> _peds; // pedestrians container
#endif

protected:
	std::vector<Wall> _walls;
	std::vector<Point> _poly; // Polygonal representation of the subroom

public:

	// constructors
	SubRoom();
	SubRoom(const SubRoom& orig);
	virtual ~SubRoom();

	/**
	 * Set/Get the subroom id
	 */
	void SetSubRoomID(int ID);

	/**
	 * Set/Get the associated room id
	 */
	void SetRoomID(int ID);
	//void SetAllWalls(const std::vector<Wall>& walls);
	//void SetWall(const Wall& wall, int index);
	//void SetPolygon(const std::vector<Point>& poly);
	//void SetArea(double a);

	void SetClosed(double c);

	/**
	 * Set the plane equation for this subroom.
	 * defined by: Z = Ax + By + C
	 */
	void SetPlanEquation(double A, double B, double C);

	/**
	 * Set/Get the subroom id
	 */
	int GetSubRoomID() const;

	/**
	 * @return the number of walls forming this subroom
	 */
	int GetNumberOfWalls() const;

	/**
	 * @return all walls
	 */
	const std::vector<Wall>& GetAllWalls() const;

	/**
	 * @return a reference to the wall at position index
	 */
	const Wall& GetWall(int index) const;

	/**
	 * @return the polygonal representation of the subroom
	 *  counterclockwise
	 */
	const std::vector<Point>& GetPolygon() const;

	/**
	 * @return a reference to all obstacles contained
	 */
	const std::vector<Obstacle*>& GetAllObstacles() const;

	/**
	 * @return the number of hlines+transitions+crossings
	 */
	int GetNumberOfGoalIDs() const;

	/**
	 * @return a vector containing all Ids
	 */
	const std::vector<int>& GetAllGoalIDs() const;

	/**
	 * @return the room containing this subroom
	 */
	int GetRoomID() const;

	/**
	 * @return the unique identifier for this subroom
	 */
	int GetUID() const;

	/**
	 * Set/Get the type of the subroom.
	 * Possible types are: stairs, room and floor.
	 * @return the type of the subroom.
	 */
	const std::string& GetType() const;

	/**
	 * Set/Get the type of the subroom.
	 * Possible types are: stairs, room and floor.
	 * @return the type of the subroom.
	 */
	void SetType(const std::string& type);


	/**
	 * @return the status
	 */
	double GetClosed() const;

	/**
	 * @return the area
	 */
	double GetArea() const;

	/**
	 * @return the centroid of the subroom
	 * @see http://en.wikipedia.org/wiki/Centroid
	 */
	Point GetCentroid() const;

	/**
	 * @return the three coefficients of the plane equation.
	 * defined by: Z = Ax + By + C
	 */
	const double * GetPlanEquation () const;

	/**
	 * @return the elevation of a 2Dimensional point using the plane equation.
	 * @see GetPlanEquation
	 */
	double GetElevation(const Point & p1) const;


	/**
	 * compute the cosine of the dihedral angle with the Horizontal plane Z=h
	 * @return the cosine of the angle
	 */
	double GetCosAngleWithHorizontal() const;

	/**
	 * Compute the area of the subroom.
	 * @see GetArea()
	 */
	void CalculateArea();

	/**
	 * @return true if the polygon is convex
	 * @see http://stackoverflow.com/questions/471962/how-do-determine-if-a-polygon-is-complex-convex-nonconvex
	 */
	bool IsConvex();

	/**
	 * @return true if the polygon is clockwise oriented
	 * @see http://stackoverflow.com/questions/9473570/polygon-vertices-clockwise-or-counterclockwise/
	 */
	bool IsClockwise();


	/**
	 * check the subroom for some inconsistencies.
	 * e.g. simple polygons
	 * no intersection between the walls and the obstacles.
	 */
	void CheckObstacles();

 	/**
	 * Check the subroom for possible errors and
	 * output user specific informations.
	 */
	void SanityCheck();

	//navigation
	void AddCrossing(Crossing* line);
	void AddTransition(Transition* line);
	void AddHline(Hline* line);

	const std::vector<Crossing*>& GetAllCrossings() const;
	const std::vector<Transition*>& GetAllTransitions() const;
	const std::vector<Hline*>& GetAllHlines() const;
	const Crossing* GetCrossing(int i) const;
	const Transition* GetTransition(int i) const;
	const Hline* GetHline(int i) const;


	/**
	 * Add a wall to the subroom
	 */
	void AddWall(const Wall& w);

	/**
	 * Adds an obstacle to the subroom.
	 * They are used for the triangulation/convexifivation process
	 */
	void AddObstacle(Obstacle* obs);

	/**
	 * Remove the pedestrian from the subroom.
	 * @param index, the index of the peds in the vector (NOT THE ID !)
	 */
	void DeletePedestrian(int index);
	//void DeletePedestrian(Pedestrian* ped);
	void AddGoalID(int ID);
	void RemoveGoalID(int ID);



	/**
	 * @return true if the two subrooms share a common walkable Edge (crossing or transition)
	 */
	bool IsDirectlyConnectedWith(const SubRoom* sub) const;

	/**
	 * @return true if the two segments are visible from each other.
	 * Alls walls and transitions and crossings are used in this check.
	 * The use of hlines is optional, because they are not real, can can be considered transparent
	 */
	bool IsVisible(Line* l1, Line* l2, bool considerHlines=false);

	/**
	 * @return true if the two points are visible from each other.
	 * Alls walls and transitions and crossings are used in this check.
	 * The use of hlines is optional, because they are not real, can be considered transparent
	 */
	bool IsVisible(const Point& p1, const Point& p2, bool considerHlines=false);



	// virtual functions
	virtual std::string WriteSubRoom() const = 0;
	virtual void WriteToErrorLog() const = 0;
	virtual std::string WritePolyLine() const=0;

	/// convert all walls and transitions(doors) into a polygon representing the subroom
    virtual bool ConvertLineToPoly(std::vector<Line*> goals) = 0;

	///check whether the pedestrians is still in the subroom
	virtual bool IsInSubRoom(const Point& ped) const = 0;


	// MPI:
	void ClearAllPedestrians();

#ifdef _SIMULATOR

	/**
	 * @return the number of pedestrians in this subroom
	 */
	int GetNumberOfPedestrians() const;
	void AddPedestrian(Pedestrian* ped);
	virtual bool IsInSubRoom(Pedestrian* ped) const;
	void SetAllPedestrians(const std::vector<Pedestrian*>& peds);
	void SetPedestrian(Pedestrian* ped, int index);
	const std::vector<Pedestrian*>& GetAllPedestrians() const;
	Pedestrian* GetPedestrian(int index) const;

#endif

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

	std::string WriteSubRoom() const;
	std::string WritePolyLine() const;

    virtual void WriteToErrorLog() const;
    virtual bool ConvertLineToPoly(std::vector<Line*> goals);
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

	std::string WriteSubRoom() const;
	std::string WritePolyLine() const;
	virtual void WriteToErrorLog() const;
    virtual bool ConvertLineToPoly(std::vector<Line*> goals);
	bool IsInSubRoom(const Point& ped) const;
};

#endif	/* _SUBROOM_H */

