/// Copyright © 2012-2022 Forschungszentrum Jülich GmbH
/// SPDX-License-Identifier: LGPL-3.0-or-later
#include "jupedsim/jupedsim_experimental.h"

#include "Conversion.hpp"
#include "jupedsim/jupedsim.h"

#include <Geometry.hpp>
#include <RoutingEngine.hpp>

////////////////////////////////////////////////////////////////////////////////
/// JPS_Path
////////////////////////////////////////////////////////////////////////////////
JUPEDSIM_API void JPS_Path_Free(JPS_Path* path)
{
    delete[] path->points;
    path->points = nullptr;
    path->len = 0;
}

////////////////////////////////////////////////////////////////////////////////
/// JPS_TriangleMesh
////////////////////////////////////////////////////////////////////////////////
JUPEDSIM_API void JPS_TriangleMesh_Free(JPS_TriangleMesh* mesh)
{
    delete[] mesh->triangles;
    mesh->triangles = nullptr;
    mesh->len = 0;
}

////////////////////////////////////////////////////////////////////////////////
/// JPS_Lines
////////////////////////////////////////////////////////////////////////////////
JUPEDSIM_API void JPS_Lines_Free(JPS_Lines* lines)
{
    delete[] lines->lines;
    lines->lines = nullptr;
    lines->len = 0;
}

////////////////////////////////////////////////////////////////////////////////
/// JPS_RoutingEngine
////////////////////////////////////////////////////////////////////////////////
JUPEDSIM_API JPS_RoutingEngine JPS_RoutingEngine_Create(JPS_Geometry geometry)
{
    auto* geo = reinterpret_cast<Geometry*>(geometry);
    return reinterpret_cast<JPS_RoutingEngine>(geo->routingEngine->Clone().release());
}

JUPEDSIM_API JPS_Path
JPS_RoutingEngine_ComputeWaypoint(JPS_RoutingEngine handle, JPS_Point from, JPS_Point to)
{
    auto* engine = reinterpret_cast<NavMeshRoutingEngine*>(handle);
    const auto path = engine->ComputeAllWaypoints(intoPoint(from), intoPoint(to));
    JPS_Path p{path.size(), new JPS_Point[path.size()]};
    std::transform(
        std::begin(path), std::end(path), p.points, [](const auto& p) { return intoJPS_Point(p); });
    return p;
}

JUPEDSIM_API bool JPS_RoutingEngine_IsRoutable(JPS_RoutingEngine handle, JPS_Point p)
{
    const auto* engine = reinterpret_cast<NavMeshRoutingEngine*>(handle);
    return engine->IsRoutable(intoPoint(p));
}

JUPEDSIM_API JPS_TriangleMesh JPS_RoutingEngine_Mesh(JPS_RoutingEngine handle)
{
    const auto* engine = reinterpret_cast<NavMeshRoutingEngine*>(handle);
    const auto res = engine->Mesh();
    JPS_TriangleMesh mesh{res.size(), new JPS_Triangle[res.size()]};
    std::transform(std::begin(res), std::end(res), mesh.triangles, [](const auto& t) {
        auto tri = JPS_Triangle{};
        tri.points[0] = intoJPS_Point(t.points[0]);
        tri.points[1] = intoJPS_Point(t.points[1]);
        tri.points[2] = intoJPS_Point(t.points[2]);
        return tri;
    });
    return mesh;
}

JUPEDSIM_API JPS_Lines JPS_RoutingEngine_EdgesFor(JPS_RoutingEngine handle, uint32_t id)
{
    const auto* engine = reinterpret_cast<NavMeshRoutingEngine*>(handle);
    const auto res = engine->EdgesFor(id);

    JPS_Lines lines{};
    lines.lines = new JPS_Line[res.size()];
    lines.len = res.size();
    std::transform(std::begin(res), std::end(res), lines.lines, [](const auto& edge) {
        JPS_Line line{};
        line.points[0] = intoJPS_Point(edge.edge.p1);
        line.points[1] = intoJPS_Point(edge.edge.p2);
        return line;
    });
    return lines;
}

JUPEDSIM_API void JPS_RoutingEngine_Free(JPS_RoutingEngine handle)
{
    delete reinterpret_cast<NavMeshRoutingEngine*>(handle);
}
