/*
 * Method_A.h
 *
 *  Created on: Jan 5, 2015
 *      Author: piccolo
 */

#ifndef METHOD_A_H_
#define METHOD_A_H_

#include <string>
#include "MeasurementArea.h"
#include "PedData.h"
#include <vector>
#include "general/Macros.h"
#include "tinyxml/tinyxml.h"
#include "IO/OutputHandler.h"
#define M2CM 100
#define CMtoM 0.01

using namespace boost::geometry;
typedef boost::geometry::model::segment<boost::geometry::model::d2::point_xy<double> > segment;

using namespace std;

class Method_A
{
public:
     Method_A();
     virtual ~Method_A();
     void SetMeasurementArea (MeasurementArea_L* area);
     void SetTimeInterval(const int& deltaT);
     bool Process (const PedData& peddata);

private:
     string _trajName;
     string _measureAreaId;
     MeasurementArea_L* _areaForMethod_A;
     string _projectRootDir;

     vector<int> _accumPedsPassLine; // the accumulative pedestrians pass a line with time
     vector<double> _accumVPassLine; // the accumulative instantaneous velocity of the pedestrians pass a line
     map<int , vector<int> > _peds_t;

     double** _xCor;
     double** _yCor;
     int* _firstFrame;

     bool *_passLine;
     int _classicFlow;    // the number of pedestrians pass a line in a certain time
     double _vDeltaT;    // define this is to measure cumulative velocity each pedestrian pass a measure line each time step to calculate the <v>delat T=sum<vi>/N
     int _deltaT;
     /**
      * Calculate the Flow rate during a certain time interval DeltaT and the mean velocity passing a line.
      * Note: here the time interval in calculating the flow rate is modified.
      * it is the actual time between the first person and last person
      * passing the line in DeltaT.
      *
      * @param DeltaT
      * @param fps
      * @param AccumPeds
      * @param AccumVelocity
      * @param ofile
      */
     void WriteFile_N_t(string data);

     void FlowRate_Velocity(int fps, const vector<int>& AccumPeds, const std::vector<double>& AccumVelocity);

     bool IsPassLine(double Line_startX,double Line_startY, double Line_endX, double Line_endY,double pt1_X, double pt1_Y,double pt2_X, double pt2_Y);

     void GetAccumFlowVelocity(int frame, const vector<int>& ids, const vector<double>& VInFrame);
};

#endif /* METHOD_A_H_ */
