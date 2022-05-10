/**
 * \file        DTriangulation.h
 * \date        Nov 30, 2012
 * \version     v0.7
 * \copyright   <2009-2015> Forschungszentrum Jülich GmbH. All rights reserved.
 *
 * \section License
 * This file is part of JuPedSim.
 *
 * JuPedSim is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * JuPedSim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with JuPedSim. If not, see <http://www.gnu.org/licenses/>.
 *
 * \section Description
 * Perform the Delauney triangulation of a polygon with holes.
 *
 *
 **/
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
