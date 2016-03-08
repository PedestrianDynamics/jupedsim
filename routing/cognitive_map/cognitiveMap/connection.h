#ifndef CONNECTION_H
#define CONNECTION_H

#include "landmark.h"
using ptrLandmark = std::shared_ptr<Landmark>;

class Connection
{
public:
    Connection(ptrLandmark landmark1, ptrLandmark landmark2);
    ~Connection();

    std::pair<ptrLandmark,ptrLandmark> GetLandmarks() const;

private:
    ptrLandmark _landmark1;
    ptrLandmark _landmark2;
};

#endif // CONNECTION_H
