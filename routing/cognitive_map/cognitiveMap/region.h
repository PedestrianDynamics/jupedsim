#ifndef REGION_H
#define REGION_H

#include "landmark.h"

using ptrLandmark = std::shared_ptr<Landmark>;
using Landmarks = std::vector<ptrLandmark>;

class Region : public Landmark
{
public:
    Region(Point pos, ptrRoom room=nullptr);
    ~Region();

    void AddLandmark(ptrLandmark landmark);

    //Getter
    Landmarks GetLandmarks() const;
    ptrLandmark GetLandmarkByID(const int& ID) const;

private:
    Landmarks _landmarks;
};

#endif // REGION_H
