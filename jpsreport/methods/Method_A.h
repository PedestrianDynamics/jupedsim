/**
 * \file        Method_A.h
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

#ifndef METHOD_A_H_
#define METHOD_A_H_

#include "../IO/OutputHandler.h"
#include "../general/Macros.h"
#include "../tinyxml/tinyxml.h"
#include "MeasurementArea.h"
#include "PedData.h"

#include <boost/numeric/ublas/io.hpp>
#include <boost/numeric/ublas/matrix.hpp>

typedef boost::geometry::model::segment<boost::geometry::model::d2::point_xy<double>> segment;

namespace ub = boost::numeric::ublas;
using namespace boost::geometry;

class Method_A
{
public:
    Method_A();
    virtual ~Method_A();
    void SetMeasurementArea(MeasurementArea_L * area);
    void SetTimeInterval(int deltaT);
    bool Process(
        const PedData & peddata,
        const fs::path & scriptsLocation,
        const double & zPos_measureArea);

private:
    fs::path _trajName;
    std::string _measureAreaId;
    MeasurementArea_L * _areaForMethod_A;

    fs::path _projectRootDir;
    fs::path _scriptsLocation;
    fs::path _outputLocation;

    std::vector<int> _accumPedsPassLine; // the accumulative pedestrians pass a line with time
    std::vector<double>
        _accumVPassLine; // the accumulative instantaneous velocity of the pedestrians pass a line
    std::map<int, std::vector<int>> _peds_t;

    ub::matrix<double> _xCor;
    ub::matrix<double> _yCor;
    int * _firstFrame;
    float _fps;

    bool * _passLine;
    int _classicFlow; // the number of pedestrians pass a line in a certain time
    double
        _vDeltaT; // define this is to measure cumulative velocity each pedestrian pass a measure line each time step to calculate the <v>delat T=sum<vi>/N
    int _deltaT;

    int _minId;
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
    void WriteFile_N_t(std::string data);

    void FlowRate_Velocity(
        int fps,
        const std::vector<int> & AccumPeds,
        const std::vector<double> & AccumVelocity);

    bool IsPassLine(
        double Line_startX,
        double Line_startY,
        double Line_endX,
        double Line_endY,
        double pt1_X,
        double pt1_Y,
        double pt2_X,
        double pt2_Y);

    void GetAccumFlowVelocity(
        int frame,
        const std::vector<int> & ids,
        const std::vector<double> & VInFrame);
};

#endif /* METHOD_A_H_ */
