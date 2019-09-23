#include "region.h"

AIRegion::AIRegion(const Point &pos) : AILandmark(pos)
{

}

AIRegion::~AIRegion()
{

}

void AIRegion::AddLandmark(const AILandmark &landmark)
{
    _landmarks.push_back(landmark);

}

void AIRegion::AddLandmarkSubCs(const AILandmark &landmark)
{
   _landmarksSubConcious.push_back(landmark);
}

const AILandmark *AIRegion::GetRegionAsLandmark() const
{
    return static_cast<const AILandmark*>(this);
}

const AILandmarks &AIRegion::GetLandmarks() const
{
    return _landmarks;
}

const AILandmark *AIRegion::GetLandmarkByID(int ID) const
{
    for (const AILandmark& landmark:_landmarks)
    {
        if (landmark.GetId()==ID)
            return &landmark;
    }
    return nullptr;
}

bool AIRegion::ContainsLandmark(const AILandmark* landmark) const
{
    for (const AILandmark& clandmark:_landmarks)
    {
        if (&clandmark==landmark)
            return true;
    }
    return false;
}

void AIRegion::InitLandmarkNetwork()
{
    _landmarkNetwork = AILandmarkNetwork(this,GetLandmarks(),GetAllConnections());
}

std::pair<std::vector<const AILandmark*>,double> AIRegion::PathLengthFromLandmarkToTarget(const AILandmark* landmark, const AILandmark* target) const
{
    return _landmarkNetwork.LengthofShortestPathToTarget(landmark,target);
}


const std::vector<AIConnection>& AIRegion::GetAllConnections() const
{
    return _connections;
}

void AIRegion::AddConnection(const AIConnection& connection)
{
    _connections.push_back(connection);
}

void AIRegion::AddConnection(int id, const std::string& caption, const std::string& type, int landmarkId1, int landmarkId2)
{
    _connections.push_back(AIConnection(id,caption,type,landmarkId1,landmarkId2));
}

void AIRegion::RemoveConnections(const AILandmark* landmark)
{
    for (auto it=_connections.begin(); it!=_connections.end(); ++it)
    {
        if (it->GetLandmarkIds().first==landmark->GetId() || it->GetLandmarkIds().second==landmark->GetId())
        {
            it=_connections.erase(it);
            --it;
        }
    }
}

std::vector<const AILandmark*> AIRegion::ConnectedWith(const AILandmark *landmark) const
{
    std::vector<const AILandmark*> cLandmarks;
    for (const AIConnection& connection:_connections)
    {
        if (connection.GetLandmarkIds().first==landmark->GetId())
        {
            cLandmarks.push_back(this->GetLandmarkByID(connection.GetLandmarkIds().second));
        }
        else if (connection.GetLandmarkIds().second==landmark->GetId())
        {
            cLandmarks.push_back(this->GetLandmarkByID(connection.GetLandmarkIds().first));
        }
    }

    return cLandmarks;

}
