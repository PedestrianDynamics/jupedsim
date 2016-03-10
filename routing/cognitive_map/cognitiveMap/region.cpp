#include "region.h"

Region::Region(Point pos, ptrRoom room) : Landmark(pos,room)
{

}

Region::~Region()
{

}

void Region::AddLandmark(ptrLandmark landmark)
{
    _landmarks.push_back(landmark);

}

ptrLandmark Region::GetRegionAsLandmark()
{
    return static_cast<ptrLandmark>(this);
}

Landmarks Region::GetLandmarks() const
{
    return _landmarks;

}

ptrLandmark Region::GetLandmarkByID(const int &ID) const
{
    for (ptrLandmark landmark:_landmarks)
    {
        if (landmark->GetId()==ID)
            return landmark;
    }
    return nullptr;
}
