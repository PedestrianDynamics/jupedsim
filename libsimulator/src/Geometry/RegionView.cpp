// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Geometry/RegionView.hpp"

#include "GeometricFunctions.hpp"
#include "Geometry/Geometry.hpp"

#include <algorithm>
#include <utility>

RegionView::RegionView(
    std::size_t regionId,
    const Geometry* geometry3d,
    SegmentGrid walls,
    std::vector<RegionSeam> seams)
    : _regionId(regionId)
    , _geometry3d(geometry3d)
    , _walls(std::move(walls))
    , _seams(std::move(seams))
{
}

bool RegionView::InsideGeometry(Point p) const
{
    return _geometry3d->locate_in_region(_regionId, Point2D{p.x, p.y}).face !=
           SurfaceMesh::null_face();
}

bool RegionView::crosses_seam(Point a, Point b) const
{
    const LineSegment chord{a, b};
    return std::any_of(_seams.begin(), _seams.end(), [&chord](const RegionSeam& seam) {
        return intersects(chord, seam.segment);
    });
}
