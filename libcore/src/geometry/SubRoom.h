/**
 * \file        SubRoom.h
 * \date        Oct 8, 2010
 * \version     v0.7
 * \copyright   <2009-2015> Forschungszentrum Jülich GmbH. All rights reserved.
 *
 * \section License
 * This file is part of JuPedSim.
 *
 * JuPedSim is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * JuPedSim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with JuPedSim. If not, see <http://www.gnu.org/licenses/>.
 *
 * \section Description
 *
 *
 **/
#pragma once

#include "general/Macros.h"
#include "routing/global_shortest/DTriangulation.h"

#include <boost/geometry.hpp>
#include <boost/polygon/polygon.hpp>
#include <string>
#include <vector>

//forward declarations
class Transition;
class Hline;
class Obstacle;
class Crossing;
class Line;
class Point;
class Wall;
class WaitingArea;

#ifdef _SIMULATOR
class Pedestrian;
#endif

namespace bg = boost::geometry;
typedef bg::model::polygon<Point, false, false> polygon_type;


class SubRoom
{
private:
    /// the id set using the SetID method
    int _id;
    /// the unique id resulting from the count of all subrooms in the system
    int _uid;
    int _roomID;
    std::vector<int> _goalIDs; // all navigation lines contained in this subroom
    double _area;
    //defined by: Z = Ax + By + C, normal vector = (A, B, -1)^T
    double _planeEquation[3];
    double _cosAngleWithHorizontalPlane;
    double _tanAngleWithHorizontalPlane;
    std::string _type;
    double _minElevation;
    double _maxElevation;

    //different types of navigation lines
    std::vector<Crossing *> _crossings;
    std::vector<Transition *> _transitions;
    std::vector<Hline *> _hlines;
    std::vector<SubRoom *> _neighbors;
    DTriangulation _delauneyTriangulator;

    /// storing and incrementing the total number of subrooms
    static int _static_uid;


protected:
    std::vector<Wall> _walls;
    std::vector<Point> _poly; // Polygonal representation of the subroom

    polygon_type _boostPoly;
    std::vector<polygon_type> _boostPolyObstacles;
    std::vector<polygon_type> _boostPolyWall;

    std::vector<double> _poly_help_constatnt; //for the function IsInsidePolygon, a.brkic
    std::vector<double> _poly_help_multiple;  //for the function IsInsidePolygon, a.brkic
    std::vector<Obstacle *> _obstacles;

public:
    /**
      * Constructor
      */
    SubRoom();
    SubRoom(const SubRoom & orig);

    /**
      * Destructor
      */
    virtual ~SubRoom();

    /**
      * Set/Get the subroom id
      */
    void SetSubRoomID(int ID);

    /**
      * Set/Get the associated room id
      */
    void SetRoomID(int ID);

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
      * Checks if a Point is on the polygon boundaries of the subroom.
      *
      * The boundary includes all walls, crossings and transitions.
      * One Wall could be excluded.
      * Only the exact same wall is excluded, no parts of walls or similar.
      */
    bool IsPointOnPolygonBoundaries(const Point & point, const Wall & exclude_wall) const;

    bool HasWall(const Wall & wall) const;


    /**
      * @return all walls
      */
    const std::vector<Wall> & GetAllWalls() const;

    /**
      * @return visible walls from position of pedestrians (considering the direction of motion)
      */
    std::vector<Wall> GetVisibleWalls(const Point & position);

    /**
      * @return the polygonal representation of the subroom
      *  counterclockwise
      */
    const std::vector<Point> & GetPolygon() const;

    /**
      * @return vertices of a polygon starting with left lower corner
      */
    std::vector<Point> StartLLCorner(const std::vector<Point> & polygon);

    /**
      * @return a reference to all obstacles contained
      */
    const std::vector<Obstacle *> & GetAllObstacles() const;

    /**
      * @return a vector containing all Ids
      */
    const std::vector<int> & GetAllGoalIDs() const;

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
    const std::string & GetType() const;

    /**
      * Set/Get the type of the subroom.
      * Possible types are: stairs, room and floor.
      * @return the type of the subroom.
      */
    void SetType(const std::string & type);

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
    const double * GetPlaneEquation() const;

    /**
      * @return the elevation of a 2Dimensional point using the plane equation.
      * @see GetPlanEquation
      */
    double GetElevation(const Point & p1) const;

    /**
      * @return the smallest elevation in subroom
      * @see GetPlanEquation
      */
    double GetMinElevation() const;

    /**
      * @return the largest elevation in subroom
      * @see GetPlanEquation
      */
    double GetMaxElevation() const;

    void SetMinElevation(double m);

    void SetMaxElevation(double M);

    /**
      * compute the cosine of the dihedral angle with the Horizontal plane Z=h
      * @return the cosine of the angle
      */
    double GetCosAngleWithHorizontal() const;

    /**
      * compute the tangent of the dihedral angle with the Horizontal plane Z=h
      * @return the tangent of the angle
      */
    double GetTanAngleWithHorizontal() const;

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
    bool CheckObstacles();

    /**
      * Check the subroom for possible errors and
      * output user specific informations.
      */
    bool SanityCheck();

    /**
      * Triangulate teh subroom
      */
    bool Triangulate();

    /**
      * @return the triangles generated by the triangulation
      */
    const std::vector<p2t::Triangle *> GetTriangles();

    /**
      * @return true if all transitions are not closed.
      */
    bool IsAccessible();

    //navigation
    bool AddCrossing(Crossing * line);
    bool AddTransition(Transition * line);
    bool RemoveTransition(Transition * t);
    bool AddHline(Hline * line);
    void AddNeighbor(SubRoom * sub);

    const std::vector<Crossing *> & GetAllCrossings() const;
    const std::vector<Transition *> & GetAllTransitions() const;
    const std::vector<Hline *> & GetAllHlines() const;
    const Crossing * GetCrossing(int i) const;
    const Transition * GetTransition(int i) const;
    const Hline * GetHline(int i) const;

    /**
      * @return true if the point is part of the polygon,
      * also considering the geometry precision.
      */
    bool IsPartOfPolygon(const Point & pt);

    /**
      *
      * @return true if the Point is inside any obstacle
      */
    bool IsInObstacle(const Point & pt);

    /**
      * @return true if there is an overlapp between the walls of the subrooms and the
      * supplied set of lines.
      */
    bool Overlapp(const std::vector<Line *> & goals) const;

    /**
      * @return the adjacent subrooms
      */
    const std::vector<SubRoom *> & GetNeighbors() const;

    /**
      * remove wall w from subroom
      * @param w: wall to remove
      * @return: true if w was removed, otherwise false
      */
    bool RemoveWall(const Wall & w);
    /**
      * Add a wall to the subroom
      */
    bool AddWall(const Wall & w);

    /**
      * Adds an obstacle to the subroom.
      * They are used for the triangulation/convexifivation process
      */
    void AddObstacle(Obstacle * obs);

    /**
      * Add/remove a goal Id
      */
    void AddGoalID(int ID);

    /**
      * Add/remove a goal Id
      */
    void RemoveGoalID(int ID);

    /**
      * @return true if the two subrooms share a common walkable Edge (crossing or transition)
      */
    bool IsDirectlyConnectedWith(SubRoom * sub) const;

    /**
      * @return true if the two points are visible from each other.
      * Alls walls and transitions and crossings are used in this check.
      * The use of hlines is optional, because they are not real, can be considered transparent
      */
    bool IsVisible(const Point & p1, const Point & p2, bool considerHlines = false);

    /**
      * @return true if the two points are visible from each other.
      * Alls walls and transitions and crossings are used in this check.
      * The use of hlines is optional, because they are not real, can be considered transparent
      */
    bool IsVisible(const Line & wall, const Point & p2);

    // virtual functions
    virtual std::string WriteSubRoom() const  = 0;
    virtual std::string WritePolyLine() const = 0;

    /// convert all walls and transitions(doors) into a polygon representing the subroom
    virtual bool ConvertLineToPoly(const std::vector<Line *> & goals) = 0;
    bool CreateBoostPoly();

    ///check whether the pedestrians is still in the subroom
    virtual bool IsInSubRoom(const Point & ped) const = 0;

    std::vector<WaitingArea *> GetAllWaitingAreas();

    bool HasGoal(int id);

    /**
     * Updates the subroom. Create the poly from the given transitions, crossings, obstacles.
     * Recomputation of the are as well as the polygons describing the subroom.
     */
    void Update();

#ifdef _SIMULATOR

    virtual bool IsInSubRoom(Pedestrian * ped) const;

#endif
};

class NormalSubRoom : public SubRoom
{
private:
    ///@see IsInSubRoom
    short WhichQuad(const Point & vertex, const Point & hitPos) const;
    double Xintercept(const Point & point1, const Point & point2, double hitY) const;

public:
    NormalSubRoom();
    NormalSubRoom(const NormalSubRoom & orig);
    virtual ~NormalSubRoom();

    std::string WriteSubRoom() const;
    std::string WritePolyLine() const;

    bool ConvertLineToPoly(const std::vector<Line *> & goals);

    /**
     * Checks whether the point \p  lies inside the subroom. Crossings and Transitions belong to the
     * subroom, whereas walls are excluded.
     * @param p Point to check
     * @return \p is inside subroom
     */
    bool IsInSubRoom(const Point & p) const;
};

class Stair : public NormalSubRoom
{
private:
    Point pUp;   /// Punkt der den oberen Bereich der Treppe markiert
    Point pDown; /// Punkt der den unteren Bereich der Treppe markiert

    const Point *
    CheckCorner(const Point ** otherPoint, const Point ** aktPoint, const Point * nextPoint);

public:
    Stair();
    Stair(const Stair & orig);
    virtual ~Stair();

    // Setter-Funktionen
    void SetUp(const Point & p);
    void SetDown(const Point & p);

    // Getter-Funktionen
    const Point & GetUp() const;
    const Point & GetDown() const;

    /// pedestrians are going the stairs downwards
    bool IsUpStairs() const;
    /// pedestrians are going the stairs upwards
    bool IsDownStair() const;

    std::string WriteSubRoom() const;
    std::string WritePolyLine() const;
    virtual bool ConvertLineToPoly(const std::vector<Line *> & goals);
};

class Escalator : public Stair
{
private:
    bool isEscalator_Up;

public:
    Escalator();

    Escalator(const Escalator & orig);
    virtual ~Escalator();

    // Setter-Funktionen
    void SetEscalatorUp();
    void SetEscalatorDown();

    // Getter-Funktionen
    bool IsEscalatorUp() const;
    bool IsEscalatorDown() const;
};
