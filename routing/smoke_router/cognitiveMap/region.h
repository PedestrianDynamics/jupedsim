#pragma once

#include "landmark.h"
#include "connection.h"
#include "landmarknetwork.h"

using ptrLandmark = std::shared_ptr<Landmark>;
using Landmarks = std::vector<ptrLandmark>;

using ptrConnection = std::shared_ptr<Connection>;
using Connections = std::list<ptrConnection>;

class Region : public Landmark
{
public:
    Region(Point pos, ptrRoom room=nullptr);
    ~Region();

    void AddLandmark(ptrLandmark landmark);
    ptrLandmark GetRegionAsLandmark();

    //Getter
    Landmarks GetLandmarks() const;
    ptrLandmark GetLandmarkByID(const int& ID) const;
    bool ContainsLandmark(const ptrLandmark& landmark) const;

    //LandmarkNetwork
    void InitLandmarkNetwork();
    double PathLengthFromLandmarkToTarget(const ptrLandmark& landmark, const ptrLandmark& target);

    //Connections
    std::vector<ptrConnection> GetAllConnections() const;
    void AddConnection(const ptrConnection &connection);
    void AddConnection(const int &id, const std::string &caption, const std::string &type, const ptrLandmark& landmark1, const ptrLandmark& landmark2);
    void RemoveConnections(const ptrLandmark& landmark);
    Landmarks ConnectedWith(const ptrLandmark& landmark) const;

private:
    Landmarks _landmarks;
    Connections _connections;
    LandmarkNetwork _landmarkNetwork;
};
