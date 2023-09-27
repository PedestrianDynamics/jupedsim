// Copyright © 2012-2023 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "GeneralizedCentrifugalForceModelUpdate.hpp"
#include "VelocityModelUpdate.hpp"

#include <variant>

using OperationalModelUpdate =
    std::variant<GeneralizedCentrifugalForceModelUpdate, VelocityModelUpdate>;
