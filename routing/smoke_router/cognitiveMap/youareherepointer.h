#pragma once

#include "geometry/Point.h"

class Pedestrian;

// Ellipse representing the approximative position of a pedestrian in his cognitive map
class YouAreHerePointer
{
public:
    YouAreHerePointer();
    ~YouAreHerePointer();

    //Getter
    const Point& GetPos() const;
    const double& GetDirection() const;
    const Pedestrian* GetPed() const;

    //Setter
    void SetPos(const Point& point);
    void SetDirection();
    void SetPed(const Pedestrian* ped);

    //Update
    void UpdateYAH(const Point& move);

    //Difference pos real-cogmap
    const Point GetPosDiff();

private:
    Point _pos;
    Point _oldpos;

    const Pedestrian* _ped;

    //Direction
    double _angle;


    //Region

};
