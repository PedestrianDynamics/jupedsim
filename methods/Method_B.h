/**
 * \file        Method_B.h
 * \date        Oct 10, 2014
 * \version     v0.7
 * \copyright   <2009-2017> Forschungszentrum Jülich GmbH. All rights reserved.
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

#ifndef METHOD_B_H_
#define METHOD_B_H_

#include "PedData.h"
#include "MeasurementArea.h"
#include "Method_C.h"


#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/io.hpp>
namespace ub=boost::numeric::ublas;


class Method_B
{
public:
     Method_B();
     virtual ~Method_B();
     bool Process (const PedData& peddata);
     void SetMeasurementArea (MeasurementArea_B* area);

private:

     fs::path  _trajName;
     fs::path  _projectRootDir;
     fs::path _outputLocation;
     std::string _measureAreaId;
     std::map<int , std::vector<int> > _peds_t;
     MeasurementArea_B* _areaForMethod_B;
     int _NumPeds;
     float _fps;
     ub::matrix<double> _xCor;
     ub::matrix<double> _yCor;
     int *_tIn;           //the time for each pedestrian enter the measurement area
     int *_tOut;          //the time for each pedestrian exit the measurement area
     std::vector<Point> _entrancePoint ; // where pedestrian enters the measurement area
     std::vector<Point> _exitPoint; // where pedestrian leaves the measurement area
     double *_DensityPerFrame; // the measured density in each frame
     void GetTinTout(int numFrames);
     void GetFundamentalTinTout(double *DensityPerFrame,double LengthMeasurementarea);
};

#endif /* METHOD_B_H_ */
