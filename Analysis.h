
#ifndef ANALYSIS_H_
#define ANALYSIS_H_

#include "general/ArgumentParser.h"
#include "geometry/Building.h"
#include "IO/OutputHandler.h"
#include "IO/IODispatcher.h"


#include <boost/geometry/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/geometries/adapted/c_array.hpp>
#include <boost/geometry/geometries/ring.hpp>
using namespace boost::geometry;
typedef model::d2::point_xy<double,  cs::cartesian> point_2d;
typedef model::polygon<point_2d> polygon_2d;
typedef model::ring<point_2d> ring;

#define CMtoM 0.01

extern OutputHandler* Log;

class Analysis {
private:
	FILE *_fClassicRhoV;
	FILE *_fVoronoiRhoV;
	FILE *_individualFD;
	FILE *_fN_t;

	Building* _building;
	IODispatcher* _iod;
	polygon_2d _geoPoly;
	polygon_2d _measureZone;
	string _trajectoryName;
	string _trajectoriesLocation;

	int _numFrames; // how much frames
	int *_tIn;   //the time for each pedestrian enter the measurement area
	int *_tOut;  //the time for each pedestrian exit the measurement area
	double _lengthMeasurementarea;  // the length of the measurement area
	int _maxNumofPed;  //the maximum index of the pedestrian in the trajectory data
	int _deltaF;		// half of the time interval that used to calculate instantaneous velocity of ped i.
							// here v_i = (X(t+deltaF) - X(t+deltaF))/(2*deltaF).   X is location.
	double **_xCor;
	double **_yCor;
	int *_firstFrame;   // Record the first frame of each pedestrian
	int *_lastFrame;	// Record the last frame of each pedestrian
	int _deltaT;   // the time interval to calculate the classic flow
	double _lineStartX ;  			//the coordinate of the line used to calculate the flow and velocity
	double _lineStartY ;
	double _lineEndX ;
	double _lineEndY ;
	bool _flowVelocity; 						// Method A (Zhang2011a)
	bool _fundamentalTinTout; 			// Method B (Zhang2011a)
	bool _classicMethod; 					// Method C //calculate and save results of classic in separate file
	bool _voronoiMethod;  					// Method D--Voronoi method
	bool _cutByCircle;  //Adjust whether cut each original voronoi cell by a circle
	bool _getProfile;   // Whether make field analysis or not
	bool _outputGraph;   // Whether output the data for plot the fundamental diagram each frame
	bool _calcIndividualFD; //Adjust whether analyze the individual density and velocity of each pedestrian in stationary state (ALWAYS VORONOI-BASED)
	char _vComponent; // to mark whether x, y or x and y coordinate are used when calculating the velocity
	std::vector<int> _accumPedsPassLine; // the accumulative pedestrians pass a line with time
	std::vector<double> _accumVPassLine;  // the accumulative instantaneous velocity of the pedestrians pass a line
	int _fps;												// Frame rate of data
	double _scaleX;   // the size of the grid
	double _scaleY;
	double _lowVertexX;// LOWest vertex of the geometry (x coordinate)
	double _lowVertexY; //  LOWest vertex of the geometry (y coordinate)
	double _highVertexX; // Highest vertex of the geometry
	double _highVertexY;

	//double GetVinFrame(int Tnow,int Tpast, int Tfuture, int Tfirst, int Tlast, double Xcor_past, double Xcor_now, double Xcor_future,double Ycor_past, double Ycor_now, double Ycor_future, char VComponent);
	double GetVinFrame(int Tnow,int Tpast, int Tfuture, int ID, int *Tfirst, int *Tlast, double **Xcor,double **Ycor, char VComponent);
	bool IsPassLine(double Line_startX,double Line_startY, double Line_endX, double Line_endY,double pt1_X, double pt1_Y,double pt2_X, double pt2_Y);
	void GetFundamentalTinTout(int *Tin, int *Tout, double *DensityPerFrame, int fps, double LengthMeasurementarea,int Nped, string ofile);
	void FlowRate_Velocity(int DeltaT, int fps, vector<int> AccumPeds, vector<double> AccumVelocity, string ofile);
	void GetIndividualFD(vector<polygon_2d> polygon, double* Velocity, polygon_2d measureArea);
	double Distance(double x1, double y1, double x2, double y2);
	double GetVoronoiDensity(vector<polygon_2d> polygon, polygon_2d measureArea);
	double GetVoronoiVelocity(vector<polygon_2d> polygon, double* Velocity, polygon_2d measureArea);
	double GetClassicalDensity(double *xs, double *ys, int pednum, polygon_2d measureArea);
	double GetClassicalVelocity(double *xs, double *ys, double *VInFrame, int pednum, polygon_2d measureArea);
	void GetProfiles(string frameId, vector<polygon_2d> polygons, double * velocity);
	void OutputVoroGraph(string frameId, vector<polygon_2d> polygons, int numPedsInFrame, double* XInFrame, double* YInFrame,double* VInFrame);
	void DistributionOnLine(int *frequency,int fraction, double Line_startX,double Line_startY, double Line_endX, double Line_endY,double pt1_X, double pt1_Y,double pt2_X, double pt2_Y);

	//create a file and the directory structure if needed.
	FILE* CreateFile(const string& filename);
	int mkpath(const char* file_path, mode_t mode=0755);

public:

	Analysis();
	virtual ~Analysis();

	int GetNPedsGlobal() const;
	Building* GetBuilding() const;
	void InitArgs(ArgumentParser *args);
	int InitAnalysis();
	void ReadTrajetories(const string& trajectoriesFile);
	polygon_2d ReadGeometry(const string& geometryFile);
	int RunAnalysis();

};

#endif /*ANALYSIS_H_*/
