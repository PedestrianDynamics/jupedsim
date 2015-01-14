/*
 * Method_B.h
 *
 *  Created on: 12.01.2015
 *      Author: JUNZHANG
 */

#ifndef METHOD_B_H_
#define METHOD_B_H_

using namespace std;
#include "PedData.h"
#include "MeasurementArea.h"
#include "Method_C.h"
#include "Analysis.h"

class Method_B {
public:
     Method_B();
     virtual ~Method_B();
     bool Process (const PedData& peddata);
     void SetMeasurementArea (MeasurementArea_B* area);

private:
     string _trajName;
     string _projectRootDir;
     string _measureAreaId;
     map<int , vector<int> > _peds_t;
     MeasurementArea_B* _areaForMethod_B;
     int _NumPeds;
     int _fps;
     double** _xCor;
     double** _yCor;
     int *_tIn;           //the time for each pedestrian enter the measurement area
     int *_tOut;          //the time for each pedestrian exit the measurement area
     double *_DensityPerFrame; // the measured density in each frame
     void GetTinTout(int numFrames);
     void GetFundamentalTinTout(double *DensityPerFrame,double LengthMeasurementarea);
};

#endif /* METHOD_B_H_ */
