#include "FireMeshStorage.h"

FireMeshStorage::FireMeshStorage(const Building * const b, const std::string &filepath, const double &updateintervall, const double &finalTime)
{
    _building=b;
    _filepath=filepath;

    CreateTimeList(updateintervall,finalTime);
    IdentifyDoors();
    CreateFireMeshes();
}

FireMeshStorage::~FireMeshStorage()
{

}

void FireMeshStorage::CreateTimeList(const double &updateIntervall, const double &finalTime, const double &startTime)
{
    //fulfill list with times when new Mesh should be used
    double i=0;
    while (i<=finalTime)
    {
        _timelist.push_back(i);
        i+=updateIntervall;
    }
}

void FireMeshStorage::IdentifyDoors()
{
    const std::map<int,std::unique_ptr<Door>> doors = _building->GetAllRooms();
    for (auto it=doors.begin(); it!= doors.end(); ++it)
    {
        _doors.push_back(it.second);
    }

}

void FireMeshStorage::CreateFireMeshes()
{
    for (auto &door:_doors)
    {
        for (auto &i:_timelist)
        {
            std::string filename = _filepath + "/Door_X_" + std::to_string(door->GetCentre().GetX())
                      + "_Y_" + std::to_string(door->GetCentre().GetY()) + "/t_"+std::to_string(i)+".csv";
            FireMesh mesh(filename);
            _fMContainer.emplace(std::make_pair(door->GetCentre(),i),mesh);

        }


    }

}

const FireMesh &FireMeshStorage::get_FireMesh(const Point &doorCentre, const double &simTime)
{
    return _fMContainer.at(std::make_pair(doorCentre,simTime));
}

