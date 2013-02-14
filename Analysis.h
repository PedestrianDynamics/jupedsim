/*Simulation.h:
  The Simulation class represents a simulation of pedestrians
  based on a certain model in a specific scenario. A simulation is defined by
  various parameters and functions.
  Copyright (C) <2009-2010>  <Jonas Mehlich and Mohcine Chraibi>

  This file is part of OpenPedSim.

  OpenPedSim is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  any later version.

  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  OpenPedSim is distributed in the hope that it will be useful,
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Foobar. If not, see <http://www.gnu.org/licenses/>.
 */

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


extern OutputHandler* Log;

class Analysis {
private:
	//int pNPeds;
	//double pTmax;
	//double pDt;
	Building* _building;
	IODispatcher* _iod;
	polygon_2d pGeoPoly;
	polygon_2d pMeasureZone;
	string TrajectoryName;
	string TrajectoryFile;
	int _numFrames; // how much frames


	int *Tin;   //the time for each pedestrian enter the measurement area
	int *Tout;  //the time for each pedestrian exit the measurement area
	//float *DensityPerFrame; // the density in measurement area each frame. this will be used to calculate the fundamental diagram with Tin Tout
	float LengthMeasurementarea;  // the length of the measurement area
	int MaxNumofPed;  //the maximum index of the pedestrian in the trajectory data
	int DeltaF;		// half of the time interval that used to calculate instantaneous velocity of ped i.
	// here v_i = (X(t+deltaF) - X(t+deltaF))/(2*deltaF).   X is location.
	float **Xcor;
	float **Ycor;
	int *firstFrame;   // Record the first frame of each pedestrian
	int *lastFrame;	// Record the last frame of each pedestrian
	int DeltaT;   // the time interval to calculate the classic flow
	float Line_startX ;  			//the coordinate of the line used to calculate the flow and velocity
	float Line_startY ;
	float Line_endX ;
	float Line_endY ;
	bool IsFlowVelocity; 						// Method A (Zhang2011a)
	bool IsFundamentalTinTout; 			// Method B (Zhang2011a)
	bool IsClassicMethod; 					// Method C //calculate and save results of classic in separate file
	bool IsVoronoiMethod;  					// Method D--Voronoi method
	bool cutbycircle;  //Adjust whether cut each original voronoi cell by a circle
	bool IsGetProfile;   // Whether make field analysis or not
	bool IsOutputGraph;   // Whether output the data for plot the fundamental diagram each frame
	bool IscalcIndividualFD; //Adjust whether analyze the individual density and velocity of each pedestrian in stationary state (ALWAYS VORONOI-BASED)
	char VComponent; // to mark whether x, y or x and y coordinate are used when calculating the velocity
	std::vector<int> AccumPedsPassLine; // the accumulative pedestrians pass a line with time
	std::vector<float> AccumVPassLine;  // the accumulative instantaneous velocity of the pedestrians pass a line
	int fps;												// Frame rate of data
	FILE *fClassic_rho_v;
	FILE *fVoronoi_rho_v;
	FILE *IndividualFD;

	float scale_x;   // the size of the grid
	float scale_y;
	float low_ed_x;// LOWest vertex of the geometry (x coordinate)
	float low_ed_y; //  LOWest vertex of the geometry (y coordinate)
	float high_ed_x; // Highest vertex of the geometry
	float high_ed_y;
	float GetVinFrame(int Tnow,int Tpast, int Tfuture, int Tfirst, int Tlast, float Xcor_past, float Xcor_now, float Xcor_future,float Ycor_past, float Ycor_now, float Ycor_future, char VComponent);
	bool IsPassLine(float Line_startX,float Line_startY, float Line_endX, float Line_endY,float pt1_X, float pt1_Y,float pt2_X, float pt2_Y);
	void getFundamentalTinTout(int *Tin, int *Tout, float *DensityPerFrame, int fps, int LengthMeasurementarea,int Nped, string ofile);
	void FlowRate_Velocity(int DeltaT, int fps, vector<int> AccumPeds, vector<float> AccumVelocity, string ofile);
	void getIndividualFD(vector<polygon_2d> polygon, float* Velocity, polygon_2d measureArea);
	float distance(float x1, float y1, float x2, float y2);
	float getVoronoiDensity(vector<polygon_2d> polygon, polygon_2d measureArea);
	float getVoronoiVelocity(vector<polygon_2d> polygon, float* Velocity, polygon_2d measureArea);
	float getClassicalDensity(float *xs, float *ys, int pednum, polygon_2d measureArea);
	float getClassicalVelocity(float *xs, float *ys, float *VInFrame, int pednum, polygon_2d measureArea);
	void GetProfiles(string frameId, vector<polygon_2d> polygons, float * velocity);
	void OutputVoroGraph(string frameId, vector<polygon_2d> polygons, int numPedsInFrame, float* XInFrame, float* YInFrame,float* VInFrame);
	void DistributionOnLine(int *frequency,int fraction, float Line_startX,float Line_startY, float Line_endX, float Line_endY,float pt1_X, float pt1_Y,float pt2_X, float pt2_Y);
public:

	Analysis();
	virtual ~Analysis();

	int GetNPedsGlobal() const;
	Building* GetBuilding() const;

	void InitArgs(ArgumentParser *args);
	int InitAnalysis();
	void ReadTrajetories(string trajectoriesFile);
	polygon_2d ReadGeometry(const string& geometryFile);
	int RunAnalysis();

};

#endif /*ANALYSIS_H_*/
