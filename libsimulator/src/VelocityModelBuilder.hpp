/// Copyright © 2012-2022 Forschungszentrum Jülich GmbH
/// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "VelocityModel.hpp"
class VelocityModelBuilder
{
    double _aPed;
    double _DPed;
    double _aWall;
    double _DWall;
    std::vector<VelocityModelAgentParameters> _profiles{};

public:
    VelocityModelBuilder(double aPed, double DPed, double aWall, double DWall);
    VelocityModelBuilder& AddAgentParameterProfile(VelocityModelAgentParameters profile);
    VelocityModel Build();
};
