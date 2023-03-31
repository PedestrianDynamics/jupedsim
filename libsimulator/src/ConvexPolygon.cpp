/// Copyright © 2012-2022 Forschungszentrum Jülich GmbH
/// SPDX-License-Identifier: LGPL-3.0-or-later
#include "ConvexPolygon.hpp"

#include "GeometricFunctions.hpp"

bool ConvexPolygon::IsInside(Point p) const
{
    size_t index_low = 0;
    size_t index_high = points.size();

    do {
        size_t index_mid = (index_low + index_high) / 2;
        if(triarea2d(points[0], points[index_mid], p) < 0.0) {
            index_low = index_mid;
        } else {
            index_high = index_mid;
        }
    } while(index_low + 1 < index_high);

    if(index_low == 0 || index_high == points.size()) {
        return false;
    }

    return triarea2d(points[index_low], points[index_high], p) < 0.0;
}
