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
