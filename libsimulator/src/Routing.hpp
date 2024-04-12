// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "DistanceMap.hpp"
#include "RoutingEngine.hpp"
#include "Stage.hpp"
#include "StageDescription.hpp"

class Routing
{
    using DMap = distance::DistanceMap<int, double>;
    using DMapBuilder = distance::DistanceMapBuilder<int, double>;
    using DPoly = distance::Polygon<double>;
    using DPoint = decltype(DPoly::points)::value_type;
    std::unique_ptr<CollisionGeometry> _geometry;
    std::unique_ptr<RoutingEngine> _routingEngine;
    std::unordered_map<BaseStage::ID, std::unique_ptr<const DMap>> _distanceMaps{};

public:
    explicit Routing(std::unique_ptr<CollisionGeometry> geometry);
    ~Routing() = default;
    Routing(const Routing& other) = delete;
    Routing& operator=(const Routing& other) = delete;
    Routing(Routing&& other) = default;
    Routing& operator=(Routing&& other) = default;
    void AddDistanceMapForStage(const BaseStage::ID id, const StageDescription stageDescription);

    const CollisionGeometry& Geometry() const { return *_geometry; }
    bool InsideGeometry(const Point& p) const { return _geometry->InsideGeometry(p); }
    Point ComputeWaypoint(Point currentPosition, BaseStage::Location destination) const;

private:
    std::unique_ptr<DMapBuilder> PrepareDistanceMapBuilder() const;
};
