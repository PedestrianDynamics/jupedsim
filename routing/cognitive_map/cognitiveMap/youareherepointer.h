#ifndef YOUAREHEREPOINTER_H
#define YOUAREHEREPOINTER_H

#include "../../../geometry/Point.h"


// Ellipse representing the approximative position of a pedestrian in his cognitive map
class YouAreHerePointer
{
public:
    YouAreHerePointer();
    ~YouAreHerePointer();

    //Getter
    const Point& GetPos() const;
    const double& GetDirection() const;


    //Setter
    void SetPos(const Point& point);
    void SetDirection(const double& angle);

    //Update
    void UpdateYAH(const Point& move);

private:
    Point _pos;
    Point _oldpos;

    //Direction
    double _angle;

    //Region

};

#endif // YOUAREHEREPOINTER_H
