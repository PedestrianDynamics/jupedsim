/*
 * Analysis.h
 *
 *  Created on: Apr 28, 2010
 *      Author: ZhangJun
 */

#ifndef ANALYSIS_H_
#define ANALYSIS_H_

#include <boost/geometry/geometry.hpp>
#include <boost/geometry/algorithms/intersection.hpp>
#include <boost/geometry/algorithms/intersects.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/geometries/adapted/c_array.hpp>

using namespace std;
using namespace boost::geometry;
typedef model::d2::point_xy<double, cs::cartesian> point_2d;
typedef model::polygon<point_2d> polygon_2d;

//BOOST_GEOMETRY_REGISTER_C_ARRAY_CS(cs::cartesian)


class Analysis {
public:
	Analysis();
	virtual ~Analysis();
	void initial(string datafile);
	float getVoronoiDensity(vector<polygon_2d> polygon, polygon_2d measureArea);
	float getclassicalDensity(float *xs, float *ys, int pointnum, polygon_2d measureArea);
	float getVoronoiVelocity(vector<polygon_2d> polygon, float* Velocity, polygon_2d measureArea);
	void getIndividualfundmentaldiagram(vector<polygon_2d> polygon, float* Velocity, polygon_2d measureArea);
	float distance(float x1, float y1, float x2, float y2);
	void DistributionOnLine(int *frequency,int fraction, float Line_startX,float Line_startY, float Line_endX, float Line_endY,float pt1_X, float pt1_Y,float pt2_X, float pt2_Y);
	bool IsPassLine(float Line_startX,float Line_startY, float Line_endX, float Line_endY,float pt1_X, float pt1_Y,float pt2_X, float pt2_Y);
	void getFundamentalTinTout(int *Tin, int *Tout, float *DensityPerFrame, int fps, int LengthMeasurementarea, int Nped,string datafile);
	void FlowRate_Velocity(int DeltaT, int fps, vector<int> AccumPeds, vector<float> AccumVelocity, string datafile);
};

#endif /* ANALYSIS_H_ */
