/**
 * \file        DTriangulation.cpp
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
 *
 *
 **/
#include "DTriangulation.hpp"

static std::vector<p2t::Point*> ConvertPointVector(const std::vector<Point>& vec)
{
    std::vector<p2t::Point*> result{};
    result.resize(vec.size());
    std::transform(vec.begin(), vec.end(), result.begin(), [](const auto& p) {
        return new p2t::Point(p.x, p.y);
    });
    return result;
}

template <class C>
static void FreeCollection(C& collection)
{
    for(auto& item : collection) {
        delete item;
    }
    collection.clear();
}

DTriangulation::DTriangulation(
    const std::vector<Point>& outerBoundary,
    const std::vector<std::vector<Point>>& holes)
{
    _outerConstraintsPolyline = ConvertPointVector(outerBoundary);

    for(const auto& hole : holes) {
        _holesPolylines.push_back(ConvertPointVector(hole));
    }
    _cdt = std::make_unique<p2t::CDT>(_outerConstraintsPolyline);

    for(const auto& hole : _holesPolylines) {
        _cdt->AddHole(hole);
    }
    _cdt->Triangulate();
}

DTriangulation::~DTriangulation()
{
    for(auto& v : _holesPolylines) {
        FreeCollection(v);
    }
    FreeCollection(_outerConstraintsPolyline);
}
