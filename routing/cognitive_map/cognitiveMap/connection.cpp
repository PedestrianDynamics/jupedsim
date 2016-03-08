#include "connection.h"
#include <utility>

Connection::Connection(ptrLandmark landmark1, ptrLandmark landmark2)
{
    _landmark1=landmark1;
    _landmark2=landmark2;
}

Connection::~Connection()
{

}

std::pair<ptrLandmark, ptrLandmark> Connection::GetLandmarks() const
{
    return std::make_pair(_landmark1, _landmark2);

}

