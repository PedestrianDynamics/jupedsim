/// Copyright © 2012-2022 Forschungszentrum Jülich GmbH
/// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "IteratorPair.hpp"
#include "Point.hpp"

#include <limits>

struct AABB {
    double xmin{std::numeric_limits<double>::max()};
    double xmax{std::numeric_limits<double>::min()};
    double ymin{std::numeric_limits<double>::max()};
    double ymax{std::numeric_limits<double>::min()};

    AABB() = default;

    template <typename Container>
    AABB(const Container& container)
    {
        for(const auto [x, y] : container) {
            xmin = std::min(x, xmin);
            xmax = std::max(x, xmax);
            ymin = std::min(y, ymin);
            ymax = std::max(y, ymax);
        };
    }

    bool Inside(Point p) const { return p.x >= xmin && p.x < xmax && p.y >= ymin && p.y < ymax; }
};
