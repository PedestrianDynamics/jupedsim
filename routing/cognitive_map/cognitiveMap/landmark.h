#ifndef LANDMARK_H
#define LANDMARK_H
#include "waypoints.h"

class Landmark : public Waypoint
{
public:
    Landmark(Point pos, ptrRoom room=nullptr);
    ~Landmark();
};

#endif // LANDMARK_H
