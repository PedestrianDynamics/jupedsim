/**
 * \file        Method_C.h
 * \date        Oct 10, 2014
 * \version     v0.6
 * \copyright   <2009-2014> Forschungszentrum J��lich GmbH. All rights reserved.
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
