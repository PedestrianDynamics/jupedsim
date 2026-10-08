// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "Journey.hpp"

/// What Python sees as jupedsim.Transition: one type for every kind of transition
/// description, created through its static factories.
struct PyTransition {
    TransitionDescription description;
};
