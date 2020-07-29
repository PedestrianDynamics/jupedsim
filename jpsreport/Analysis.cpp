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

#include <Logger.h>
#include <algorithm> // std::min_element, std::max_element
#include <cfloat>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
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

    _vComponent =
        "B"; // to mark whether x, y or x and y coordinate are used when calculating the velocity
    _IgnoreBackwardMovement = false;
    _lowVertexX             = 0;  // LOWest vertex of the geometry (x coordinate)
    _lowVertexY             = 0;  //  LOWest vertex of the geometry (y coordinate)
    _highVertexX            = 10; // Highest vertex of the geometry
    _highVertexY            = 10;
    _trajFormat             = FileFormat::FORMAT_PLAIN;
}

Analysis::~Analysis()
{
    delete _building;
}

// file.txt ---> file
std::string Analysis::GetBasename(const std::string & str)
{
    unsigned found = str.find_last_of(".");
    return str.substr(0, found);
}
// c:\\windows\\winhelp.exe ---> winhelp.exe
std::string Analysis::GetFilename(const std::string & str)
{
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
        _DoesUseMethodD = true;
        // TODO[DH]: modernize loops
        for(unsigned int i = 0; i < args->_configDataD.areaIDs.size(); i++) {
            _areaForMethod_D.push_back(dynamic_cast<MeasurementArea_B *>(
                args->GetMeasurementArea(args->_configDataD.areaIDs[i])));
        }

        _geoPolyMethodD = ReadGeometry(args->GetGeometryFilename(), _areaForMethod_D);
    }
    if(args->GetIsMethodI()) {
        _DoesUseMethodI = true;
        // TODO[DH]: modernize loops
        for(unsigned int i = 0; i < args->_configDataI.areaIDs.size(); i++) {
            _areaForMethod_I.push_back(dynamic_cast<MeasurementArea_B *>(
                args->GetMeasurementArea(args->_configDataI.areaIDs[i])));
        }

        _geoPolyMethodI = ReadGeometry(args->GetGeometryFilename(), _areaForMethod_I);
    }

    if(args->GetIsMethodJ()) {
        _DoesUseMethodJ = true;
        // TODO[DH]: modernize loops
        for(unsigned int i = 0; i < args->_configDataJ.areaIDs.size(); i++) {
            _areaForMethod_J.push_back(dynamic_cast<MeasurementArea_B *>(
                args->GetMeasurementArea(args->_configDataJ.areaIDs[i])));
        }

        _geoPolyMethodJ = ReadGeometry(args->GetGeometryFilename(), _areaForMethod_J);
    }

    _deltaF                 = args->GetDelatT_Vins();
    _vComponent             = args->GetVComponent();
    _IgnoreBackwardMovement = args->GetIgnoreBackwardMovement();
    _geometryFileName       = args->GetGeometryFilename();
    _projectRootDir         = args->GetProjectRootDir();
    _trajFormat             = args->GetFileFormat();
    _outputLocation         = args->GetOutputLocation();

    configData_D = args->_configDataD;
    configData_I = args->_configDataI;
    configData_J = args->_configDataJ;
}


std::map<int, polygon_2d> Analysis::ReadGeometry(
    const fs::path & geometryFile,
    const std::vector<MeasurementArea_B *> & areas)
{
    LOG_INFO("ReadGeometry with {}.", geometryFile.string());
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
                    //append the holes/obstacles if any
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
            geoPoly[area->_id] = area->_poly;
            throw std::runtime_error(
                fmt::format(FMT_STRING("No polygon containing the measurement id {}."), area->_id));
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
        LOG_ERROR("Could not parse the file {}", filename);
        return EXIT_FAILURE;
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
                LOG_WARNING(
                    "Warning:\tAt %dth frame pedestrian at <x={}, y={}> is not in geometry!",
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
            LOG_ERROR("Method A selected with no measurement area!");
            exit(EXIT_FAILURE);
        }
#pragma omp parallel for
        for(int i = 0; i < int(_areaForMethod_A.size()); i++) {
            Method_A method_A;
            method_A.SetMeasurementArea(_areaForMethod_A[i]);
            method_A.SetTimeInterval(_deltaT[i]);
            bool result_A = method_A.Process(data, _scriptsLocation, _areaForMethod_A[i]->_zPos);
            if(result_A) {
                LOG_INFO(
                    "Success with Method A using measurement area id {}!\n",
                    _areaForMethod_A[i]->_id);
            } else {
                LOG_ERROR(
                    "Failed with Method A using measurement area id {}!\n",
                    _areaForMethod_A[i]->_id);
            }
        }
    }

    if(_DoesUseMethodB) //Method_B
    {
        if(_areaForMethod_B.empty()) {
            LOG_ERROR("Method B selected with no measurement area!");
            exit(EXIT_FAILURE);
        }

#pragma omp parallel for
        for(int i = 0; i < int(_areaForMethod_B.size()); i++) {
            Method_B method_B;
            method_B.SetMeasurementArea(_areaForMethod_B[i]);
            bool result_B = method_B.Process(data);
            if(result_B) {
                LOG_INFO(
                    "Success with Method B using measurement area id {}!\n",
                    _areaForMethod_B[i]->_id);
            } else {
                LOG_ERROR(
                    "Failed with Method B using measurement area id {}!\n",
                    _areaForMethod_B[i]->_id);
            }
        }
    }

    if(_DoesUseMethodC) //Method C
    {
        if(_areaForMethod_C.empty()) {
            LOG_ERROR("Method C selected with no measurement area!");
            exit(EXIT_FAILURE);
        }
#pragma omp parallel for
        for(int i = 0; i < int(_areaForMethod_C.size()); i++) {
            Method_C method_C;
            method_C.SetMeasurementArea(_areaForMethod_C[i]);
            bool result_C = method_C.Process(data, _areaForMethod_C[i]->_zPos);
            if(result_C) {
                LOG_INFO(
                    "Success with Method C using measurement area id {}!\n",
                    _areaForMethod_C[i]->_id);
            } else {
                LOG_ERROR(
                    "Failed with Method C using measurement area id {}!\n",
                    _areaForMethod_C[i]->_id);
            }
        }
    }

    if(_DoesUseMethodD) //method_D
    {
        if(_areaForMethod_D.empty()) {
            LOG_ERROR("Method D selected with no measurement area!");
            exit(EXIT_FAILURE);
        }

#pragma omp parallel for
        for(int i = 0; i < int(_areaForMethod_D.size()); i++) {
            Method_D method_D;
            method_D.SetGeometryPolygon(_geoPolyMethodD[_areaForMethod_D[i]->_id]);
            method_D.SetGeometryFileName(_geometryFileName);
            method_D.SetGeometryBoundaries(_lowVertexX, _lowVertexY, _highVertexX, _highVertexY);
            method_D.SetTrajectoriesLocation(path);
            method_D.SetMeasurementArea(_areaForMethod_D[i]);
            bool result_D = method_D.Process(
                configData_D, i, data, _scriptsLocation, _areaForMethod_D[i]->_zPos);
            if(result_D) {
                LOG_INFO(
                    "Success with Method D using measurement area id {}!\n",
                    _areaForMethod_D[i]->_id);
            } else {
                LOG_ERROR(
                    "Failed with Method D using measurement area id {}!\n",
                    _areaForMethod_D[i]->_id);
            }
        }
    }

    if(_DoesUseMethodI) //method_I
    {
        if(_areaForMethod_I.empty()) {
            LOG_ERROR("Method I selected with no measurement area!");
            exit(EXIT_FAILURE);
        }

#pragma omp parallel for
        for(int i = 0; i < int(_areaForMethod_I.size()); i++) {
            Method_I method_I;
            method_I.SetGeometryPolygon(_geoPolyMethodI[_areaForMethod_I[i]->_id]);
            method_I.SetGeometryFileName(_geometryFileName);
            method_I.SetGeometryBoundaries(_lowVertexX, _lowVertexY, _highVertexX, _highVertexY);
            method_I.SetTrajectoriesLocation(path);
            method_I.SetMeasurementArea(_areaForMethod_I[i]);
            bool result_I = method_I.Process(
                configData_I, i, data, _scriptsLocation, _areaForMethod_I[i]->_zPos);
            if(result_I) {
                LOG_INFO(
                    "Success with Method I using measurement area id {}!\n",
                    _areaForMethod_I[i]->_id);
            } else {
                LOG_ERROR(
                    "Failed with Method I using measurement area id {}!\n",
                    _areaForMethod_I[i]->_id);
            }
        }
    }

    if(_DoesUseMethodJ) //Method_J
    {
        if(_areaForMethod_J.empty()) {
            LOG_ERROR("Method Voronoi selected with no measurement area!");
            exit(EXIT_FAILURE);
        }

#pragma omp parallel for
        for(int i = 0; i < int(_areaForMethod_J.size()); i++) {
            Method_J Method_J;
            Method_J.SetGeometryPolygon(_geoPolyMethodJ[_areaForMethod_J[i]->_id]);
            Method_J.SetGeometryFileName(_geometryFileName);
            Method_J.SetGeometryBoundaries(_lowVertexX, _lowVertexY, _highVertexX, _highVertexY);
            Method_J.SetTrajectoriesLocation(path);
            Method_J.SetMeasurementArea(_areaForMethod_J[i]);
            bool result_Voronoi = Method_J.Process(
                configData_J, i, data, _scriptsLocation, _areaForMethod_J[i]->_zPos);
            if(result_Voronoi) {
                LOG_INFO(
                    "Success with Method J using measurement area id {}!\n",
                    _areaForMethod_J[i]->_id);
            } else {
                LOG_ERROR(
                    "Failed with Method J using measurement area id {}!\n",
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
            LOG_ERROR("cannot create the directory <{}>", filepath.string());
            return NULL;
        }
        LOG_INFO("create the directory <{}>", filepath.string());
    }

    //create the file
    FILE * fHandle = fopen(filename.c_str(), "w");
    return fHandle;
}
