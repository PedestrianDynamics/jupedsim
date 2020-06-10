/**
 * \file        Method_C.h
 * \date        Oct 10, 2014
 * \version     v0.7
 * \copyright   <2009-2015> Forschungszentrum J��lich GmbH. All rights reserved.
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

#include "../Analysis.h"
#include "PedData.h"


class Method_C
{
public:
    Method_C();
    virtual ~Method_C();
    bool Process(const PedData & peddata, const double & zPos_measureArea);
    void SetMeasurementArea(MeasurementArea_B * area);

private:
    std::map<int, std::vector<int>> _peds_t;
    int _minFrame;
    std::string _measureAreaId;
    MeasurementArea_B * _areaForMethod_C;

    fs::path _trajName;
    fs::path _projectRootDir;
    fs::path _outputLocation;

    float _fps;
    FILE * _fClassicRhoV;
    void OpenFileMethodC();
    void OutputClassicalResults(
        int frmNr,
        int numPedsInFrame,
        const std::vector<double> & XInFrame,
        const std::vector<double> & YInFrame,
        const std::vector<double> & VInFrame) const;
    double GetClassicalVelocity(
        const std::vector<double> & xs,
        const std::vector<double> & ys,
        const std::vector<double> & VInFrame,
        int pednum) const;
    double GetClassicalDensity(
        const std::vector<double> & xs,
        const std::vector<double> & ys,
        int pednum,
        polygon_2d measurearea) const;
};

#endif /* METHOD_C_H_ */
