#ifndef FIREMESHSTORAGE_H
#define FIREMESHSTORAGE_H

#include <unordered_map>
#include "FireMesh.h"
#include "../../../geometry/Building.h"


// Container to store all fireMeshs. Sorted first by coordinates of the corresponding door, secondly by simulation's global time
using fireMeshContainer = std::unordered_map<std::string,FireMesh>;

class FireMeshStorage
{
public:
    FireMeshStorage(const Building * const b, const std::string &filepath, const double &updateintervall, const double &finalTime);
    ~FireMeshStorage();
    void CreateTimeList(const double &updateIntervall, const double &finalTime, const double &startTime=0);
    void IdentifyDoors();
    void CreateFireMeshes();
    const FireMesh& get_FireMesh(const Point &doorCentre, const double &simTime) const;

private:
    const Building *_building;
    fireMeshContainer _fMContainer;
    std::string _filepath;
    double _updateIntervall;
    std::vector<double> _timelist;
    std::vector<Crossing> _doors;


};

#endif // FIREMESHSTORAGE_H
