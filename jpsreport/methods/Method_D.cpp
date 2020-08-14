/**
 * \file        Method_D.cpp
 * \date        Oct 10, 2014
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
 * In this file functions related to method D are defined.
 *
 *
 **/

#include "Method_D.h"

#include "../general/Macros.h"
#include "ConfigData_DIJ.h"

#include <Logger.h>
#include <cmath>
#include <iostream>
#include <map>
#include <tuple>
#include <vector>

//using std::string;
//using std::vector;
//using std::ofstream;
using namespace std;


Method_D::Method_D()
{
    _fps             = 16;
    _geoMinX         = 0;
    _geoMinY         = 0;
    _geoMaxX         = 0;
    _geoMaxY         = 0;
    _fIndividualFD   = nullptr;
    _fVoronoiRhoV    = nullptr;
    _measurementArea = nullptr;
}

Method_D::~Method_D() {}

bool Method_D::Process(
    const ConfigData_DIJ & configData,
    int measurementAreaIndex,
    const PedData & pedData,
    const double & zPos_measureArea)
{
    // TODO: lots of these parameters should be initialized in the constructor
    bool return_value = true;
    _outputLocation   = pedData.GetOutputLocation();
    _pedIDsByFrameNr  = pedData.GetPedIDsByFrameNr();
    _trajName         = pedData.GetTrajName();
    _projectRootDir   = pedData.GetProjectRootDir();
    _fps              = pedData.GetFps();
    int minFrame      = pedData.GetMinFrame();

    auto _startFrame   = configData.startFrames[measurementAreaIndex];
    auto _stopFrame    = configData.stopFrames[measurementAreaIndex];
    auto _calcLocalIFD = configData.calcLocalIFD[measurementAreaIndex];

    auto _getProfile        = configData.getProfile;
    auto _isOneDimensional  = configData.isOneDimensional;
    auto _useBlindPoints    = configData.useBlindPoints;
    bool _calcGlobalIFDOnly = false;


    if(_measurementArea->_id == -1) {
        // change parameters for calculating global IFD only
        _getProfile =
            false; // TODO: should be changed in future. There should be a function for processing global data (once) which are measurement area independent
        _calcGlobalIFDOnly = true;
    }

    LOG_INFO(
        "Method D: frame rate fps: <{:.2f}>, start: <{}>, stop: <{}> (minFrame = {})",
        _fps,
        _startFrame,
        _stopFrame,
        minFrame);
    if(_startFrame != _stopFrame) {
        if(_startFrame == -1) {
            _startFrame = minFrame;
        }
        if(_stopFrame == -1) {
            _stopFrame = pedData.GetNumFrames() + minFrame;
        }
        for(std::map<int, std::vector<int>>::iterator ite = _pedIDsByFrameNr.begin();
            ite != _pedIDsByFrameNr.end();) {
            if((ite->first + minFrame) < _startFrame || (ite->first + minFrame) > _stopFrame) {
                ite = _pedIDsByFrameNr.erase(ite);
            } else {
                ++ite;
            }
        }
    }

    if(!_calcGlobalIFDOnly && !OpenFileMethodD(_isOneDimensional)) {
        return_value = false;
    }
    if(_calcLocalIFD) {
        if(!OpenFileIndividualFD(_isOneDimensional, _calcGlobalIFDOnly)) {
            return_value = false;
        }
    }
    LOG_INFO("------------------------Analyzing with Method D-----------------------------");
    for(const auto & ite : _pedIDsByFrameNr) {
        int frameNr = ite.first;
        int frid    = frameNr + minFrame;
        //padd the frameid with 0
        std::ostringstream ss;
        ss << std::setw(5) << std::setfill('0') << std::internal << frid;
        const std::string str_frid = ss.str();
        if((frid % 50) == 0) {
            LOG_INFO("frame ID = {}", frid);
        }
        vector<int> ids         = _pedIDsByFrameNr[frameNr];
        vector<int> IdInFrame   = pedData.GetIdInFrame(frameNr, ids, zPos_measureArea);
        vector<double> XInFrame = pedData.GetXInFrame(frameNr, ids, zPos_measureArea);
        vector<double> YInFrame = pedData.GetYInFrame(frameNr, ids, zPos_measureArea);
        vector<double> ZInFrame = pedData.GetZInFrame(frameNr, ids, zPos_measureArea);
        vector<double> VInFrame = pedData.GetVInFrame(frameNr, ids, zPos_measureArea);

        if(IdInFrame.size() == 0) {
            LOG_WARNING("no pedestrians in frame <{}>", frameNr);
            continue;
        }

        //------------------------------Remove peds outside geometry------------------------------------------
        for(size_t i = 0; i < static_cast<size_t>(IdInFrame.size()); i++) {
            if(!within(point_2d(round(XInFrame[i]), round(YInFrame[i])), _geoPoly)) {
                LOG_WARNING(
                    "Pedestrian with id <{}> at <x={:.4f}, y={:.4f}, z={:.4f}> is not in the "
                    "geometry and will not be"
                    "considered in the analysis!",
                    IdInFrame[i],
                    XInFrame[i] * CMtoM,
                    YInFrame[i] * CMtoM,
                    ZInFrame[i] * CMtoM);
                IdInFrame.erase(IdInFrame.begin() + i);
                XInFrame.erase(XInFrame.begin() + i);
                YInFrame.erase(YInFrame.begin() + i);
                ZInFrame.erase(ZInFrame.begin() + i);
                VInFrame.erase(VInFrame.begin() + i);
                i--;
            }
        }
        int numPeds = IdInFrame.size();
        //---------------------------------------------------------------------------------------------------------------
        if((numPeds > 3) || _useBlindPoints) {
            if(_isOneDimensional) {
                CalcVoronoiResults1D(
                    XInFrame,
                    VInFrame,
                    IdInFrame,
                    _measurementArea->_poly,
                    str_frid,
                    _calcLocalIFD);
            } else {
                // TODO: not sure what is happening here?? Positions are shifted when they are on the same line?
                if(ArePointsOnOneLine(XInFrame, YInFrame)) {
                    if(fabs(XInFrame[1] - XInFrame[0]) < DMIN) {
                        XInFrame[1] += JPS_OFFSET;
                    } else {
                        YInFrame[1] += JPS_OFFSET;
                    }
                }
                std::vector<std::pair<polygon_2d, int>> polygons_id =
                    GetPolygons(configData, XInFrame, YInFrame, VInFrame, IdInFrame);

                vector<polygon_2d> polygons;
                polygons.reserve(polygons_id.size());
                for(const auto & p : polygons_id)
                    polygons.push_back(p.first);

                if(!polygons.empty()) {
                    if(!_calcGlobalIFDOnly) {
                        OutputVoronoiResults(polygons, str_frid, VInFrame); // TODO polygons_id
                    }

                    if(_calcLocalIFD) {
                        if(!_isOneDimensional) {
                            GetIndividualFD(
                                polygons,
                                VInFrame,
                                IdInFrame,
                                _measurementArea->_poly,
                                str_frid,
                                XInFrame,
                                YInFrame,
                                ZInFrame,
                                _calcGlobalIFDOnly);
                        }
                    }
                    // TODO: profiles should be calculated for the default MA only.
                    if(_getProfile) {                                          //	field analysis
                        GetProfiles(configData, str_frid, polygons, VInFrame); // TODO polygons_id
                    }
                } else {
                    LOG_WARNING(
                        "Voronoi Diagrams are not obtained!. Frame: {} (minFrame = "
                        "{})",
                        frid,
                        minFrame);
                }
            }
        } else {
            // numPeds <= 3 and _useblindPoints is disabled
            LOG_INFO(
                "Not enough pedestrians (N={}) available to calculate Voronoi cells for frame = {} "
                "(minFrame = {}). Consider enable use of blind points.",
                numPeds,
                frid,
                minFrame);
        }
    }
    if(!_calcGlobalIFDOnly) {
        fclose(_fVoronoiRhoV);
    }
    if(_calcLocalIFD) {
        fclose(_fIndividualFD);
    }
    return return_value;
}

bool Method_D::OpenFileMethodD(bool _isOneDimensional)
{
    std::string voroLocation(VORO_LOCATION);
    fs::path tmp("_id_" + std::to_string(_measurementArea->_id) + ".dat");
    tmp = _outputLocation / voroLocation / ("rho_v_Voronoi_" + _trajName.string() + tmp.string());
    string results_V = tmp.string();

    if((_fVoronoiRhoV = Analysis::CreateFile(results_V)) == nullptr) {
        LOG_ERROR("cannot open the file to write Voronoi density and velocity.");
        return false;
    } else {
        if(_isOneDimensional) {
            fprintf(
                _fVoronoiRhoV,
                "#framerate:\t%.2f\n\n#Frame \t Voronoi density(m^(-1))\t	Voronoi "
                "velocity(m/s)\n",
                _fps);
        } else {
            fprintf(
                _fVoronoiRhoV,
                "#framerate:\t%.2f\n\n#Frame \t Voronoi density(m^(-2))\t	Voronoi "
                "velocity(m/s)\n",
                _fps);
        }
        return true;
    }
}

bool Method_D::OpenFileIndividualFD(bool _isOneDimensional, bool global)
{
    fs::path trajFileName("_id_" + std::to_string(_measurementArea->_id) + ".dat");
    fs::path indFDPath("Fundamental_Diagram");
    indFDPath = _outputLocation / indFDPath / "IndividualFD" /
                ("IFD_D_" + _trajName.string() + trajFileName.string());
    string Individualfundment = indFDPath.string();
    if((_fIndividualFD = Analysis::CreateFile(Individualfundment)) == nullptr) {
        LOG_ERROR("cannot open the file individual.");
        return false;
    } else {
        if(_isOneDimensional) {
            fprintf(
                _fIndividualFD,
                "#framerate (fps):\t%.2f\n\n#Frame\tPersID\tIndividual density(m^(-1))\tIndividual "
                "velocity(m/s)\tHeadway(m)\n",
                _fps);
        } else if(!global) {
            fprintf(
                _fIndividualFD,
                "#framerate (fps):\t%.2f\n\n#Frame\tPersID\tx/m\ty/m\tz/m\tIndividual "
                "density(m^(-2))\tIndividual velocity(m/s)\tVoronoi Polygon\tIntersection "
                "Polygon\n",
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

std::vector<std::pair<polygon_2d, int>> Method_D::GetPolygons(
    const ConfigData_DIJ & configData,
    vector<double> & XInFrame,
    vector<double> & YInFrame,
    vector<double> & VInFrame,
    vector<int> & IdInFrame)
{
    VoronoiDiagram vd;

    // get range for bounding box around the geoemtry
    double boundpoint =
        10 * max(max(fabs(_geoMinX), fabs(_geoMinY)), max(fabs(_geoMaxX), fabs(_geoMaxY)));
    std::vector<std::pair<polygon_2d, int>> polygons_id;
    polygons_id = vd.getVoronoiPolygons(XInFrame, YInFrame, VInFrame, IdInFrame, boundpoint);

    polygon_2d poly;
    if(configData.cutByCircle) {
        polygons_id = vd.cutPolygonsWithCircle(
            polygons_id, XInFrame, YInFrame, configData.cutRadius, configData.circleEdges);
    }

    polygons_id = vd.cutPolygonsWithGeometry(polygons_id, _geoPoly, XInFrame, YInFrame);

    for(auto && p : polygons_id) {
        poly = p.first;
        ReducePrecision(poly);
        // TODO update polygon_id?
    }
    return polygons_id;
}
/**
 * Output the Voronoi density and velocity in the corresponding file
 */
void Method_D::OutputVoronoiResults(
    const vector<polygon_2d> & polygons,
    const string & frid,
    const vector<double> & VInFrame)
{
    double VoronoiVelocity = 1;
    double VoronoiDensity  = -1;
    std::tie(VoronoiDensity, VoronoiVelocity) =
        GetVoronoiDensityVelocity(polygons, VInFrame, _measurementArea->_poly);
    fprintf(_fVoronoiRhoV, "%s\t%.3f\t%.3f\n", frid.c_str(), VoronoiDensity, VoronoiVelocity);
}

/**
 * calculate the voronoi density and velocity according to voronoi cell of each pedestrian and their instantaneous velocity "Velocity".
 * input: voronoi cell and velocity of each pedestrian and the measurement area
 * output: the voronoi density and velocity in the measurement area (tuple)
 */
std::tuple<double, double> Method_D::GetVoronoiDensityVelocity(
    const vector<polygon_2d> & polygon,
    const vector<double> & Velocity,
    const polygon_2d & measureArea)
{
    double meanV   = 0;
    double density = 0;
    int temp       = 0;
    for(auto && polygon_iterator : polygon) {
        polygon_list v;
        intersection(measureArea, polygon_iterator, v);
        if(!v.empty()) {
            meanV += Velocity[temp] * area(v[0]);
            density += area(v[0]) / area(polygon_iterator);
            if((area(v[0]) - area(polygon_iterator)) > J_EPS) {
                std::stringstream stringStream;
                stringStream << "----------------------Now calculating "
                                "density-velocity!!!-----------------\n ";
                stringStream << "measure area: \t" << std::setprecision(16) << dsv(measureArea)
                             << "\n";
                stringStream << "Original polygon:\t" << std::setprecision(16)
                             << dsv(polygon_iterator) << "\n";
                stringStream << "intersected polygon: \t" << std::setprecision(16) << dsv(v[0])
                             << "\n";
                stringStream << "this is a wrong result in density calculation\t " << area(v[0])
                             << '\t' << area(polygon_iterator)
                             << "  (diff=" << (area(v[0]) - area(polygon_iterator)) << ")"
                             << "\n";
                LOG_WARNING("{}", stringStream.str());
            }
        }
        temp++;
    }
    meanV   = meanV / area(measureArea);
    density = density / (area(measureArea) * CMtoM * CMtoM);
    return std::make_tuple(density, meanV);
}
// and velocity is calculated for every frame
void Method_D::GetProfiles(
    const ConfigData_DIJ & configData,
    const string & frameId,
    const vector<polygon_2d> & polygons,
    const vector<double> & velocity)
{
    float _grid_size_X = configData.gridSizeX;
    float _grid_size_Y = configData.gridSizeY;

    std::string voroLocation(VORO_LOCATION);
    fs::path tmp("field");
    fs::path vtmp("velocity");
    fs::path dtmp("density");
    tmp  = _outputLocation / voroLocation / tmp;
    vtmp = tmp / vtmp /
           ("Prf_v_" + _trajName.string() + "_id_" + std::to_string(_measurementArea->_id) + "_" +
            frameId + ".dat");
    dtmp = tmp / dtmp /
           ("Prf_d_" + _trajName.string() + "_id_" + std::to_string(_measurementArea->_id) + "_" +
            frameId + ".dat");
    string Prfvelocity = vtmp.string();
    string Prfdensity  = dtmp.string();

    FILE * Prf_velocity;
    if((Prf_velocity = Analysis::CreateFile(Prfvelocity)) == nullptr) {
        LOG_ERROR("cannot open the file {} to write the field data", Prfvelocity);
        exit(EXIT_FAILURE);
    }
    FILE * Prf_density;
    if((Prf_density = Analysis::CreateFile(Prfdensity)) == nullptr) {
        LOG_ERROR("cannot open the file to write the field density");
        exit(EXIT_FAILURE);
    }

    int NRow = (int) ceil(
        (_geoMaxY - _geoMinY) /
        _grid_size_Y); // the number of rows that the geometry will be discretized for field analysis
    int NColumn = (int) ceil(
        (_geoMaxX - _geoMinX) /
        _grid_size_X); //the number of columns that the geometry will be discretized for field analysis
    for(int row_i = 0; row_i < NRow; row_i++) { //
        for(int colum_j = 0; colum_j < NColumn; colum_j++) {
            polygon_2d measurezoneXY;
            {
                const double coor[][2] = {
                    {_geoMinX + colum_j * _grid_size_X, _geoMaxY - row_i * _grid_size_Y},
                    {_geoMinX + colum_j * _grid_size_X + _grid_size_X,
                     _geoMaxY - row_i * _grid_size_Y},
                    {_geoMinX + colum_j * _grid_size_X + _grid_size_X,
                     _geoMaxY - row_i * _grid_size_Y - _grid_size_Y},
                    {_geoMinX + colum_j * _grid_size_X,
                     _geoMaxY - row_i * _grid_size_Y - _grid_size_Y},
                    {_geoMinX + colum_j * _grid_size_X,
                     _geoMaxY - row_i * _grid_size_Y} // closing point is opening point
                };
                assign_points(measurezoneXY, coor);
            }
            correct(
                measurezoneXY); // Polygons should be closed, and directed clockwise. If you're not sure if that is the case, call this function

            double densityXY;
            double velocityXY;
            std::tie(densityXY, velocityXY) =
                GetVoronoiDensityVelocity(polygons, velocity, measurezoneXY);
            fprintf(Prf_density, "%.3f\t", densityXY);
            fprintf(Prf_velocity, "%.3f\t", velocityXY);
        }
        fprintf(Prf_density, "\n");
        fprintf(Prf_velocity, "\n");
    }
    fclose(Prf_velocity);
    fclose(Prf_density);
}

void Method_D::GetIndividualFD(
    const vector<polygon_2d> & polygon,
    const vector<double> & Velocity,
    const vector<int> & Id,
    const polygon_2d & measureArea,
    const string & frid,
    vector<double> & XInFrame,
    vector<double> & YInFrame,
    vector<double> & ZInFrame,
    bool global)
{
    double uniquedensity  = 0;
    double uniquevelocity = 0;
    double x, y, z;
    int uniqueId = 0;
    int temp     = 0;
    for(const auto & polygon_iterator : polygon) {
        polygon_list v;
        intersection(measureArea, polygon_iterator, v);
        if(!v.empty()) {
            string polygon_str = polygon_to_string(polygon_iterator);


            uniquedensity  = 1.0 / (area(polygon_iterator) * CMtoM * CMtoM);
            uniquevelocity = Velocity[temp];
            uniqueId       = Id[temp];
            x              = XInFrame[temp] * CMtoM;
            y              = YInFrame[temp] * CMtoM;
            z              = ZInFrame[temp] * CMtoM;

            if(global) {
                // no need to print inersection polygon
                fprintf(
                    _fIndividualFD,
                    "%s\t %d\t %.4f\t %.4f\t %.4f\t %.4f\t %.4f\t%s\n",
                    frid.c_str(),
                    uniqueId,
                    x,
                    y,
                    z,
                    uniquedensity,
                    uniquevelocity,
                    polygon_str.c_str());
            } else {
                // print intersection polygons as well
                string v_str = polygon_to_string(v[0]);
                fprintf(
                    _fIndividualFD,
                    "%s\t %d\t %.4f\t %.4f\t %.4f\t %.4f\t %.4f\t%s\t%s\n",
                    frid.c_str(),
                    uniqueId,
                    x,
                    y,
                    z,
                    uniquedensity,
                    uniquevelocity,
                    polygon_str.c_str(),
                    v_str.c_str());
            }
        }
        temp++;
    }
}

void Method_D::SetGeometryPolygon(polygon_2d geometryPolygon)
{
    _geoPoly = geometryPolygon;
}

void Method_D::SetGeometryBoundaries(double minX, double minY, double maxX, double maxY)
{
    _geoMinX = minX;
    _geoMinY = minY;
    _geoMaxX = maxX;
    _geoMaxY = maxY;
}

void Method_D::SetMeasurementArea(MeasurementArea_B * area)
{
    _measurementArea = area;
}

void Method_D::ReducePrecision(polygon_2d & polygon)
{
    for(auto && point : polygon.outer()) {
        point.x(round(point.x() * 100000000000.0) / 100000000000.0);
        point.y(round(point.y() * 100000000000.0) / 100000000000.0);
    }
}

void Method_D::CalcVoronoiResults1D(
    vector<double> & XInFrame,
    vector<double> & VInFrame,
    vector<int> & IdInFrame,
    const polygon_2d & measureArea,
    const string & frid,
    bool _calcIndividualFD)
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
        if(_calcIndividualFD) {
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
    }
    VoronoiDensity /= ((right_boundary - left_boundary) * CMtoM);
    VoronoiVelocity /= ((right_boundary - left_boundary) * CMtoM);
    fprintf(_fVoronoiRhoV, "%s\t%.3f\t%.3f\n", frid.c_str(), VoronoiDensity, VoronoiVelocity);
}

double Method_D::getOverlapRatio(
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

bool Method_D::ArePointsOnOneLine(vector<double> & XInFrame, vector<double> & YInFrame)
{
    double deltaX    = XInFrame[1] - XInFrame[0];
    bool isOnOneLine = true;
    if(fabs(deltaX) < DMIN) {
        for(unsigned int i = 2; i < XInFrame.size(); i++) {
            if(fabs(XInFrame[i] - XInFrame[0]) > DMIN) {
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
            if(dist > DMIN) {
                isOnOneLine = false;
                break;
            }
        }
    }
    return isOnOneLine;
}
