// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "DTriangulation.hpp"

#include <algorithm>

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
