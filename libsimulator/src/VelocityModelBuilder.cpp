/// Copyright © 2012-2022 Forschungszentrum Jülich GmbH
/// SPDX-License-Identifier: LGPL-3.0-or-later
#include "VelocityModelBuilder.hpp"

VelocityModelBuilder::VelocityModelBuilder(double aPed, double DPed, double aWall, double DWall)
    : _aPed(aPed), _DPed(DPed), _aWall(aWall), _DWall(DWall)
{
}

VelocityModelBuilder&
VelocityModelBuilder::AddAgentParameterProfile(VelocityModelAgentParameters profile)
{
    _profiles.emplace_back(profile);
    return *this;
}

VelocityModel VelocityModelBuilder::Build()
{
    return VelocityModel(_aPed, _DPed, _aWall, _DWall, _profiles);
}
