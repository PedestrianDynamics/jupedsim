#ifndef YOUAREHEREPOINTER_H
#define YOUAREHEREPOINTER_H

#include "../../../geometry/Point.h"


/// Ellipse representing the approximative position of a pedestrian in his cognitive map
class YouAreHerePointer
{
public:
    YouAreHerePointer();
    ~YouAreHerePointer();

    ///Getter
    Point GetPos() const;


    ///Setter
    void SetPos(const Point& point);


private:
    Point _pos;
    //Direction
    //Region

};

#endif // YOUAREHEREPOINTER_H
