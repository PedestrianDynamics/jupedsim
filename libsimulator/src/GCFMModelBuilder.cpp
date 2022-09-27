/// Copyright © 2012-2022 Forschungszentrum Jülich GmbH
/// SPDX-License-Identifier: LGPL-3.0-or-later
#include "GCFMModelBuilder.hpp"
#include "GCFMModel.hpp"

GCFMModelBuilder::GCFMModelBuilder(
    double nuped,
    double nuwall,
    double dist_effPed,
    double dist_effWall,
    double intp_widthped,
    double intp_widthwall,
    double maxfped,
    double maxfwall)
    : _nuped(nuped)
    , _nuwall(nuwall)
    , _dist_effPed(dist_effPed)
    , _dist_effWall(dist_effWall)
    , _intp_widthped(intp_widthped)
    , _intp_widthwall(intp_widthwall)
    , _maxfped(maxfped)
    , _maxfwall(maxfwall)
{
}

GCFMModelBuilder& GCFMModelBuilder::AddAgentParameterProfile(GCFMModelAgentParameters profile)

{
    _profiles.emplace_back(profile);
    return *this;
}

GCFMModel GCFMModelBuilder::Build()
{
    return GCFMModel(
        _nuped,
        _nuwall,
        _dist_effPed,
        _dist_effWall,
        _intp_widthped,
        _intp_widthwall,
        _maxfped,
        _maxfwall,
        _profiles);
}
