// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "jupedsim/build_info.h"

#include <BuildInfo.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////
/// BuildInfo
////////////////////////////////////////////////////////////////////////////////////////////////////
JPS_BuildInfo JPS_GetBuildInfo()
{
    return JPS_BuildInfo{
        GIT_COMMIT_HASH.c_str(),
        GIT_COMMIT_DATE.c_str(),
        GIT_BRANCH.c_str(),
        COMPILER.c_str(),
        COMPILER_VERSION.c_str(),
        LIBRARY_VERSION.c_str()};
}
