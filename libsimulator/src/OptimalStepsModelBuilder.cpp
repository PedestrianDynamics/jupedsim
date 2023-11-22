// Copyright © 2012-2023 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "OptimalStepsModelBuilder.hpp"

OptimalStepsModelBuilder::OptimalStepsModelBuilder(
    double aPed,
    double DPed,
    double aWall,
    double DWall)
    : _aPed(aPed), _DPed(DPed), _aWall(aWall), _DWall(DWall)
{
}

OptimalStepsModel OptimalStepsModelBuilder::Build()
{
    return OptimalStepsModel(_aPed, _DPed, _aWall, _DWall);
}
