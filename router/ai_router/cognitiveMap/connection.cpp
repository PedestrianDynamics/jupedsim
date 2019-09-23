#include "connection.h"

#include <utility>

AIConnection::AIConnection(int id, const std::string &caption, const std::string &type, int landmarkId1, int landmarkId2)
{
    _landmarkId1=landmarkId1;
    _landmarkId2=landmarkId2;
    _id=id;
    _caption=caption;
    _type=type;
}

AIConnection::~AIConnection()
{

}

std::pair<int, int> AIConnection::GetLandmarkIds() const
{
    return std::make_pair(_landmarkId1, _landmarkId2);

}

int AIConnection::GetId() const
{
    return _id;
}

const std::string &AIConnection::GetCaption() const
{
    return _caption;
}

const std::string &AIConnection::GetType() const
{
    return _type;
}

void AIConnection::SetId(int id)
{
    _id=id;
}

void AIConnection::SetCaption(const std::string &caption)
{
    _caption=caption;
}

void AIConnection::SetType(const std::string &type)
{
    _type=type;
}
