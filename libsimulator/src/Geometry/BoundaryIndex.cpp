// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Geometry/BoundaryIndex.hpp"

#include "SimulationError.hpp"

//==================================================================================================
// NaiveBoundaryIndex
//==================================================================================================
NaiveBoundaryIndex::NaiveBoundaryIndex(std::vector<SegmentGrid<>> regions_)
    : regions(std::move(regions_))
{
}
std::vector<LineSegment> NaiveBoundaryIndex::Query(const Location& loc, double maximum_distance)
{
    auto range = regions[loc.region()].LineSegmentsInDistanceTo(maximum_distance, loc.xy());
    return {std::begin(range), std::end(range)};
}

//==================================================================================================
// Factories
//==================================================================================================
std::unique_ptr<BoundaryIndex>
MakeNaiveBoundaryIndex(const SurfaceMesh& mesh, const RegionSplit& region_split)
{
    auto boundaries = CreatePerRegionSegmentGrids(mesh, region_split);
    return std::make_unique<NaiveBoundaryIndex>(std::move(boundaries));
}

//==================================================================================================
// Supporting code
//==================================================================================================
std::tuple<SurfaceMesh::Face_index, SurfaceMesh::Halfedge_index>
IncidentFaceAndHalfedge(const SurfaceMesh& mesh, SurfaceMesh::Edge_index e)
{
    auto h = mesh.halfedge(e, 0);
    if(mesh.is_border(h)) {
        h = mesh.opposite(h);
    }
    if(mesh.is_border(h)) {
        throw SimulationError("Input mesh contains edges without incident faces.");
    }
    return {mesh.face(h), h};
}

std::vector<SegmentGrid<>>
CreatePerRegionSegmentGrids(const SurfaceMesh& mesh, const RegionSplit& region_split)
{
    std::vector<std::vector<LineSegment>> elements{};
    elements.resize(region_split.count);

    for(const auto edge : mesh.edges()) {
        if(mesh.is_border(edge)) {
            const auto [f, he] = IncidentFaceAndHalfedge(mesh, edge);
            const auto region_id = region_split.region[f];
            const auto v0 = mesh.point(mesh.source(he));
            const auto v1 = mesh.point(mesh.target(he));
            elements.at(region_id).emplace_back(Point(v0.x(), v0.y()), Point(v1.x(), v1.y()));
            continue;
        }
        // const auto he0 = _mesh.halfedge(edge, 0);
        // const auto he1 = _mesh.halfedge(edge, 1);
        // const auto he0_region = _region[_mesh.face(he0)];
        // const auto he1_region = _region[_mesh.face(he1)];
        // if(he0_region != he1_region) {
        //     const auto v0 = _mesh.point(_mesh.source(he0));
        //     const auto v1 = _mesh.point(_mesh.target(he0));
        //     elements.at(he0_region)
        //         .emplace_back(
        //             DirectedSeam{
        //                 .Segment = LineSegment(Point(v0.x(), v0.y()), Point(v1.x(), v1.y())),
        //                 .ConnectedRegion = he1_region});
        //     elements.at(he1_region)
        //         .emplace_back(
        //             DirectedSeam{
        //                 .Segment = LineSegment(Point(v1.x(), v1.y()), Point(v0.x(), v0.y())),
        //                 .ConnectedRegion = he0_region});
        // }
    }
    std::vector<SegmentGrid<>> boundaries{};
    boundaries.reserve(region_split.count);
    std::transform(
        std::begin(elements),
        std::end(elements),
        std::back_inserter(boundaries),
        [](const auto& e) { return SegmentGrid(e); });
    return boundaries;
}
