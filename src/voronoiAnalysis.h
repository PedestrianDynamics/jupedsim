/*
 * VoronoiAnalysis.h
 *
 *  Created on: Mar 17, 2011
 *      Author: JunZhang
 */

#ifndef VORONOIANALYSIS_H_
#define VORONOIANALYSIS_H_
#include <vector>

	int *Tin1;   //the time for each pedestrian enter the measurement area
	int *Tout1;  //the time for each pedestrian exit the measurement area
	bool *IsRecord_Ped;
	float *DensityPerFrame; // the density in measurement area each frame. this will be used to calculate the fundamental diagram with Tin Tout
    float LengthMeasurementarea;  // the length of the measurement area
    int MaxNumofPed=0;  //the maximum index of the pedestrian in the trajectory data
    int DeltaT;   // the time interval to calculate the classic flow
    float Line_startX ;  			//the coordinate of the line used to calculate the flow and velocity
    float Line_startY ;
    float Line_endX ;
    float Line_endY ;
    bool cutbycircle;  //Adjust whether cut each original voronoi cell by a circle
    bool fieldAnalysis;   // Whether make field analysis or not
    bool IsOutputGraph;   // Whether output the data for plot the fundamental diagram each frame
    bool calcIndividualfunddata; //Adjust whether analyze the individual density and velocity of each pedestrian in stationary state (ALWAYS VORONOI-BASED)
    bool use_Vx;  // only the x coordinate is used when calculating the velocity
    bool use_Vy; // only the y coordinate is used when calculating the velocity
    bool use_Vxy; // both x and y coordinate are used when calculating the velocity
    std::vector<int> AccumPedsPassLine; // the accumulative pedestrians pass a line with time
    std::vector<float> AccumVPassLine;  // the accumulative instantaneous velocity of the pedestrians pass a line
	int fps;												// Frame rate of data
	int ClassicFlow=0; // the number of pedestrians pass a line in a certain time
	float V_deltaT=0;   // define this is to measure cumulative velocity each pedestrian pass a measure line each time step to calculate the <v>delat T=sum<vi>/N
	FILE *fClassic_rho_v;
	FILE *fVoronoi_rho_v;

	int NRow;  // the number of rows that the geometry will be discretized for field analysis
	int NColumn;	//the number of columns that the geometry will be discretized for field analysis
	float scale_x;   // the size of the grid
	float scale_y;
	float low_ed_x;//0; //todo LOWest vertex of the geometry (x coordinate)
	float low_ed_y; // 0 todo  LOWest vertex of the geometry (y coordinate)

#endif /* VORONOIANALYSIS_H_ */
