/**
 * \file        Method_J.h
 * \date        Oct 29, 2019
 * \version     v0.8.5
 * \copyright   <2009-2019> Forschungszentrum Juelich GmbH. All rights reserved.
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

#ifndef Method_J_H_
#define Method_J_H_

#include "PedData.h"
#include "../Analysis.h"
#include "VoronoiDiagram.h"



//handle more than two person are in one line
#define dmin 200
#define offset 200


class Method_J
{
public:
     Method_J();
     virtual ~Method_J();
     bool Process (const PedData& peddata,const fs::path& scriptsLocation, const double& zPos_measureArea);
     void SetCalculateIndividualFD(bool individualFD);
     void Setcutbycircle(double radius,int edges);
     void SetGeometryPolygon(polygon_2d geometryPolygon);
     void SetGeometryFileName(const fs::path& geometryFile);
     void SetGeometryBoundaries(double minX, double minY, double maxX, double maxY);
     void SetGridSize(double x, double y);
     void SetCalculateProfiles(bool calcProfile);
     void SetMeasurementArea (MeasurementArea_B* area);
     void SetDimensional (bool dimension);
     void SetTrajectoriesLocation(const fs::path& trajectoryPath);
     void SetStartFrame(int startFrame);
     void SetStopFrame(int stopFrame);

private:
     std::map<int , std::vector<int> > _peds_t;
     std::string _measureAreaId;
     MeasurementArea_B* _areaForMethod_J;
     fs::path _trajName;
     fs::path _projectRootDir;
     fs::path _outputLocation;
     fs::path _scriptsLocation;
     bool _calcIndividualFD;
     polygon_2d _areaIndividualFD;
     bool _getProfile;
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
     bool OpenFileMethodVoronoi();
     bool OpenFileIndividualFD();
     fs::path _geometryFileName;
     fs::path _trajectoryPath;
     int _startFrame;
     int _stopFrame;


     std::vector<std::pair<polygon_2d, int> >  GetPolygons(std::vector<double>& XInFrame, std::vector<double>& YInFrame,
                                                           std::vector<double>& VInFrame, std::vector<int>& IdInFrame);
     void OutputVoronoiResults(const std::vector<polygon_2d>&  polygons, const std::string& frid, const std::vector<double>& VInFrame);
     std::tuple<double,double> GetVoronoiDensityVelocity(const std::vector<polygon_2d>& polygon, const std::vector<double>& Velocity, const polygon_2d & measureArea);
     void GetProfiles(const std::string& frameId, const std::vector<polygon_2d>& polygons, const std::vector<double>& velocity);
     void OutputVoroGraph(const std::string & frameId,  std::vector<std::pair<polygon_2d, int> >& polygons, int numPedsInFrame,const std::vector<double>& VInFrame);
     void GetIndividualFD(const std::vector<polygon_2d>& polygon, const std::vector<double>& Velocity, const std::vector<int>& Id, const polygon_2d& measureArea, const std::string& frid, std::vector<double>& XInFrame, std::vector<double>& YInFrame, std::vector<double>& ZInFrame);
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

#endif /* Method_J_H_ */
