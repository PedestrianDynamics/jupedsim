/**
 * \file        Method_I.cpp
 * \date        Feb 07, 2019
 * \version     v0.8
 * \copyright   <2009-2019> Forschungszentrum Jülich GmbH. All rights reserved.
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
 * In this file functions related to method I are defined.
 *
 *
 **/

#include "Method_I.h"

#include <cmath>
#include <iostream>
#include <map>
#include <tuple>
#include <vector>
//using std::string;
//using std::vector;
//using std::ofstream;
using namespace std;


Method_I::Method_I()
{
    _grid_size_X           = 0.10;
    _grid_size_Y           = 0.10;
    _fps                   = 16;
    _outputVoronoiCellData = false;
    _getProfile            = false;
    _geoMinX               = 0;
    _geoMinY               = 0;
    _geoMaxX               = 0;
    _geoMaxY               = 0;
    _cutByCircle           = false;
    _cutRadius             = -1;
    _circleEdges           = -1;
    _fIndividualFD         = nullptr;
    _areaForMethod_I       = nullptr;
    _isOneDimensional      = false;
    _startFrame            = -1;
    _stopFrame             = -1;
}

Method_I::~Method_I() {}

bool Method_I::Process(
    const PedData & peddata,
    const fs::path & scriptsLocation,
    const double & zPos_measureArea)
{
    bool return_value = true;
    _scriptsLocation  = scriptsLocation;
    _outputLocation   = peddata.GetOutputLocation();
    _peds_t           = peddata.GetPedsFrame();
    _trajName         = peddata.GetTrajName();
    _projectRootDir   = peddata.GetProjectRootDir();
    _measureAreaId    = boost::lexical_cast<string>(_areaForMethod_I->_id);
    _fps              = peddata.GetFps();
    int mycounter     = 0;
    int minFrame      = peddata.GetMinFrame();
    Log->Write(
        "INFO:\tMethod I: frame rate fps: <%.2f>, start: <%d>, stop: <%d> (minFrame = %d)",
        _fps,
        _startFrame,
        _stopFrame,
        minFrame);
    if(_startFrame != _stopFrame) {
        if(_startFrame == -1) {
            _startFrame = minFrame;
        }
        if(_stopFrame == -1) {
            _stopFrame = peddata.GetNumFrames() + minFrame;
        }
        for(std::map<int, std::vector<int>>::iterator ite = _peds_t.begin();
            ite != _peds_t.end();) {
            if((ite->first + minFrame) < _startFrame || (ite->first + minFrame) > _stopFrame) {
                mycounter++;
                ite = _peds_t.erase(ite);
            } else {
                ++ite;
            }
        }
    }

    if(!OpenFileIndividualFD()) {
        return_value = false;
    }

    Log->Write("------------------------ Analyzing with Method I -----------------------------");

    for(auto ite : _peds_t) {
        int frameNr = ite.first;
        int frid    = frameNr + minFrame;
        //padd the frameid with 0
        std::ostringstream ss;
        ss << std::setw(5) << std::setfill('0') << std::internal << frid;
        const std::string str_frid = ss.str();
        if(!(frid % 50)) {
            Log->Write("INFO:\tframe ID = %d", frid);
        }
        vector<int> ids         = _peds_t[frameNr];
        vector<int> IdInFrame   = peddata.GetIdInFrame(frameNr, ids, zPos_measureArea);
        vector<double> XInFrame = peddata.GetXInFrame(frameNr, ids, zPos_measureArea);
        vector<double> YInFrame = peddata.GetYInFrame(frameNr, ids, zPos_measureArea);
        vector<double> ZInFrame = peddata.GetZInFrame(frameNr, ids, zPos_measureArea);
        vector<double> VInFrame = peddata.GetVInFrame(frameNr, ids, zPos_measureArea);
        if(XInFrame.size() == 0) {
            Log->Write("Warning:\t no pedestrians in frame <%d>", frameNr);
            continue;
        }
        //------------------------------Remove peds outside geometry------------------------------------------
        for(int i = 0; i < (int) IdInFrame.size(); i++) {
            if(false == within(point_2d(round(XInFrame[i]), round(YInFrame[i])), _geoPoly)) {
                Log->Write(
                    "Warning:\tPedestrian at <x=%.4f, y=%.4f, , z=%.4f> is not in geometry and not "
                    "considered in analysis!",
                    XInFrame[i] * CMtoM,
                    YInFrame[i] * CMtoM,
                    ZInFrame[i] * CMtoM);
                IdInFrame.erase(IdInFrame.begin() + i);
                XInFrame.erase(XInFrame.begin() + i);
                YInFrame.erase(YInFrame.begin() + i);
                ZInFrame.erase(ZInFrame.begin() + i);
                VInFrame.erase(VInFrame.begin() + i);
                Log->Write("Warning:\t Pedestrian removed");
                i--;
            }
        }

        //---------------------------------------------------------------------------------------------------------------
        if(_isOneDimensional) {
            CalcVoronoiResults1D(XInFrame, VInFrame, IdInFrame, _areaForMethod_I->_poly, str_frid);
        } else {
            if(IsPointsOnOneLine(XInFrame, YInFrame)) {
                if(fabs(XInFrame[1] - XInFrame[0]) < dmin) {
                    XInFrame[1] += offset;
                } else {
                    YInFrame[1] += offset;
                }
            }
            std::vector<std::pair<polygon_2d, int>> polygons_id =
                GetPolygons(XInFrame, YInFrame, VInFrame, IdInFrame);
            // std::cout << ">> polygons_id " << polygons_id.size() << "\n";
            vector<polygon_2d> polygons;
            for(auto p : polygons_id) {
                polygons.push_back(p.first);
            }

            if(!polygons.empty()) {
                if(!_isOneDimensional) {
                    // GetIndividualFD(polygons,VInFrame, IdInFrame,  str_frid); // TODO polygons_id
                    GetIndividualFD(
                        polygons,
                        VInFrame,
                        IdInFrame,
                        str_frid,
                        XInFrame,
                        YInFrame,
                        ZInFrame); //
                }
            } else {
                for(int i = 0; i < (int) IdInFrame.size(); i++) {
                    std::cout << XInFrame[i] * CMtoM << "   " << YInFrame[i] * CMtoM << "   "
                              << IdInFrame[i] << "\n";
                }
                Log->Write(
                    "WARNING: \tVoronoi Diagrams are not obtained!. Frame: %d (minFrame = %d)\n",
                    frid,
                    minFrame);
            }
        }
    } //peds

    fclose(_fIndividualFD);

    return return_value;
}

bool Method_I::OpenFileIndividualFD()
{
    fs::path trajFileName("_id_" + _measureAreaId + ".dat");
    fs::path indFDPath("Fundamental_Diagram");
    indFDPath = _outputLocation / indFDPath / "IndividualFD" /
                ("IFD_I_" + _trajName.string() + trajFileName.string());
    string Individualfundment = indFDPath.string();
    if((_fIndividualFD = Analysis::CreateFile(Individualfundment)) == nullptr) {
        Log->Write("ERROR:\tcannot open the file individual\n");
        return false;
    } else {
        if(_isOneDimensional) {
            fprintf(
                _fIndividualFD,
                "#framerate (fps):\t%.2f\n\n#Frame\tPersID\tIndividual density(m^(-1))\tIndividual "
                "velocity(m/s)\tHeadway(m)\n",
                _fps);
        } else {
            fprintf(
                _fIndividualFD,
                "#framerate (fps):\t%.2f\n\n#Frame\tPersID\tx/m\ty/m\tz/m\tIndividual "
                "density(m^(-2))\tIndividual velocity(m/s)\tVoronoi Polygon\n",
                _fps);
        }
        return true;
    }
}

std::vector<std::pair<polygon_2d, int>> Method_I::GetPolygons(
    vector<double> & XInFrame,
    vector<double> & YInFrame,
    vector<double> & VInFrame,
    vector<int> & IdInFrame)
{
    VoronoiDiagram vd;
    //int NrInFrm = ids.size();
    double boundpoint =
        10 * max(max(fabs(_geoMinX), fabs(_geoMinY)), max(fabs(_geoMaxX), fabs(_geoMaxY)));
    std::vector<std::pair<polygon_2d, int>> polygons_id;
    polygons_id = vd.getVoronoiPolygons(XInFrame, YInFrame, VInFrame, IdInFrame, boundpoint);
    polygon_2d poly;
    if(_cutByCircle) {
        polygons_id =
            vd.cutPolygonsWithCircle(polygons_id, XInFrame, YInFrame, _cutRadius, _circleEdges);
    }
    //todo HH
    polygons_id = vd.cutPolygonsWithGeometry(polygons_id, _geoPoly, XInFrame, YInFrame);
    // todo HH
    for(auto && p : polygons_id) {
        poly = p.first;
        ReducePrecision(poly);
        // TODO update polygon_id?
    }
    return polygons_id;
}

void Method_I::GetIndividualFD(
    const vector<polygon_2d> & polygon,
    const vector<double> & Velocity,
    const vector<int> & Id,
    const string & frid,
    vector<double> & XInFrame,
    vector<double> & YInFrame,
    vector<double> & ZInFrame)
{
    double uniquedensity  = 0;
    double uniquevelocity = 0;
    double x, y, z;
    int uniqueId = 0;
    int temp     = 0;
    for(const auto & polygon_iterator : polygon) {
        string polygon_str = polygon_to_string(polygon_iterator);
        uniquedensity      = 1.0 / (area(polygon_iterator) * CMtoM * CMtoM);
        uniquevelocity     = Velocity[temp];
        uniqueId           = Id[temp];
        x                  = XInFrame[temp] * CMtoM;
        y                  = YInFrame[temp] * CMtoM;
        z                  = ZInFrame[temp] * CMtoM;
        fprintf(
            _fIndividualFD,
            "%s\t %d\t %.4f\t %.4f\t %.4f\t %.4f\t %.4f\t %s\n",
            frid.c_str(),
            uniqueId,
            x,
            y,
            z,
            uniquedensity,
            uniquevelocity,
            polygon_str.c_str());
        temp++;
    }
}

void Method_I::SetStartFrame(int startFrame)
{
    _startFrame = startFrame;
}

void Method_I::SetStopFrame(int stopFrame)
{
    _stopFrame = stopFrame;
}

void Method_I::Setcutbycircle(double radius, int edges)
{
    _cutByCircle = true;
    _cutRadius   = radius;
    _circleEdges = edges;
}

void Method_I::SetGeometryPolygon(polygon_2d geometryPolygon)
{
    _geoPoly = geometryPolygon;
}

void Method_I::SetGeometryBoundaries(double minX, double minY, double maxX, double maxY)
{
    _geoMinX = minX;
    _geoMinY = minY;
    _geoMaxX = maxX;
    _geoMaxY = maxY;
}

void Method_I::SetGeometryFileName(const fs::path & geometryFile)
{
    _geometryFileName = geometryFile;
}

void Method_I::SetTrajectoriesLocation(const fs::path & trajectoryPath)
{
    _trajectoryPath = trajectoryPath;
}

void Method_I::SetGridSize(double x, double y)
{
    _grid_size_X = x;
    _grid_size_Y = y;
}

void Method_I::SetOutputVoronoiCellData(bool outputCellData)
{
    _outputVoronoiCellData = outputCellData;
}

void Method_I::SetMeasurementArea(MeasurementArea_B * area)
{
    _areaForMethod_I = area;
}

void Method_I::SetDimensional(bool dimension)
{
    _isOneDimensional = dimension;
}

void Method_I::ReducePrecision(polygon_2d & polygon)
{
    for(auto && point : polygon.outer()) {
        point.x(round(point.x() * 100000000000.0) / 100000000000.0);
        point.y(round(point.y() * 100000000000.0) / 100000000000.0);
    }
}

void Method_I::CalcVoronoiResults1D(
    vector<double> & XInFrame,
    vector<double> & VInFrame,
    vector<int> & IdInFrame,
    const polygon_2d & measureArea,
    const string & frid)
{
    vector<double> measurearea_x;
    for(unsigned int i = 0; i < measureArea.outer().size(); i++) {
        measurearea_x.push_back(measureArea.outer()[i].x());
    }
    double left_boundary  = *min_element(measurearea_x.begin(), measurearea_x.end());
    double right_boundary = *max_element(measurearea_x.begin(), measurearea_x.end());

    vector<double> voronoi_distance;
    vector<double> Xtemp = XInFrame;
    vector<double> dist;
    vector<double> XLeftNeighbor;
    vector<double> XLeftTemp;
    vector<double> XRightNeighbor;
    vector<double> XRightTemp;
    sort(Xtemp.begin(), Xtemp.end());
    dist.push_back(Xtemp[1] - Xtemp[0]);
    XLeftTemp.push_back(2 * Xtemp[0] - Xtemp[1]);
    XRightTemp.push_back(Xtemp[1]);
    for(unsigned int i = 1; i < Xtemp.size() - 1; i++) {
        dist.push_back((Xtemp[i + 1] - Xtemp[i - 1]) / 2.0);
        XLeftTemp.push_back(Xtemp[i - 1]);
        XRightTemp.push_back(Xtemp[i + 1]);
    }
    dist.push_back(Xtemp[Xtemp.size() - 1] - Xtemp[Xtemp.size() - 2]);
    XLeftTemp.push_back(Xtemp[Xtemp.size() - 2]);
    XRightTemp.push_back(2 * Xtemp[Xtemp.size() - 1] - Xtemp[Xtemp.size() - 2]);
    for(unsigned int i = 0; i < XInFrame.size(); i++) {
        for(unsigned int j = 0; j < Xtemp.size(); j++) {
            if(fabs(XInFrame[i] - Xtemp[j]) < 1.0e-5) {
                voronoi_distance.push_back(dist[j]);
                XLeftNeighbor.push_back(XLeftTemp[j]);
                XRightNeighbor.push_back(XRightTemp[j]);
                break;
            }
        }
    }

    double VoronoiDensity  = 0;
    double VoronoiVelocity = 0;
    for(unsigned int i = 0; i < XInFrame.size(); i++) {
        double ratio = getOverlapRatio(
            (XInFrame[i] + XLeftNeighbor[i]) / 2.0,
            (XRightNeighbor[i] + XInFrame[i]) / 2.0,
            left_boundary,
            right_boundary);
        VoronoiDensity += ratio;
        VoronoiVelocity += (VInFrame[i] * voronoi_distance[i] * ratio * CMtoM);

        double headway           = (XRightNeighbor[i] - XInFrame[i]) * CMtoM;
        double individualDensity = 2.0 / ((XRightNeighbor[i] - XLeftNeighbor[i]) * CMtoM);
        fprintf(
            _fIndividualFD,
            "%s\t%d\t%.3f\t%.3f\t%.3f\n",
            frid.c_str(),
            IdInFrame[i],
            individualDensity,
            VInFrame[i],
            headway);
    }
    VoronoiDensity /= ((right_boundary - left_boundary) * CMtoM);
    VoronoiVelocity /= ((right_boundary - left_boundary) * CMtoM);
    fprintf(_fVoronoiRhoV, "%s\t%.3f\t%.3f\n", frid.c_str(), VoronoiDensity, VoronoiVelocity);
}

double Method_I::getOverlapRatio(
    const double & left,
    const double & right,
    const double & measurearea_left,
    const double & measurearea_right)
{
    double OverlapRatio  = 0;
    double PersonalSpace = right - left;
    if(left > measurearea_left && right < measurearea_right) //case1
    {
        OverlapRatio = 1;
    } else if(right > measurearea_left && right < measurearea_right && left < measurearea_left) {
        OverlapRatio = (right - measurearea_left) / PersonalSpace;
    } else if(left < measurearea_left && right > measurearea_right) {
        OverlapRatio = (measurearea_right - measurearea_left) / PersonalSpace;
    } else if(left > measurearea_left && left < measurearea_right && right > measurearea_right) {
        OverlapRatio = (measurearea_right - left) / PersonalSpace;
    }
    return OverlapRatio;
}

bool Method_I::IsPointsOnOneLine(vector<double> & XInFrame, vector<double> & YInFrame)
{
    double deltaX    = XInFrame[1] - XInFrame[0];
    bool isOnOneLine = true;
    if(fabs(deltaX) < dmin) {
        for(unsigned int i = 2; i < XInFrame.size(); i++) {
            if(fabs(XInFrame[i] - XInFrame[0]) > dmin) {
                isOnOneLine = false;
                break;
            }
        }
    } else {
        double slope     = (YInFrame[1] - YInFrame[0]) / deltaX;
        double intercept = YInFrame[0] - slope * XInFrame[0];
        for(unsigned int i = 2; i < XInFrame.size(); i++) {
            double dist =
                fabs(slope * XInFrame[i] - YInFrame[i] + intercept) / sqrt(slope * slope + 1);
            if(dist > dmin) {
                isOnOneLine = false;
                break;
            }
        }
    }
    return isOnOneLine;
}
