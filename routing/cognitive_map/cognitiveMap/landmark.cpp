#include "landmark.h"
#include "associations.h"


Landmark::Landmark(Point pos, ptrRoom room):Waypoint(pos,2.0,2.0,room)
{


}

Landmark::~Landmark()
{

}

Associations Landmark::GetAssociations() const
{
    return _assoContainer;
}

void Landmark::AddAssociation(ptrAssociation asso)
{
    if (std::find(_assoContainer.begin(), _assoContainer.end(), asso)!=_assoContainer.end())
        return;
    else
        _assoContainer.push_back(asso);
}

