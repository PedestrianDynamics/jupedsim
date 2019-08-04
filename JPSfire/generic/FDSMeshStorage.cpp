/**
 * \file        FDSMeshStorage.h
 * \date        Jan 1, 2014
 * \version     v0.7
 * \copyright   <2009-2015> Forschungszentrum Jülich GmbH. All rights reserved.
 *
 * \section License
 * This file is part of JuPedSim.
 *
 * JuPedSim is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * JuPedSim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with JuPedSim. If not, see <http://www.gnu.org/licenses/>.
 *
 * \section Description
 * Container to store all FDSMeshs. Sorted first by coordinates of the corresponding door,
 * secondly by simulation's global time
 *
 *
 **/
#include "FDSMeshStorage.h"
#include "general/Filesystem.h"
#include "IO/OutputHandler.h"


FDSMeshStorage::FDSMeshStorage()
{

}

FDSMeshStorage::FDSMeshStorage(const std::string &filepath, const double &finalTime, const double &updateIntervall, const std::string &study, const std::string &irritant) :
    _filepath(filepath), _finalTime(finalTime),
    _updateIntervall(updateIntervall), _study(study),
    _elevationlist(), _timelist(), _irritant(irritant)
{
    ///Check if _filepath exists
        fs::path p(_filepath);
                p = fs::canonical(p).make_preferred(); //remove ..
                _filepath = p.string(); // TODO: refactor this class. Use path instead of string
        std::cout << "\nFDSMeshStorage: <" << p << ">\n";
    if (fs::exists(p) )
    {
        std::cout << "\nCreating QuantityList..." << std::endl;
        CreateQuantityList();
        std::cout << "Success!" << std::endl;
        std::cout << "\nCreating ElevationList..." << std::endl;
        CreateElevationList();
        std::cout << "Success!" << std::endl;
        std::cout << "\nCreating DoorList..." << std::endl;
        CreateDoorList();
        std::cout << "Success!" << std::endl;
        std::cout << "\nCreating TimeList..." << std::endl;
        CreateTimeList();
        std::cout << "Success!" << std::endl;
        std::cout << "\nCreating FDSMeshes..." << std::endl;
        CreateFDSMeshes();
        std::cout << "Success!" << std::endl;
    }
    else {
        Log->Write("ERROR:\tCould not find directory <%s>\n", _filepath.c_str());
        exit(EXIT_FAILURE);
    }
}


FDSMeshStorage::~FDSMeshStorage()
{

}

bool FDSMeshStorage::CreateQuantityList()
{
    /// Create quantity list
    _quantitylist.clear();
    fs::directory_iterator end ;
    for( fs::directory_iterator iter(_filepath) ; iter != end ; ++iter ) {
      if ( fs::is_directory( *iter ) )
      {
                  std::string quant_dir = iter->path().string();
          quant_dir =  quant_dir.substr( quant_dir.find_last_of("/\\") + 1 );
          //std::cout << quant_dir << std::endl;
           _quantitylist.push_back(quant_dir);
      }
    }
    if (_quantitylist.size() == 0) {
        Log->Write("ERROR:\tCould not find suitable quantities in %s", _filepath.c_str());
        exit(EXIT_FAILURE);
        return false;
    }
        std::cout << "_quantitylist.size(): " << _quantitylist.size() << "\n";
    return true;
}


bool FDSMeshStorage::CreateElevationList()
{
    /// Create elevation list out of the available Z_* dirs for each quantity
    _elevationlist.clear();
    fs::directory_iterator end ;
        fs::path q(_filepath);
        q /= fs::path(_quantitylist[0]);
    for( fs::directory_iterator iter(q) ; iter != end ; ++iter ) {
      if ( fs::is_directory( *iter ) )
      {
          std::string elev_dir = iter->path().string();
          double elev =  std::stod(elev_dir.substr( elev_dir.rfind("_") + 1 ));
          //std::cout << elev << std::endl;
          _elevationlist.push_back(elev);
      }
    }
    if (_elevationlist.size() == 0) {
        Log->Write("ERROR:\tCould not find suitable grid elevations in %s", _filepath.c_str());
        exit(EXIT_FAILURE);
        return false;
    }
        std::cout << "_elevationlist.size(): " << _elevationlist.size() << "\n";
    return true;
}


void FDSMeshStorage::CreateDoorList()
{
    /// Create door list only neceassry if smoke sensor is active
    _doorlist.clear();
    fs::directory_iterator end ;
        fs::path q(_filepath);
        q /= fs::path(_quantitylist[0]);
    for( auto &elv:_elevationlist){
        std::string elvAsString = std::to_string(elv);
        for( fs::directory_iterator iter(q) ; iter != end ; ++iter ) {
           if ( !fs::is_directory( *iter ) )
                continue;

           // skip directories not starting with Z_
           if (iter->path().string().find("Z_") == std::string::npos)
                continue;
           // Skip if different elevation
           double this_elev =  std::stod(elvAsString.substr( elvAsString.rfind("_") + 1 ));
           if(this_elev != elv) continue;
           // iterate over "Z_/" directories
           for( fs::directory_iterator i(iter->path() ) ; i != end ; ++i ) {
                if ( ! fs::is_directory( *i ) ) continue;
                std::string tempString = i->path().string();
                tempString = tempString.substr(0, tempString.find_last_of("/\\"));
                unsigned long startChar = tempString.find_last_of("/\\") + 1;
                tempString = i->path().string();
                tempString = tempString.substr(startChar);
                                std::replace(tempString.begin(), tempString.end(), '\\', '/'); // fox for windows Z_1\Door_X_2_Y_6 -> Z_1/Door_X_2_Y_6
                _doorlist.push_back(tempString);
           }
        }
    }
        std::cout << "_doorlist.size(): " << _doorlist.size() << "\n";
}
void FDSMeshStorage::CreateTimeList()
{
    /// Create time list for mesh refreshment
    _timelist.clear();
    double i=0;
    while (i<=_finalTime)
    {
        _timelist.push_back(i);
        i+=_updateIntervall;
    }

    ///Check if specified final and update times are compliant with available data
    for(auto elem : _timelist) {
                fs::path npz_file(_filepath);
        if (_doorlist.size() > 0) {     // Smoke sensor active
                        npz_file = npz_file / _quantitylist[0] / _doorlist[0] / ("t_" + std::to_string(elem) + ".npz");
        }
        else if (_doorlist.size() == 0) {   // Smoke sensor not active
                        npz_file = npz_file / _quantitylist[0] / ("Z_" + std::to_string(_elevationlist[0])) / ("t_" + std::to_string(elem) + ".npz");
        }

        if (fs::exists(npz_file) == false)
        {
            Log->Write("ERROR:\tSpecified times are not compliant with JPSfire data " + npz_file.string());
             std::cout << "\n\nCreateTimeList(): File not found: " << npz_file.string() << std::endl;
            exit(EXIT_FAILURE);
        }
        //std::cout << "LEAVING \n" ;
    }
        std::cout << "_timelist.size(): " << _timelist.size() << "\n";
}

void FDSMeshStorage::CreateFDSMeshes()
{
        std::cout << "Enter CreateFDSMeshes\n";
    _fMContainer.clear();
    if (_doorlist.size() > 0) {     // Smoke sensor active
        for (auto &h:_quantitylist)     //list of quantities
        {
                for (auto &j:_doorlist)         //list of doors
                {
                    //std::cout << "door " << j << std::endl;
                    for (auto &k:_timelist)         //list of times
                    {
                        fs::path npz_file(_filepath);
                        npz_file = npz_file / h / j / ("t_" + std::to_string(k) + ".npz");
                        npz_file = fs::canonical(npz_file).make_preferred(); // correct sep and remove ..
                        FDSMesh mesh(npz_file.string());
                        _fMContainer.insert(std::make_pair(npz_file.string(), mesh));
                    }
                }
       }
   }
   else if (_doorlist.size() == 0) {     // Smoke sensor not active
        for (auto &h:_quantitylist)     //list of quantities
        {
            for (auto &i:_elevationlist)    //list of elevations
            {
                //std::cout << "i " << i << std::endl;
                for (auto &k:_timelist)         //list of times
                {
                     fs::path npz_file(_filepath);
                     npz_file = npz_file / h / ("Z_" + std::to_string(i)) / ("t_" + std::to_string(k) + ".npz");
                     npz_file = fs::canonical(npz_file).make_preferred(); // correct sep and remove ..
                     FDSMesh mesh(npz_file.string());
                    _fMContainer.insert(std::make_pair(npz_file.string(), mesh));
                }
            }
       }
   }
   std::cout << "_fdcontainer.size(): " << _fMContainer.size() << "\n";

}

const FDSMesh &FDSMeshStorage::GetFDSMesh(const double &simTime, const double &pedElev, const std::string &quantity)
{
    //Smoke Sensor NOT active
    int simT=simTime/_updateIntervall;
    simT*=_updateIntervall;
    _PedEyeHeight = pedElev + 1.8;
    GetNearestHeight(_PedEyeHeight);

    if (simT>=_finalTime)
        simT=_finalTime;

    //std::cout << "\t" << quantity << std::endl;
        fs::path Ztime(quantity);
        Ztime = Ztime / ("Z_" + std::to_string(_NearestHeight)) / ("t_" + std::to_string(simT) + ".000000.npz");
        Ztime = _filepath / Ztime;
        Ztime = fs::canonical(Ztime).make_preferred();
    if (_fMContainer.count(Ztime.string()) == 0) {
        //std::cout << str << std::endl;
        std::cout << "\n time ERROR: requested grid not available: " << Ztime.string() << std::endl;
        std::exit(EXIT_FAILURE);
    }
    return _fMContainer.at(Ztime.string());

//    TODO
//    if(_fMContainer.??(str) ) {
//        return _fMContainer.at(str);
//    }
//    else {
//        Log->Write("ERROR:\tCould find no appropriate FDS mesh: ", quantity.string(), pedElev, simT);
//        exit(EXIT_FAILURE);
    //    }
}

const FDSMesh &FDSMeshStorage::GetFDSMesh(const double &pedElev, const Point &doorCentre, const double &simTime)
{
    //Smoke Sensor active

     std::string quantity = "EXTINCTION_COEFFICIENT";

    _PedEyeHeight = pedElev + 1.8;
    GetNearestHeight(_PedEyeHeight);

    int simT=simTime/_updateIntervall;
    simT*=_updateIntervall;

    if (simT>=_finalTime)
        simT=_finalTime;
    // @todo: what if the files have the format Z_%.2f ?
        fs::path door_xy(quantity);
        door_xy = door_xy / ("Z_" + std::to_string(_NearestHeight)) /
                ("Door_X_" + std::to_string(doorCentre._x) + "_Y_" + std::to_string(doorCentre._y)) /
                ("t_" + std::to_string(simT) + ".000000.npz");

        door_xy = _filepath / door_xy;
        door_xy = fs::canonical(door_xy).make_preferred();
    if (_fMContainer.count(door_xy.string()) == 0) {
        std::cout << "\n > ERROR: requested sfgrid not available: " << door_xy.string() << std::endl;
        std::exit(EXIT_FAILURE);
    }

    // if (_fMContainer.count(str) == 1) {
    //      std::cout << "INFO: requested sfgrid: " << str << std::endl;
    // }
    return _fMContainer.at(door_xy.string());
}

double FDSMeshStorage::GetNearestHeight(double PedEyeHeight)
{
    ///find the nearest height in the JPSfire data related to the ped elevation
     double min_val = FLT_MAX;// std::numeric_limits<double>::max();
     int index = 0;

    for(unsigned int i=0;i < _elevationlist.size() ;++i) {
        double diff = std::abs(_elevationlist[i] - PedEyeHeight);
        if(diff < min_val) {
            min_val = diff;
            index = i;
        }
    }
    _NearestHeight = _elevationlist[index];
    //std::cout << "NEAREST: " << _NearestHeight << std::endl;
    return _NearestHeight;
}

std::string FDSMeshStorage::GetStudy() const
{
    return _study;
}

std::string FDSMeshStorage::IrritantOrNot() const
{
    return _irritant;
}
