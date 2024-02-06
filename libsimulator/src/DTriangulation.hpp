// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "Point.hpp"

#include <poly2tri/poly2tri.h>

#include <vector>

class DTriangulation
{
public:
    DTriangulation(
        const std::vector<Point>& outerBoundary,
        const std::vector<std::vector<Point>>& holes);
    ~DTriangulation();
    DTriangulation(const DTriangulation& other) = delete;
    DTriangulation& operator=(const DTriangulation& other) = delete;
    DTriangulation(DTriangulation&& other) = default;
    DTriangulation& operator=(DTriangulation&& other) = default;

    /// @return the triangles resulting from the triangulation
    std::vector<p2t::Triangle*> GetTriangles() { return _cdt->GetTriangles(); }

private:
    /// Poly2Tri does not take ownership of any p2t::Point* and requires the caller to keep them
    /// alive until p2t::CDT livetime ends
    std::vector<p2t::Point*> _outerConstraintsPolyline{};
    /// Poly2Tri does not take ownership of any p2t::Point* and requires the caller to keep them
    /// alive until p2t::CDT livetime ends
    std::vector<std::vector<p2t::Point*>> _holesPolylines{};
    std::unique_ptr<p2t::CDT> _cdt{};
};
