// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "route.hpp"

/// What Python sees as jupedsim.Rule: one type for every kind of rule
/// description, created through its static factories.
struct PyRule {
    RuleDescription description;
};
