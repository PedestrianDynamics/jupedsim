#ifndef VISIBLEENVIRONMENT_H
#define VISIBLEENVIRONMENT_H

class Building;
class Pedestrian;


class VisibleEnvironment
{
public:
    VisibleEnvironment();
    VisibleEnvironment(const Building* b, const Pedestrian* ped=nullptr);

private:
    const Building* _b;
    const Pedestrian* _ped;
};

#endif // VISIBLEENVIRONMENT_H
