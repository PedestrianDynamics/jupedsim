// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "CfgCgal.hpp"
#include "Geometry/Location.hpp"
#include "Geometry/RegionSplit.hpp"
#include "LineSegment.hpp"
#include "SegmentGrid.hpp"

#include <vector>

//==================================================================================================
// BoundaryIndex Interface
//==================================================================================================
/// BoundaryIndex Interface.
/// Spatial index over boundary line segments.
/// Note: This is designed as interface to develop differnet implementations. We may choose to
/// remove the interface once we decided on an implementation.
class BoundaryIndex
{
public:
    BoundaryIndex() = default;
    virtual ~BoundaryIndex() = default;
    BoundaryIndex(const BoundaryIndex&) = delete;
    BoundaryIndex(BoundaryIndex&&) = delete;
    void operator=(const BoundaryIndex&) = delete;
    void operator=(BoundaryIndex&&) = delete;

    virtual std::vector<LineSegment> Query(const Location& loc, double maximum_distance) = 0;
};

//==================================================================================================
// NaiveBoundaryIndex
//==================================================================================================
/// NaiveBoundaryIndex
/// Most naive implementation:
/// - Returns only linesegments from current region
/// - Does not remove occluded LineSegments
class NaiveBoundaryIndex : public BoundaryIndex
{
private:
    std::vector<SegmentGrid<>> regions;

public:
    explicit NaiveBoundaryIndex(std::vector<SegmentGrid<>> regions);
    std::vector<LineSegment> Query(const Location& loc, double maximum_distance) override;
};

//==================================================================================================
// Factories
//==================================================================================================
/// @precondition mesh has been split into region.
std::unique_ptr<BoundaryIndex>
MakeNaiveBoundaryIndex(const SurfaceMesh& mesh, const RegionSplit& region_split);

//==================================================================================================
// Supporting code
//==================================================================================================
std::tuple<SurfaceMesh::Face_index, SurfaceMesh::Halfedge_index>
IncidentFaceAndHalfedge(const SurfaceMesh& mesh, SurfaceMesh::Edge_index e);

std::vector<SegmentGrid<>>
CreatePerRegionSegmentGrids(const SurfaceMesh& mesh, const RegionSplit& region_split);
