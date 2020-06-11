/**
 * \file        Method_D.h
 * \date        Oct 10, 2014
 * \version     v0.7
 * \copyright   <2009-2015> Forschungszentrum Juelich GmbH. All rights reserved.
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
 *
 *
 **/

#ifndef METHOD_D_H_
#define METHOD_D_H_

#include "../Analysis.h"
#include "ConfigData_DIJ.h"
#include "PedData.h"
#include "VoronoiDiagram.h"


class Method_D
{
public:
    Method_D();
    virtual ~Method_D();
    bool Process(
        const ConfigData_DIJ & configData,
        int measurementAreaID,
        const PedData & peddata,
        const fs::path & scriptsLocation,
        const double & zPos_measureArea);
    void SetGeometryPolygon(polygon_2d geometryPolygon);
    void SetGeometryFileName(const fs::path & geometryFile);
    void SetGeometryBoundaries(double minX, double minY, double maxX, double maxY);
    void SetMeasurementArea(MeasurementArea_B * area);
    void SetTrajectoriesLocation(const fs::path & trajectoryPath);

private:
    std::map<int, std::vector<int>> _peds_t;
    std::string _measureAreaId;
    MeasurementArea_B * _areaForMethod_D;
    fs::path _trajName;
    fs::path _projectRootDir;
    fs::path _outputLocation;
    fs::path _scriptsLocation;
    polygon_2d _areaIndividualFD;
    polygon_2d _geoPoly;
    double _geoMinX; // LOWest vertex of the geometry (x coordinate)
    double _geoMinY; //  LOWest vertex of the geometry (y coordinate)
    double _geoMaxX; // Highest vertex of the geometry
    double _geoMaxY;
    FILE * _fVoronoiRhoV;
    FILE * _fIndividualFD;
    float _fps;
    bool OpenFileMethodD(bool _isOneDimensional);
    bool OpenFileIndividualFD(bool _isOneDimensional);
    fs::path _geometryFileName;
    fs::path _trajectoryPath;


    std::vector<std::pair<polygon_2d, int>> GetPolygons(
        const ConfigData_DIJ & configData,
        std::vector<double> & XInFrame,
        std::vector<double> & YInFrame,
        std::vector<double> & VInFrame,
        std::vector<int> & IdInFrame);
    void OutputVoronoiResults(
        const std::vector<polygon_2d> & polygons,
        const std::string & frid,
        const std::vector<double> & VInFrame);
    std::tuple<double, double> GetVoronoiDensityVelocity(
        const std::vector<polygon_2d> & polygon,
        const std::vector<double> & Velocity,
        const polygon_2d & measureArea);
    void GetProfiles(
        const ConfigData_DIJ & configData,
        const std::string & frameId,
        const std::vector<polygon_2d> & polygons,
        const std::vector<double> & velocity);
    void OutputVoroGraph(
        const std::string & frameId,
        std::vector<std::pair<polygon_2d, int>> & polygons,
        int numPedsInFrame,
        const std::vector<double> & VInFrame);
    void GetIndividualFD(
        const std::vector<polygon_2d> & polygon,
        const std::vector<double> & Velocity,
        const std::vector<int> & Id,
        const polygon_2d & measureArea,
        const std::string & frid,
        std::vector<double> & XInFrame,
        std::vector<double> & YInFrame,
        std::vector<double> & ZInFrame);
    /**
      * Reduce the precision of the points to two digits
      * @param polygon
      */
    void CalcVoronoiResults1D(
        std::vector<double> & XInFrame,
        std::vector<double> & VInFrame,
        std::vector<int> & IdInFrame,
        const polygon_2d & measureArea,
        const std::string & frid,
        bool _calcIndividualFD);
    void ReducePrecision(polygon_2d & polygon);
    bool IsPedInGeometry(
        int frames,
        int peds,
        double ** Xcor,
        double ** Ycor,
        int * firstFrame,
        int * lastFrame); //check whether all the pedestrians are in the geometry
    double getOverlapRatio(
        const double & left,
        const double & right,
        const double & measurearea_left,
        const double & measurearea_right);
    bool IsPointsOnOneLine(std::vector<double> & XInFrame, std::vector<double> & YInFrame);
};

#endif /* METHOD_D_H_ */
