/**
 * \file        ToxicityAnalysis.h
 * \date        Jan 1, 2014
 * \version     v0.7
 * \copyright   <2009-2015> Forschungszentrum Jülich GmbH. All rights reserved.
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
 * detects smoke in the next rooms
 *
 *
 **/
#pragma once

#include "ToxicityOutputhandler.h"

#include <memory>
#include <string>

class Pedestrian;
class Point;
class FDSMeshStorage;

class ToxicityAnalysis
{

public:

     ToxicityAnalysis(const std::string & projectFilename, double _fps);
    virtual ~ToxicityAnalysis();

    void HazardAnalysis(Pedestrian *);
    double GetFDSQuantity(const Pedestrian *, std::string);
    void WriteOutHazardAnalysis(const Pedestrian *, double, double, double, double, double, double, double,
                             double, double, double, double, double);

    void set_FMStorage(const std::shared_ptr<FDSMeshStorage> fmStorage);
    const std::shared_ptr<FDSMeshStorage> get_FMStorage();


    bool LoadJPSfireInfo(const std::string projectFilename);
    bool ConductToxicityAnalysis();

    double CalculateFEDIn(Pedestrian *, double CO2, double CO, double O2, double HCN, double FED_In);
    double CalculateFEDHeat(Pedestrian *, double T, double FED_Heat);
     const std::string getProjectFileName(void);
private:
    std::shared_ptr<FDSMeshStorage> _FMStorage;
    std::shared_ptr<ToxicityOutputHandler> _outputhandler;
    const std::string _projectFileName;
    double _dt;
    double _fps;
    double _t_prev;
    void InitializeWriteOut();
};
