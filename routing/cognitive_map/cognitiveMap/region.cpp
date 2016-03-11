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

bool Region::ContainsLandmark(const ptrLandmark &landmark) const
{
    for (ptrLandmark clandmark:_landmarks)
    {
        if (clandmark==landmark)
            return true;
    }
    return false;
}

void Region::InitLandmarkNetwork()
{
    _landmarkNetwork = LandmarkNetwork(GetLandmarks(),GetAllConnections());
}

double Region::PathLengthFromLandmarkToTarget(const ptrLandmark &landmark, const ptrLandmark &target)
{
    return _landmarkNetwork.LengthofShortestPathToTarget(landmark,target);
}


std::vector<ptrConnection> Region::GetAllConnections() const
{
    std::vector<ptrConnection> con{ std::begin(_connections), std::end(_connections) };
    return con;
}

void Region::AddConnection(const ptrConnection& connection)
{
    _connections.push_back(connection);
}

void Region::AddConnection(const int& id, const std::string& caption, const std::string& type, const ptrLandmark& landmark1,const ptrLandmark& landmark2)
{
    _connections.push_back(std::make_shared<Connection>(id,caption,type,landmark1,landmark2));
}

void Region::RemoveConnections(const ptrLandmark &landmark)
{
    for (ptrConnection connection:_connections)
    {
        if (connection->GetLandmarks().first==landmark || connection->GetLandmarks().second==landmark)
        {
            _connections.remove(connection);
        }
    }
}

Landmarks Region::ConnectedWith(const ptrLandmark &landmark) const
{
    Landmarks cLandmarks;
    for (ptrConnection connection:_connections)
    {
        if (connection->GetLandmarks().first==landmark )
        {
            cLandmarks.push_back(connection->GetLandmarks().second);
        }
        else if (connection->GetLandmarks().second==landmark )
        {
            cLandmarks.push_back(connection->GetLandmarks().first);
        }
    }


    return cLandmarks;

}
