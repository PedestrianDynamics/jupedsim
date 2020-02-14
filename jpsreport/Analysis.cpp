/**
 * \file        Analysis.cpp
 * \date        Oct 10, 2014
 * \version     v0.7
 * \copyright   <2009-2016> Forschungszentrum Juelich GmbH. All rights reserved.
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
 * The Analysis class represents a process of analyzing groups of pedestrian
 * trajectories from experiment or simulation. Different measurement methods
 * can be used and are defined by various parameters and functions.
 *
 *
 **/

#include "Analysis.h"

#include "geometry/Room.h"
#include "geometry/SubRoom.h"
#include "methods/Method_A.h"
#include "methods/Method_B.h"
#include "methods/Method_C.h"
#include "methods/Method_D.h"
#include "methods/Method_I.h"
#include "methods/Method_J.h"
#include "methods/PedData.h"
#include "methods/VoronoiDiagram.h"

#include <algorithm> // std::min_element, std::max_element
#include <cfloat>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <vector>

#ifdef __linux__
#include <dirent.h>
#include <sys/stat.h>
#elif __APPLE__
#include <dirent.h>
#include <sys/stat.h>
#else
#include <direct.h>
#endif


using boost::geometry::dsv;
using namespace std;
OutputHandler * Log = new STDIOHandler();

/************************************************
 // Konstruktoren
 ************************************************/

Analysis::Analysis()
{
    _building       = NULL;
    _projectRootDir = "";
    _deltaF =
        5; // half of the time interval that used to calculate instantaneous velocity of ped i. Here v_i = (X(t+deltaF) - X(t+deltaF))/(2*deltaF).   X is location.
    _DoesUseMethodA = false; // Method A (Zhang2011a)
    _DoesUseMethodB = false; // Method B (Zhang2011a)
    _DoesUseMethodC = false; // Method C //calculate and save results of classic in separate file
    _DoesUseMethodD = false; // Method D--Voronoi method
    _DoesUseMethodI = false;
    _DoesUseMethodJ = false;
    _cutByCircle    = false; //Adjust whether cut each original voronoi cell by a circle
    _getProfile     = false; // Whether make field analysis or not
    _calcIndividualFD =
        false; //Adjust whether analyze the individual density and velocity of each pedestrian in stationary state (ALWAYS VORONOI-BASED)
    _vComponent =
        "B"; // to mark whether x, y or x and y coordinate are used when calculating the velocity
    _IgnoreBackwardMovement = false;
    _grid_size_X            = 0.10; // the size of the grid
    _grid_size_Y            = 0.10;
    _lowVertexX             = 0;  // LOWest vertex of the geometry (x coordinate)
    _lowVertexY             = 0;  //  LOWest vertex of the geometry (y coordinate)
    _highVertexX            = 10; // Highest vertex of the geometry
    _highVertexY            = 10;
    _cutRadius              = 1.0;
    _circleEdges            = 6;
    _trajFormat             = FileFormat::FORMAT_PLAIN;
    _isOneDimensional       = false;
}

Analysis::~Analysis()
{
    delete _building;
}

// file.txt ---> file
std::string Analysis::GetBasename(const std::string & str)
{
    std::cout << "Splitting: " << str << '\n';
    unsigned found = str.find_last_of(".");
    return str.substr(0, found);
}
// c:\\windows\\winhelp.exe ---> winhelp.exe
std::string Analysis::GetFilename(const std::string & str)
{
    std::cout << "GetFilename: " << str << '\n';
    unsigned found = str.find_last_of("/\\");
    return str.substr(found + 1);
}

void Analysis::InitArgs(ArgumentParser * args)
{
    string s  = "Parameter:\n";
    _building = new Building();
    _building->LoadGeometry(args->GetGeometryFilename().string());
    // create the polygons
    _building->InitGeometry();
    // _building->AddSurroundingRoom();

    if(args->GetIsMethodA()) {
        _DoesUseMethodA                  = true;
        vector<int> Measurement_Area_IDs = args->GetAreaIDforMethodA();
        for(unsigned int i = 0; i < Measurement_Area_IDs.size(); i++) {
            _areaForMethod_A.push_back(dynamic_cast<MeasurementArea_L *>(
                args->GetMeasurementArea(Measurement_Area_IDs[i])));
        }
        _deltaT = args->GetTimeIntervalA();
    }

    if(args->GetIsMethodB()) {
        _DoesUseMethodB                  = true;
        vector<int> Measurement_Area_IDs = args->GetAreaIDforMethodB();
        for(unsigned int i = 0; i < Measurement_Area_IDs.size(); i++) {
            _areaForMethod_B.push_back(dynamic_cast<MeasurementArea_B *>(
                args->GetMeasurementArea(Measurement_Area_IDs[i])));
        }
    }

    if(args->GetIsMethodC()) {
        _DoesUseMethodC                  = true;
        vector<int> Measurement_Area_IDs = args->GetAreaIDforMethodC();
        for(unsigned int i = 0; i < Measurement_Area_IDs.size(); i++) {
            _areaForMethod_C.push_back(dynamic_cast<MeasurementArea_B *>(
                args->GetMeasurementArea(Measurement_Area_IDs[i])));
        }
    }

    if(args->GetIsMethodD()) {
        _DoesUseMethodD                  = true;
        vector<int> Measurement_Area_IDs = args->GetAreaIDforMethodD();
        for(unsigned int i = 0; i < Measurement_Area_IDs.size(); i++) {
            _areaForMethod_D.push_back(dynamic_cast<MeasurementArea_B *>(
                args->GetMeasurementArea(Measurement_Area_IDs[i])));
        }
        _StartFramesMethodD = args->GetStartFramesMethodD();
        _StopFramesMethodD  = args->GetStopFramesMethodD();
        _IndividualFDFlags  = args->GetIndividualFDFlags();
        _geoPolyMethodD     = ReadGeometry(args->GetGeometryFilename(), _areaForMethod_D);
    }
    if(args->GetIsMethodI()) {
        _DoesUseMethodI                  = true;
        vector<int> Measurement_Area_IDs = args->GetAreaIDforMethodI();
        for(unsigned int i = 0; i < Measurement_Area_IDs.size(); i++) {
            _areaForMethod_I.push_back(dynamic_cast<MeasurementArea_B *>(
                args->GetMeasurementArea(Measurement_Area_IDs[i])));
        }
        _StartFramesMethodI = args->GetStartFramesMethodI();
        _StopFramesMethodI  = args->GetStopFramesMethodI();
        _IndividualFDFlags  = args->GetIndividualFDFlags();
        _geoPolyMethodI     = ReadGeometry(args->GetGeometryFilename(), _areaForMethod_I);
    }

    if(args->GetIsMethodJ()) {
        _DoesUseMethodJ                  = true;
        vector<int> Measurement_Area_IDs = args->GetAreaIDforMethodJ();
        for(unsigned int i = 0; i < Measurement_Area_IDs.size(); i++) {
            _areaForMethod_J.push_back(dynamic_cast<MeasurementArea_B *>(
                args->GetMeasurementArea(Measurement_Area_IDs[i])));
        }
        _StartFramesMethodJ = args->GetStartFramesMethodJ();
        _StopFramesMethodJ  = args->GetStopFramesMethodJ();
        _IndividualFDFlags  = args->GetIndividualFDFlags();
        _geoPolyMethodJ     = ReadGeometry(args->GetGeometryFilename(), _areaForMethod_J);
    }

    _deltaF                 = args->GetDelatT_Vins();
    _cutByCircle            = args->GetIsCutByCircle();
    _getProfile             = args->GetIsGetProfile();
    _isOneDimensional       = args->GetIsOneDimensional();
    _vComponent             = args->GetVComponent();
    _IgnoreBackwardMovement = args->GetIgnoreBackwardMovement();
    _grid_size_X            = int(args->GetGridSizeX());
    _grid_size_Y            = int(args->GetGridSizeY());
    _geometryFileName       = args->GetGeometryFilename();
    _projectRootDir         = args->GetProjectRootDir();
    _trajFormat             = args->GetFileFormat();
    _cutRadius              = args->GetCutRadius();
    _circleEdges            = args->GetCircleEdges();
    _outputLocation         = args->GetOutputLocation();
}


std::map<int, polygon_2d> Analysis::ReadGeometry(
    const fs::path & geometryFile,
    const std::vector<MeasurementArea_B *> & areas)
{
    Log->Write("INFO:\tReadGeometry with %s", geometryFile.string().c_str());
    double geo_minX = FLT_MAX;
    double geo_minY = FLT_MAX;
    double geo_maxX = -FLT_MAX;
    double geo_maxY = -FLT_MAX;

    std::map<int, polygon_2d> geoPoly;

    //loop over all areas
    for(auto && area : areas) {
        //search for the subroom that contains that area
        for(auto && it_room : _building->GetAllRooms()) {
            for(auto && it_sub : it_room.second->GetAllSubRooms()) {
                SubRoom * subroom = it_sub.second.get();
                point_2d point(0, 0);
                boost::geometry::centroid(area->_poly, point);
                //check if the area is contained in the obstacle
                if(subroom->IsInSubRoom(Point(point.x() / M2CM, point.y() / M2CM))) {
                    for(auto && tmp_point : subroom->GetPolygon()) {
                        append(
                            geoPoly[area->_id],
                            make<point_2d>(tmp_point._x * M2CM, tmp_point._y * M2CM));
                        geo_minX =
                            (tmp_point._x * M2CM <= geo_minX) ? (tmp_point._x * M2CM) : geo_minX;
                        geo_minY =
                            (tmp_point._y * M2CM <= geo_minY) ? (tmp_point._y * M2CM) : geo_minY;
                        geo_maxX =
                            (tmp_point._x * M2CM >= geo_maxX) ? (tmp_point._x * M2CM) : geo_maxX;
                        geo_maxY =
                            (tmp_point._y * M2CM >= geo_maxY) ? (tmp_point._y * M2CM) : geo_maxY;
                    }
                    correct(geoPoly[area->_id]);
                    //cout<<"this is:\t"<<subroom->GetAllObstacles().size()<<endl;
                    //appen the holes/obstacles if any
                    int k = 1;
                    for(auto && obst : subroom->GetAllObstacles()) {
                        geoPoly[area->_id].inners().resize(k++);
                        geoPoly[area->_id].inners().back();
                        model::ring<point_2d> & inner = geoPoly[area->_id].inners().back();
                        for(auto && tmp_point : obst->GetPolygon()) {
                            append(inner, make<point_2d>(tmp_point._x * M2CM, tmp_point._y * M2CM));
                        }
                        correct(geoPoly[area->_id]);
                    }
                }
            }
        } //room

        if(geoPoly.count(area->_id) == 0) {
            Log->Write("ERROR: \t No polygon containing the measurement id [%d]", area->_id);
            geoPoly[area->_id] = area->_poly;
        }
    }

    _highVertexX = geo_maxX;
    _highVertexY = geo_maxY;
    _lowVertexX  = geo_minX;
    _lowVertexY  = geo_minY;
    return geoPoly;
}


int Analysis::RunAnalysis(const fs::path & filename, const fs::path & path)
{
    PedData data;
    if(data.ReadData(
           _projectRootDir,
           _outputLocation,
           path,
           filename,
           _trajFormat,
           _deltaF,
           _vComponent,
           _IgnoreBackwardMovement) == false) {
        Log->Write("ERROR:\tCould not parse the file %s", filename.c_str());
        return -1;
    }

    //-----------------------------check whether there is pedestrian outside the whole geometry--------------------------------------------
    std::map<int, std::vector<int>> _peds_t = data.GetPedsFrame();
    for(int frameNr = 0; frameNr < data.GetNumFrames(); frameNr++) {
        vector<int> ids         = _peds_t[frameNr];
        vector<int> IdInFrame   = data.GetIdInFrame(frameNr, ids);
        vector<double> XInFrame = data.GetXInFrame(frameNr, ids);
        vector<double> YInFrame = data.GetYInFrame(frameNr, ids);
        for(unsigned int i = 0; i < IdInFrame.size(); i++) {
            bool IsInBuilding = false;
            for(auto && it_room : _building->GetAllRooms()) {
                for(auto && it_sub : it_room.second->GetAllSubRooms()) {
                    SubRoom * subroom = it_sub.second.get();
                    if(subroom->IsInSubRoom(Point(XInFrame[i] * CMtoM, YInFrame[i] * CMtoM))) {
                        IsInBuilding = true;
                        break;
                    }
                }
                if(IsInBuilding) {
                    break;
                }
            }
            if(false == IsInBuilding) {
                Log->Write(
                    "Warning:\tAt %dth frame pedestrian at <x=%.4f, y=%.4f> is not in geometry!",
                    frameNr + data.GetMinFrame(),
                    XInFrame[i] * CMtoM,
                    YInFrame[i] * CMtoM);
            }
        }
    }
    //-----------------------------------------------------------------------------------------------------------------------------------------------------------------

    if(_DoesUseMethodA) //Method A
    {
        if(_areaForMethod_A.empty()) {
            Log->Write("ERROR: Method A selected with no measurement area!");
            exit(EXIT_FAILURE);
        }
#pragma omp parallel for
        for(int i = 0; i < int(_areaForMethod_A.size()); i++) {
            Method_A method_A;
            method_A.SetMeasurementArea(_areaForMethod_A[i]);
            method_A.SetTimeInterval(_deltaT[i]);
            bool result_A = method_A.Process(data, _scriptsLocation, _areaForMethod_A[i]->_zPos);
            if(result_A) {
                Log->Write(
                    "INFO:\tSuccess with Method A using measurement area id %d!\n",
                    _areaForMethod_A[i]->_id);
                std::cout << "INFO:\tSuccess with Method A using measurement area id "
                          << _areaForMethod_A[i]->_id << "\n";
            } else {
                Log->Write(
                    "INFO:\tFailed with Method A using measurement area id %d!\n",
                    _areaForMethod_A[i]->_id);
            }
        }
    }

    if(_DoesUseMethodB) //Method_B
    {
        if(_areaForMethod_B.empty()) {
            Log->Write("ERROR: Method B selected with no measurement area!");
            exit(EXIT_FAILURE);
        }

#pragma omp parallel for
        for(int i = 0; i < int(_areaForMethod_B.size()); i++) {
            Method_B method_B;
            method_B.SetMeasurementArea(_areaForMethod_B[i]);
            bool result_B = method_B.Process(data);
            if(result_B) {
                Log->Write(
                    "INFO:\tSuccess with Method B using measurement area id %d!\n",
                    _areaForMethod_B[i]->_id);
                std::cout << "INFO:\tSuccess with Method B using measurement area id "
                          << _areaForMethod_B[i]->_id << "\n";
            } else {
                Log->Write(
                    "INFO:\tFailed with Method B using measurement area id %d!\n",
                    _areaForMethod_B[i]->_id);
            }
        }
    }

    if(_DoesUseMethodC) //Method C
    {
        if(_areaForMethod_C.empty()) {
            Log->Write("ERROR: Method C selected with no measurement area!");
            exit(EXIT_FAILURE);
        }
#pragma omp parallel for
        for(int i = 0; i < int(_areaForMethod_C.size()); i++) {
            Method_C method_C;
            method_C.SetMeasurementArea(_areaForMethod_C[i]);
            bool result_C = method_C.Process(data, _areaForMethod_C[i]->_zPos);
            if(result_C) {
                Log->Write(
                    "INFO:\tSuccess with Method C using measurement area id %d!\n",
                    _areaForMethod_C[i]->_id);
                std::cout << "INFO:\tSuccess with Method C using measurement area id "
                          << _areaForMethod_C[i]->_id << "\n";
            } else {
                Log->Write(
                    "INFO:\tFailed with Method C using measurement area id %d!\n",
                    _areaForMethod_C[i]->_id);
            }
        }
    }

    if(_DoesUseMethodD) //method_D
    {
        if(_areaForMethod_D.empty()) {
            Log->Write("ERROR: Method D selected with no measurement area!");
            exit(EXIT_FAILURE);
        }

#pragma omp parallel for
        for(int i = 0; i < int(_areaForMethod_D.size()); i++) {
            Method_D method_D;
            method_D.SetStartFrame(_StartFramesMethodD[i]);
            method_D.SetStopFrame(_StopFramesMethodD[i]);
            method_D.SetCalculateIndividualFD(_IndividualFDFlags[i]);
            method_D.SetGeometryPolygon(_geoPolyMethodD[_areaForMethod_D[i]->_id]);
            method_D.SetGeometryFileName(_geometryFileName);
            method_D.SetGeometryBoundaries(_lowVertexX, _lowVertexY, _highVertexX, _highVertexY);
            method_D.SetGridSize(_grid_size_X, _grid_size_Y);
            method_D.SetDimensional(_isOneDimensional);
            method_D.SetCalculateProfiles(_getProfile);
            method_D.SetTrajectoriesLocation(path);
            if(_cutByCircle) {
                method_D.Setcutbycircle(_cutRadius, _circleEdges);
            }
            method_D.SetMeasurementArea(_areaForMethod_D[i]);
            bool result_D = method_D.Process(data, _scriptsLocation, _areaForMethod_D[i]->_zPos);
            if(result_D) {
                Log->Write(
                    "INFO:\tSuccess with Method D using measurement area id %d!\n",
                    _areaForMethod_D[i]->_id);
                std::cout << "INFO:\tSuccess with Method D using measurement area id "
                          << _areaForMethod_D[i]->_id << "\n";
            } else {
                Log->Write(
                    "INFO:\tFailed with Method D using measurement area id %d!\n",
                    _areaForMethod_D[i]->_id);
            }
        }
    }

    if(_DoesUseMethodI) //method_I
    {
        if(_areaForMethod_I.empty()) {
            Log->Write("ERROR: Method I selected with no measurement area!");
            exit(EXIT_FAILURE);
        }

#pragma omp parallel for
        for(int i = 0; i < int(_areaForMethod_I.size()); i++) {
            Method_I method_I;
            method_I.SetStartFrame(_StartFramesMethodI[i]);
            method_I.SetStopFrame(_StopFramesMethodI[i]);
            method_I.SetCalculateIndividualFD(_IndividualFDFlags[i]);
            method_I.SetGeometryPolygon(_geoPolyMethodI[_areaForMethod_I[i]->_id]);
            method_I.SetGeometryFileName(_geometryFileName);
            method_I.SetGeometryBoundaries(_lowVertexX, _lowVertexY, _highVertexX, _highVertexY);
            method_I.SetGridSize(_grid_size_X, _grid_size_Y);
            method_I.SetDimensional(_isOneDimensional);
            method_I.SetTrajectoriesLocation(path);
            if(_cutByCircle) {
                method_I.Setcutbycircle(_cutRadius, _circleEdges);
            }
            method_I.SetMeasurementArea(_areaForMethod_I[i]);
            bool result_I = method_I.Process(data, _scriptsLocation, _areaForMethod_I[i]->_zPos);
            if(result_I) {
                Log->Write(
                    "INFO:\tSuccess with Method I using measurement area id %d!\n",
                    _areaForMethod_I[i]->_id);
                std::cout << "INFO:\tSuccess with Method I using measurement area id "
                          << _areaForMethod_I[i]->_id << "\n";
            } else {
                Log->Write(
                    "INFO:\tFailed with Method I using measurement area id %d!\n",
                    _areaForMethod_I[i]->_id);
            }
        }
    }

    if(_DoesUseMethodJ) //Method_J
    {
        if(_areaForMethod_J.empty()) {
            Log->Write("ERROR: Method Voronoi selected with no measurement area!");
            exit(EXIT_FAILURE);
        }

#pragma omp parallel for
        for(int i = 0; i < int(_areaForMethod_J.size()); i++) {
            Method_J Method_J;
            Method_J.SetStartFrame(_StartFramesMethodJ[i]);
            Method_J.SetStopFrame(_StopFramesMethodJ[i]);
            Method_J.SetCalculateIndividualFD(_IndividualFDFlags[i]);
            Method_J.SetGeometryPolygon(_geoPolyMethodJ[_areaForMethod_J[i]->_id]);
            Method_J.SetGeometryFileName(_geometryFileName);
            Method_J.SetGeometryBoundaries(_lowVertexX, _lowVertexY, _highVertexX, _highVertexY);
            Method_J.SetGridSize(_grid_size_X, _grid_size_Y);
            Method_J.SetDimensional(_isOneDimensional);
            Method_J.SetCalculateProfiles(_getProfile);
            Method_J.SetTrajectoriesLocation(path);
            if(_cutByCircle) {
                Method_J.Setcutbycircle(_cutRadius, _circleEdges);
            }
            Method_J.SetMeasurementArea(_areaForMethod_J[i]);
            bool result_Voronoi =
                Method_J.Process(data, _scriptsLocation, _areaForMethod_J[i]->_zPos);
            if(result_Voronoi) {
                Log->Write(
                    "INFO:\tSuccess with Method J using measurement area id %d!\n",
                    _areaForMethod_J[i]->_id);
                std::cout << "INFO:\tSuccess with Method J using measurement area id "
                          << _areaForMethod_J[i]->_id << "\n";
            } else {
                Log->Write(
                    "INFO:\tFailed with Method J using measurement area id %d!\n",
                    _areaForMethod_J[i]->_id);
            }
        }
    }

    return 0;
}

FILE * Analysis::CreateFile(const string & filename)
{
    // create the directory for the file
    fs::path filepath = fs::path(filename.c_str()).parent_path();
    if(fs::is_directory(filepath) == false) {
        if(fs::create_directories(filepath) == false && fs::is_directory(filepath) == false) {
            Log->Write("ERROR:\tcannot create the directory <%s>", filepath.string().c_str());
            return NULL;
        }
        Log->Write("INFO:\tcreate the directory <%s>", filepath.string().c_str());
    }

    //create the file
    FILE * fHandle = fopen(filename.c_str(), "w");
    return fHandle;
}
