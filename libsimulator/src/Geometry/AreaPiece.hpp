// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "CfgCgal.hpp"

#include <cstddef>

/// A piece of a polygon that lies in one region. The polygon is clipped to the
/// region's footprint, and only the outer boundary is taken (holes are ignored).
struct AreaPiece {
    Poly polygon;
    std::size_t region;
};
