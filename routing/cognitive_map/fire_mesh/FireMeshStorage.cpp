#include "FireMeshStorage.h"

FireMeshStorage::FireMeshStorage(const Building * const b, const std::string &filepath, const double &updateintervall, const double &finalTime)
{
    _building=b;
    _filepath=filepath;
    _updateIntervall=updateintervall;
    _finalTime=finalTime;

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

    const std::map<int,Crossing*> doors = _building->GetAllCrossings();

    for (auto it=doors.begin(); it!= doors.end(); ++it)
    {
        _doors.push_back(*it->second);
    }

    const std::map<int,Transition*> exits = _building->GetAllTransitions();

    for (auto it=exits.begin(); it!= exits.end(); ++it)
    {
        _doors.push_back(*it->second);
    }

}

void FireMeshStorage::CreateFireMeshes()
{
    _fMContainer.clear();
    for (auto &door:_doors)
    {
        for (auto &i:_timelist)
        {
            std::string suffix = "Door_X_" + std::to_string(door.GetCentre().GetX())
                      + "_Y_" + std::to_string(door.GetCentre().GetY()) + "\\t_"+std::to_string(i)+".csv";
            FireMesh mesh(_filepath+suffix);
            std::string str = "Door_X_"+ std::to_string(door.GetCentre().GetX())
                    + "_Y_" + std::to_string(door.GetCentre().GetY()) + "_t_"+std::to_string(i);


            _fMContainer.emplace(str,mesh);

        }

    }

}

const FireMesh &FireMeshStorage::get_FireMesh(const Point &doorCentre, const double &simTime) const
{
    int simT=simTime/_updateIntervall;
    simT*=_updateIntervall;

    if (simT>=_finalTime)
        simT=_finalTime;

    std::string str = "Door_X_"+ std::to_string(doorCentre.GetX())
            + "_Y_" + std::to_string(doorCentre.GetY()) + "_t_"+std::to_string(simT)+".000000";

    return _fMContainer.at(str);
}

