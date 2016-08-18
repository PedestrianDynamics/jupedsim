#ifndef PERCEPTION_H
#define PERCEPTION_H


#include <memory>

class Pedestrian;
class Building;
class VisibleEnvironment;

using ptrBuilding = std::shared_ptr<const Building>;
using ptrPed = std::shared_ptr<const Pedestrian>;
using ptrEnv = std::shared_ptr<const VisibleEnvironment>;


class Perception
{
public:
    Perception(ptrBuilding b, ptrPed ped, ptrEnv env);

private:

    ptrBuilding _building;
    ptrPed _ped;
    ptrEnv _env;

};

#endif // PERCEPTION_H
