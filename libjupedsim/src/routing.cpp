// SPDX-License-Identifier: LGPL-3.0-or-later
#include "jupedsim/routing.h"

#include "Conversion.hpp"

#include <CollisionGeometry.hpp>
#include <RoutingEngine.hpp>

#include <algorithm>

using jupedsim::detail::intoJPS_Point;
using jupedsim::detail::intoPoint;
using jupedsim::detail::intoTuple;

////////////////////////////////////////////////////////////////////////////////
/// JPS_Path
////////////////////////////////////////////////////////////////////////////////
JUPEDSIM_API void JPS_Path_Free(JPS_Path path)
{
    delete[] path.points;
    path.points = nullptr;
    path.len = 0;
}

////////////////////////////////////////////////////////////////////////////////
/// JPS_Mesh
////////////////////////////////////////////////////////////////////////////////
JUPEDSIM_API void JPS_Mesh_Free(JPS_Mesh mesh)
{
    delete[] mesh.indices;
    delete[] mesh.polygons;
    delete[] mesh.vertices;
}

////////////////////////////////////////////////////////////////////////////////
/// JPS_RoutingEngine
////////////////////////////////////////////////////////////////////////////////
JUPEDSIM_API JPS_RoutingEngine JPS_RoutingEngine_Create(JPS_Geometry geometry)
{
    auto* geo = reinterpret_cast<const CollisionGeometry*>(geometry);
    return reinterpret_cast<JPS_RoutingEngine>(new RoutingEngine(geo->Polygon()));
}

JUPEDSIM_API JPS_Path
JPS_RoutingEngine_ComputeWaypoint(JPS_RoutingEngine handle, JPS_Point from, JPS_Point to)
{
    auto* engine = reinterpret_cast<RoutingEngine*>(handle);
    const auto path = engine->ComputeAllWaypoints(intoPoint(from), intoPoint(to));
    auto points = new JPS_Point[path.size()];
    JPS_Path p{path.size(), points};
    std::transform(
        std::begin(path), std::end(path), points, [](const auto& p) { return intoJPS_Point(p); });
    return p;
}

JUPEDSIM_API bool JPS_RoutingEngine_IsRoutable(JPS_RoutingEngine handle, JPS_Point p)
{
    const auto* engine = reinterpret_cast<RoutingEngine*>(handle);
    return engine->IsRoutable(intoPoint(p));
}

JUPEDSIM_API JPS_Mesh JPS_RoutingEngine_Mesh(JPS_RoutingEngine handle)
{
    const auto* engine = reinterpret_cast<RoutingEngine*>(handle);
    const auto mesh = engine->MeshData();

    JPS_Mesh result{};
    result.vertices_len = mesh->CountVertices();
    result.vertices = new JPS_Point[result.vertices_len];
    for(size_t index = 0; index < result.vertices_len; ++index) {
        const auto pt = mesh->Vertex(index);
        result.vertices[index] = JPS_Point{pt.x, pt.y};
    }
    result.polygons_len = mesh->CountPolygons();
    result.polygons = new JPS_Polygon_Desc[result.polygons_len];
    size_t offset = 0;
    for(size_t index = 0; index < result.polygons_len; ++index) {
        const auto& polygon = mesh->Polygons(index);
        const auto num_vertices = polygon.vertices.size();
        result.polygons[index] = JPS_Polygon_Desc{offset, num_vertices};
        offset += num_vertices;
    }
    result.indices = new uint16_t[offset];
    auto index_ptr = result.indices;
    for(size_t index = 0; index < result.polygons_len; ++index) {
        const auto& polygon = mesh->Polygons(index);
        for(const auto& v : polygon.vertices) {
            *index_ptr = v;
            ++index_ptr;
        }
    };

    return result;
}

JUPEDSIM_API void JPS_RoutingEngine_Free(JPS_RoutingEngine handle)
{
    delete reinterpret_cast<RoutingEngine*>(handle);
}
