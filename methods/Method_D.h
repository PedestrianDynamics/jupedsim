/*
 * Method_D.h
 *
 *  Created on: 12.01.2015
 *      Author: JUNZHANG
 */

#ifndef METHOD_D_H_
#define METHOD_D_H_
#include <vector>
#include "PedData.h"
#include "Analysis.h"
#include "VoronoiDiagram.h"
#ifdef __linux__
#include <sys/stat.h>
#include <dirent.h>
#elif   __APPLE__
#include <sys/stat.h>
#include <dirent.h>
#else
#include <direct.h>
#endif
class Method_D {
public:
     Method_D();
     virtual ~Method_D();
     bool Process (const PedData& peddata);
     const void SetCalculateIndividualFD(bool individualFD);

     //TODO: combine the three methods Setcutbycircle(radius,edge)
     const void SetCutByCircle(bool iscut);
     const void SetCuttingCircleRadius(double radius);
     const void SetCuttingCircleEdges(int edges);

     const void SetGeometryPolygon(polygon_2d geometryPolygon);
     const void SetGeometryBoundaries(double minX, double minY, double maxX, double maxY);
     const void SetScale(double x, double y);

     const void SetCalculateProfiles(bool calcProfile);
     const void SetOutputVoronoiCellData(bool outputCellData);
     void SetMeasurementArea (MeasurementArea_B* area);

private:
     map<int , vector<int> > _peds_t;
     string _measureAreaId;
     MeasurementArea_B* _areaForMethod_D;
     string _trajName;
     string _projectRootDir;
     bool _calcIndividualFD;
     bool _getProfile;
     bool _outputVoronoiCellData;
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
     double _scaleX;      // the size of the grid
     double _scaleY;
     bool OpenFileMethodD();
     bool OpenFileIndividualFD();
     vector<polygon_2d> GetPolygons(int NrInFrm);
     vector<polygon_2d> GetPolygons(vector<int> ids, vector<double>& XInFrame, vector<double>& YInFrame,
               vector<double>& VInFrame, vector<int>& IdInFrame);
     void OutputVoronoiResults(const vector<polygon_2d>&  polygons, int frid, const vector<double>& VInFrame);
     double GetVoronoiDensity(const vector<polygon_2d>& polygon, const polygon_2d& measureArea);
     double GetVoronoiDensity2(const vector<polygon_2d>& polygon, double* XInFrame, double* YInFrame, const polygon_2d& measureArea);
     double GetVoronoiVelocity(const vector<polygon_2d>& polygon, const vector<double>& Velocity, const polygon_2d& measureArea);
     void GetProfiles(const string& frameId, const vector<polygon_2d>& polygons, const vector<double>& velocity);
     void OutputVoroGraph(const string & frameId, const vector<polygon_2d>& polygons, int numPedsInFrame, const vector<double>& XInFrame,
               const vector<double>& YInFrame,const vector<double>& VInFrame);
     void GetIndividualFD(const vector<polygon_2d>& polygon, const vector<double>& Velocity, const vector<int>& Id, const polygon_2d& measureArea, int frid);

};

#endif /* METHOD_D_H_ */
