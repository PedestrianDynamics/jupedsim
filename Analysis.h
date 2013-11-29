
#ifndef ANALYSIS_H_
#define ANALYSIS_H_

#include "general/ArgumentParser.h"
#include "tinyxml/tinyxml.h"
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

	//polygon_2d _measureZone;

	int _numFrames; // how much frames
	int *_tIn;   //the time for each pedestrian enter the measurement area
	int *_tOut;  //the time for each pedestrian exit the measurement area
	int _maxNumofPed;  //the maximum index of the pedestrian in the trajectory data
	int _deltaF;		// half of the time interval that used to calculate instantaneous velocity of ped i.
							// here v_i = (X(t+deltaF) - X(t+deltaF))/(2*deltaF).   X is location.
	double **_xCor;
	double **_yCor;
	int *_firstFrame;   // Record the first frame of each pedestrian
	int *_lastFrame;	// Record the last frame of each pedestrian
	int _deltaT;   // the time interval to calculate the classic flow

	double *DensityPerFrame; // the measured density in each frame
	bool *PassLine;

	bool _flowVelocity; 				// Method A (Zhang2011a)
	bool _fundamentalTinTout; 			// Method B (Zhang2011a)
	bool _classicMethod; 					// Method C //calculate and save results of classic in separate file
	bool _voronoiMethod;  					// Method D--Voronoi method

	MeasurementArea_L* _areaForMethod_A;
	MeasurementArea_B* _areaForMethod_B;
	MeasurementArea_B* _areaForMethod_C;
	MeasurementArea_B* _areaForMethod_D;

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

	TiXmlElement* xRootNode;

	double GetVinFrame(int Tnow,int Tpast, int Tfuture, int ID, int *Tfirst, int *Tlast, double **Xcor,double **Ycor, char VComponent);
	bool IsPassLine(double Line_startX,double Line_startY, double Line_endX, double Line_endY,double pt1_X, double pt1_Y,double pt2_X, double pt2_Y);
	void GetFundamentalTinTout(int *Tin, int *Tout, double *DensityPerFrame, int fps, double LengthMeasurementarea,int Nped, const std::string & ofile);
	void FlowRate_Velocity(int DeltaT, int fps, const std::vector<int>& AccumPeds, const std::vector<double>& AccumVelocity, const std::string& ofile);
	void GetIndividualFD(const std::vector<polygon_2d>& polygon, double* Velocity, int* Id, const polygon_2d& measureArea, int frid);
	double Distance(double x1, double y1, double x2, double y2);
	double GetVoronoiDensity(const std::vector<polygon_2d>& polygon, const polygon_2d& measureArea);
	double GetVoronoiVelocity(const std::vector<polygon_2d>& polygon, double* Velocity, const polygon_2d& measureArea);
	double GetClassicalDensity(double *xs, double *ys, int pednum, const polygon_2d& measureArea);
	double GetClassicalVelocity(double *xs, double *ys, double *VInFrame, int pednum, const polygon_2d& measureArea);
	void GetProfiles(const std::string& frameId, const std::vector<polygon_2d>& polygons, double * velocity, const std::string& filename);
	void OutputVoroGraph(const std::string & frameId, const std::vector<polygon_2d>& polygons, int numPedsInFrame, double* XInFrame, double* YInFrame,double* VInFrame, const std::string& filename);
	void DistributionOnLine(int *frequency,int fraction, double Line_startX,double Line_startY, double Line_endX, double Line_endY,double pt1_X, double pt1_Y,double pt2_X, double pt2_Y);
	//void InitializeVariables(const  std::string& filename, const  std::string& path);
	void InitializeVariables(TiXmlElement* xRootNode);
	int getPedsNumInFrame(TiXmlElement* xFrame);
	//create a file and the directory structure if needed.
	FILE* CreateFile(const std::string& filename);



#ifdef __linux__
	int mkpath(char* file_path, mode_t mode=0755);
#else
	int mkpath(char* file_path);
#endif

public:

	Analysis();
	virtual ~Analysis();

	void InitArgs(ArgumentParser *args);
    //
	void InitializeFiles(const std::string& file);
    std::string GetBasename(const std::string& str);
    std::string GetFilename (const std::string& str);

	polygon_2d ReadGeometry(const std::string& geometryFile);
	int RunAnalysis(const std::string& file, const std::string& path);

};

#endif /*ANALYSIS_H_*/
