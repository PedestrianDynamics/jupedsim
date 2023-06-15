/// Copyright © 2012-2022 Forschungszentrum Jülich GmbH
/// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "GCFMModel.hpp"
#include <vector>

class GCFMModelBuilder
{
    double _nuped;
    double _nuwall;
    double _dist_effPed;
    double _dist_effWall;
    double _intp_widthped;
    double _intp_widthwall;
    double _maxfped;
    double _maxfwall;
    std::vector<GCFMModelAgentParameters> _profiles{};

public:
    GCFMModelBuilder(
        double nuped,
        double nuwall,
        double dist_effPed,
        double dist_effWall,
        double intp_widthped,
        double intp_widthwall,
        double maxfped,
        double maxfwall);
    GCFMModelBuilder& AddAgentParameterProfile(GCFMModelAgentParameters profile);
    GCFMModel Build();
};
