// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "jupedsim/operational_model.h"

#include <OperationalModel.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Operational Model
////////////////////////////////////////////////////////////////////////////////////////////////////
void JPS_OperationalModel_Free(JPS_OperationalModel handle)
{
    delete reinterpret_cast<OperationalModel*>(handle);
}
