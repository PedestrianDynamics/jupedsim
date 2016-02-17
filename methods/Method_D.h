/**
 * \file        Method_D.h
 * \date        Oct 10, 2014
 * \version     v0.7
 * \copyright   <2009-2015> Forschungszentrum J��lich GmbH. All rights reserved.
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
#include <vector>
#include "PedData.h"
#include "../Analysis.h"
#include "VoronoiDiagram.h"
#include <algorithm>

#ifdef __linux__
#include <sys/stat.h>
#include <dirent.h>
#elif   __APPLE__
#include <sys/stat.h>
#include <dirent.h>
#else
#include <direct.h>
#endif

//handle more than two person are in one line
#define dmin 0.1


class Method_D
{
public:
     Method_D();
     virtual ~Method_D();
     bool Process (const PedData& peddata,const std::string& scriptsLocation);
     void SetCalculateIndividualFD(bool individualFD);
     void SetAreaIndividualFD(polygon_2d areaindividualFD);
     void Setcutbycircle(double radius,int edges);
     void SetGeometryPolygon(polygon_2d geometryPolygon);
     void SetGeometryFileName(const std::string& geometryFile);
     void SetGeometryBoundaries(double minX, double minY, double maxX, double maxY);
     void SetGridSize(double x, double y);
     void SetCalculateProfiles(bool calcProfile);
     void SetOutputVoronoiCellData(bool outputCellData);
     void SetPlotVoronoiGraph(bool plotVoronoiGraph);
     void SetMeasurementArea (MeasurementArea_B* area);
     void SetDimensional (bool dimension);

private:
     std::map<int , std::vector<int> > _peds_t;
     std::string _measureAreaId;
     MeasurementArea_B* _areaForMethod_D;
     std::string _trajName;
     std::string _projectRootDir;
     std::string _scriptsLocation;
     bool _calcIndividualFD;
     polygon_2d _areaIndividualFD;
     bool _getProfile;
     bool _outputVoronoiCellData;
     bool _plotVoronoiCellData;
     bool _isOneDimensional;
     bool _cutByCircle;       //Adjust whether cut each original voronoi cell by a circle
     double _cutRadius;
     int _circleEdges;
     polygon_2d _geoPoly;
     double _geoMinX;  // LOWest vertex of the geometry (x coordinate)
     double _geoMinY;  //  LOWest vertex of the geometry (y coordinate)
     double _geoMaxX; // Highest vertex of the geometry
     double _geoMaxY;
     FILE* _fVoronoiRhoV;
     FILE* _fIndividualFD;
     double _grid_size_X;      // the size of the grid
     double _grid_size_Y;
     float _fps;
     bool OpenFileMethodD();
     bool OpenFileIndividualFD();
     std::string _geometryFileName;

     std::vector<polygon_2d> GetPolygons(std::vector<double>& XInFrame, std::vector<double>& YInFrame,
               std::vector<double>& VInFrame, std::vector<int>& IdInFrame);
     void OutputVoronoiResults(const std::vector<polygon_2d>&  polygons, const std::string& frid, const std::vector<double>& VInFrame);
     double GetVoronoiDensity(const std::vector<polygon_2d>& polygon, const polygon_2d & measureArea);
     double GetVoronoiDensity2(const std::vector<polygon_2d>& polygon, double* XInFrame, double* YInFrame, const polygon_2d& measureArea);
     double GetVoronoiVelocity(const std::vector<polygon_2d>& polygon, const std::vector<double>& Velocity, const polygon_2d & measureArea);
     void GetProfiles(const std::string& frameId, const std::vector<polygon_2d>& polygons, const std::vector<double>& velocity);
     void OutputVoroGraph(const std::string & frameId, std::vector<polygon_2d>& polygons, int numPedsInFrame,std::vector<double>& XInFrame,
               std::vector<double>& YInFrame,const std::vector<double>& VInFrame);
     void GetIndividualFD(const std::vector<polygon_2d>& polygon, const std::vector<double>& Velocity, const std::vector<int>& Id, const polygon_2d& measureArea, const std::string& frid);
     /**
      * Reduce the precision of the points to two digits
      * @param polygon
      */
     void CalcVoronoiResults1D(std::vector<double>& XInFrame, std::vector<double>& VInFrame, std::vector<int>& IdInFrame, const polygon_2d & measureArea, const std::string& frid);
     void ReducePrecision(polygon_2d& polygon);
     bool IsPedInGeometry(int frames, int peds, double **Xcor, double **Ycor, int  *firstFrame, int *lastFrame); //check whether all the pedestrians are in the geometry
     double getOverlapRatio(const double& left, const double& right, const double& measurearea_left, const double& measurearea_right);
     bool IsPointsOnOneLine(std::vector<double>& XInFrame, std::vector<double>& YInFrame);
};

#endif /* METHOD_D_H_ */
