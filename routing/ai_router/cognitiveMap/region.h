#pragma once

#include "landmark.h"
#include "connection.h"
#include "landmarknetwork.h"


using AILandmarks = std::vector<AILandmark>;
using AIConnections = std::vector<AIConnection>;

class AIRegion : public AILandmark
{
public:
    AIRegion(const Point& pos);
    ~AIRegion();

    void AddLandmark(const AILandmark& landmark);
    void AddLandmarkSubCs(const AILandmark &landmark);
    const AILandmark* GetRegionAsLandmark() const;

    //Getter
    const AILandmarks& GetLandmarks() const;
    const AILandmark* GetLandmarkByID(int ID) const;
    bool ContainsLandmark(const AILandmark* landmark) const;

    //LandmarkNetwork
    void InitLandmarkNetwork();
    std::pair<std::vector<const AILandmark*>, double> PathLengthFromLandmarkToTarget(const AILandmark* landmark, const AILandmark* target) const;

    //Connections
    const std::vector<AIConnection> &GetAllConnections() const;
    void AddConnection(const AIConnection &connection);
    void AddConnection(int id, const std::string &caption, const std::string &type, int landmarkId1, int landmarkId2);
    void RemoveConnections(const AILandmark *landmark);
    std::vector<const AILandmark *> ConnectedWith(const AILandmark *landmark) const;

private:
    AILandmarks _landmarks;
    AIConnections _connections;
    AILandmarkNetwork _landmarkNetwork;
    AIAssociations _assoContainer;
    AILandmarks _landmarksSubConcious;
};
