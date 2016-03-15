#ifndef YOUAREHEREPOINTER_H
#define YOUAREHEREPOINTER_H

#include "../../../geometry/Point.h"
class Pedestrian;
using ptrPed = const Pedestrian*;


// Ellipse representing the approximative position of a pedestrian in his cognitive map
class YouAreHerePointer
{
public:
    YouAreHerePointer();
    ~YouAreHerePointer();

    //Getter
    const Point& GetPos() const;
    const double& GetDirection() const;
    ptrPed GetPed() const;

    //Setter
    void SetPos(const Point& point);
    void SetDirection(const double& angle);
    void SetPed(const ptrPed ped);

    //Update
    void UpdateYAH(const Point& move);

    //Difference pos real-cogmap
    const Point GetPosDiff();

private:
    Point _pos;
    Point _oldpos;

     ptrPed _ped;

    //Direction
    double _angle;


    //Region

};

#endif // YOUAREHEREPOINTER_H
