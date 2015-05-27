#ifndef ASSOCIATIONS_H
#define ASSOCIATIONS_H

#include<memory>
class Waypoint;

class Association
{
public:
    Association();
    ~Association();
private:
    std::shared_ptr<Waypoint> _waypoint1;
    std::shared_ptr<Waypoint> _waypoint2;
};

#endif // ASSOCIATIONS_H
