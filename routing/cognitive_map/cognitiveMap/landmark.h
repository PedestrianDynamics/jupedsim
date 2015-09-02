#ifndef LANDMARK_H
#define LANDMARK_H
#include "waypoints.h"

class Association;
using ptrAssociation = std::shared_ptr<Association>;
using Associations = std::vector<ptrAssociation>;

class Landmark : public Waypoint
{
public:
    Landmark(Point pos, ptrRoom room=nullptr);
    ~Landmark();
    /// Associations
    Associations GetAssociations() const;
    void AddAssociation(ptrAssociation asso);
private:
    Associations _assoContainer;
};

#endif // LANDMARK_H
