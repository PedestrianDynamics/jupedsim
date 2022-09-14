/// Copyright © 2012-2022 Forschungszentrum Jülich GmbH
/// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "Point.hpp"

#include <string>
#include <vector>

// forward declarations
class OutputHandler;
class Wall;

class Line
{
private:
    Point _point1;
    Point _point2;
    Point _centre;
    double _length;
    // unique identifier for all line elements
    static int _static_UID;
    int _uid;

public:
    Line();

    Line(
        const Point& p1,
        const Point& p2,
        int count); // count = 0 --> don't count the line. Useful for temporary lines.

    Line(const Point& p1, const Point& p2);

    virtual ~Line() = default;

    /**
     * All Line elements (also derived class) have a unique ID
     * @return the unique ID of this line element.
     */
    int GetUniqueID() const;

    /**
     * Set/Get the first end point of the line
     */
    void SetPoint1(const Point& p);

    /**
     * Set/Get the second end point of the line
     */
    void SetPoint2(const Point& p);

    /**
     * Set/Get the first end point of the line
     */
    const Point& GetPoint1() const;

    /**
     * Set/Get the second end point of the line
     */
    const Point& GetPoint2() const;

    /**
     * Return the center of the line
     */
    const Point& GetCentre() const;

    /**
     * Return the length of the line
     */
    double GetLength() const;

    /**
     * @return a normal vector to this line
     */
    Point NormalVec() const; // Normalen_Vector zu Line

    /**
     *TODO: FIXME
     */
    double NormalComp(const Point& v) const; // Normale Komponente von v auf l

    /**
     * Note that that result must not lie on the segment
     * @return the orthogonal projection of p on the line defined by the segment points.
     */
    Point LotPoint(const Point& p) const;

    /**
     * @return the point on the segment with the minimum distance to p
     */
    Point ShortestPoint(const Point& p) const;

    /**
     * @return true if the point p lies on the line defined by the first and the second point
     */
    bool IsInLine(const Point& p) const;

    /**
     * @see IsInLine
     * @return true if the point p is within the line segment defined the line end points
     */
    bool IsInLineSegment(const Point& p) const;

    bool NearlyInLineSegment(const Point& p) const;
    bool NearlyHasEndPoint(const Point& point) const;

    /**
     * @return the distance from the line to the point p
     */
    double DistTo(const Point& p) const;

    /**
     * @return the distance square from the line to the point p
     */
    double DistToSquare(const Point& p) const;

    /**
     * @return the length (Norm) of the line
     */
    double Length() const;

    /**
     * @return the lenght square of  the segment
     */
    double LengthSquare() const;

    /**
     *
     * @return true if both lines overlapp
     */
    bool Overlapp(const Line& l) const;

    /**
     * return true if point the orthogonal projection of p on Line segment is on the
     * line segment.
     */
    bool isBetween(const Point& p) const;

    /**
     * @return true if both segments are equal. The end points must be in the range of J_EPS.
     * @see Macro.h
     */
    bool operator==(const Line& l) const;

    /**
     * @return true if both segments are not equal. The end points must be in the range of J_EPS.
     * @see Macro.h
     */
    bool operator!=(const Line& l) const;

    /**
     * @return true if this < l segments are not equal. The end points must be in the range of
     * J_EPS.
     * @see Macro.h
     */
    bool operator<(const Line& l) const;

    /**
     * @see http://alienryderflex.com/intersect/
     * @see
     * http://social.msdn.microsoft.com/Forums/en-US/csharpgeneral/thread/e5993847-c7a9-46ec-8edc-bfb86bd689e3/
     * @return 0 if no intersection
     * @return 1 if both segments intersect at one point
     * @return 2 if Lines overlap
     */
    int IntersectionWith(const Line& l) const; // check two segments for intersections

    /**
     * @see http://alienryderflex.com/intersect/
     * @see
     * http://social.msdn.microsoft.com/Forums/en-US/csharpgeneral/thread/e5993847-c7a9-46ec-8edc-bfb86bd689e3/
     * @return 0 if no intersection
     * @return 1 if both segments intersect at one point
     * @return 2 if Lines overlap
     */
    int IntersectionWith(const Point& p1, const Point& p2) const;

    /*
     * @return 0 if no intersection
     * @return 1 if both segments intersect at one point
     * @return 2 if Lines overlap
     * Special case:  If the Lines are superposed/ Parallel/ no intersection,
     * then the Point of intersection is stored as NaN.
     */
    int IntersectionWith(const Point& p1, const Point& p2, Point& p3) const;

    /*
     * @return 0 if no intersection
     * @return 1 if both segments intersect at one point
     * @return 2 if Lines overlap
     * Special case:  If the Lines are superposed/ Parallel/ no intersection,
     * then the Point of intersection is stored as NaN.
     */
    int IntersectionWith(const Line& L, Point& p3) const;

    /**
     * @return the distance squared between the first point and the intersection
     * point with line l. This is exactly the same function
     * as @see IntersectionWith() but returns a double insteed.
     */
    double GetDistanceToIntersectionPoint(const Line& l) const;

    /**
     * @return true if the segment intersects with the circle of radius r
     */
    bool
    IntersectionWithCircle(const Point& centre, double radius = 0.30 /*m for pedestrians*/) const;

    /**
     * Returns the intersection points of the line segment with a circle centered at \p centre with
     * \p radius.
     * @param centre centre of the circle
     * @param radius radius of the circle
     * @return intersection points of line segment with circle
     */
    std::vector<Point> IntersectionPointsWithCircle(const Point& centre, double radius) const;

    /**
     * @return true if both segments share at least one common point
     */
    bool ShareCommonPointWith(const Line& line) const;

    /**
     * @return true if both segments share at least one common point.
     * store the common point in P.
     */
    bool ShareCommonPointWith(const Line& line, Point& P) const;

    /**
     * @return true if the given point is one end point of the segment
     */
    bool HasEndPoint(const Point& point) const;

    /**
     * Determine on which side the point is located on of the line directed from (_point1 to
     * _point2).
     * @return 0 (Left) or 1 (Right) depending on which side of the line the point is located.
     * The return value is undefined if the points are colinear.
     */
    int WichSide(const Point& pt);

    /**
     * @return true if the point is located in the left hand side of the line directed from (_point1
     * to _point2).
     */
    bool IsLeft(const Point& pt);

    /**
     * @return true for horizontal lines
     */
    bool IsHorizontal();

    /**
     * @return true for vertical lines
     */
    bool IsVertical();

    /**
     * @return left point wrt. the point pt
     */
    const Point& GetLeft(const Point& pt);

    /**
     * @return left point wrt. the point pt
     */
    const Point& GetRight(const Point& pt);

    /**
     * @return a nice formated string describing the line
     */
    virtual std::string Write() const;

    /**
     * @return a nice formated string describing the line
     */
    virtual std::string toString() const;

    /**
     * @return the angle between two lines
     */
    double GetDeviationAngle(const Line& l) const;

    double GetAngle(const Line& l) const;

    /**
     * Extend the line by d on both extremities. The line becomes larger by 2*d
     * @param d:
     * @return
     */
    Line Enlarge(double d) const;

private:
    // checks whether [a, b] intersects [x y]
    // derrived from [A,B ) [X, Y) test
    // see: http://world.std.com/~swmcd/steven/tech/interval.html
    inline bool IntervalsIntersect(double a, double b, double x, double y) const
    {
        return x <= b && a <= y;
    }
};
