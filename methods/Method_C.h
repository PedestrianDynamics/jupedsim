/*
 * Method_C.h
 *
 *  Created on: 09.01.2015
 *      Author: JUNZHANG
 */

#ifndef METHOD_C_H_
#define METHOD_C_H_

using namespace std;
//using namespace boost::geometry;
#include "PedData.h"
#include "Analysis.h"

class Method_C
{
public:
	Method_C();
	virtual ~Method_C();
	bool Process (const PedData& peddata);
    void SetMeasurementArea (MeasurementArea_B* area);

private:
	map<int , vector<int> > _peds_t;
	int _minFrame;
	string _measureAreaId;
	MeasurementArea_B* _areaForMethod_C;
    string _trajName;
    string _projectRootDir;
    FILE *_fClassicRhoV;
	void OpenFileMethodC();
	void OutputClassicalResults(int frmNr, int numPedsInFrame, const vector<double>& XInFrame,const vector<double>& YInFrame, const vector<double>& VInFrame) const;
	double GetClassicalVelocity(const vector<double>& xs, const vector<double>& ys,  const vector<double>& VInFrame, int pednum) const;
	double GetClassicalDensity(const vector<double>& xs, const vector<double>& ys, int pednum, polygon_2d measurearea) const;

};

#endif /* METHOD_C_H_ */
