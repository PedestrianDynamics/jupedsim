#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "../../../geometry/Point.h"
#include "../cognitiveMap/waypoints.h"

/**
 * @brief The Triangle class
 * Triangle (2D) in vector-depiction
 * p1 is positionVector
 */

class Triangle
{
public:
    Triangle(Point p, const Waypoint& waypoint);
    Triangle(Point p1, Point p2, Point p3);
    ~Triangle();
    bool Contains(const Point &point) const;

    /**
     * @brief CalcBPoints: Calculates touch-points straight
     * line through p to ellipse with pos, ah and bh
     */
    std::tuple<Point,Point> CalcBPoints(const Point& p, const Point& pos, const double& ah, const double& bh);
private:
    Point _positionVector;
    Point _vectorA;
    Point _vectorB;
};

#endif // TRIANGLE_H
