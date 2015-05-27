#ifndef LANDMARK_H
#define LANDMARK_H
#include "waypoints.h"

class Landmark : public Waypoint
{
public:
    Landmark(Point pos, JEllipse ellipse, ptrRoom room);
    ~Landmark();
};

#endif // LANDMARK_H
