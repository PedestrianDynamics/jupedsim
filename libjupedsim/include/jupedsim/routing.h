/* Copyright © 2012-2024 Forschungszentrum Jülich GmbH */
/* SPDX-License-Identifier: LGPL-3.0-or-later */
#pragma once

#include "export.h"
#include "geometry.h"
#include "types.h"

#include <stddef.h> /*NOLINT(modernize-deprecated-headers)*/

#ifdef __cplusplus
extern "C" {
#endif

/**
 * A path inside the walkable area.
 */
typedef struct JPS_Path {
    /**
     * Number of points in this path
     */
    size_t len;
    /**
     * JPS_Points in this path.
     */
    const JPS_Point* points;
} JPS_Path;

/**
 * Frees memory held by JPS_Path
 *
 * @param path to free its memory.
 */
JUPEDSIM_API void JPS_Path_Free(JPS_Path path);

/**
 * Describes a polygon in terms of indices of vertices belonging to a JPS_Mesh.
 */
typedef struct JPS_Polygon_Desc {
    /**
     * Offset point to the beginning of this polygons indices in the JPS_Mesh indices array.
     */
    size_t offset;
    /**
     * Number of vertices in this polygon
     */
    size_t len;
} JPS_Polygon_Desc;

/**
 * Describes a polygonal mesh.
 * Each polygon in the mesh is defined in CCW order.
 * Each vertex is stored once and referred by index in each polygon.
 */
typedef struct JPS_Mesh {
    /**
     * Number of vertices in this mesh
     */
    size_t vertices_len;
    /**
     * The vertex data
     */
    JPS_Point* vertices;
    /**
     * Number of polygons in this mesh
     */
    size_t polygons_len;
    /**
     * Definitions of all polygons in this mesh
     */
    JPS_Polygon_Desc* polygons;
    /**
     * Array of all vertex indices used by the polygons.
     * Length of this array is sum of all polygon indices.
     * (Sum of all JPS_Polygon_Desc.len)
     */
    uint16_t* indices;
} JPS_Mesh;

/**
 * Frees the memory held by JPS_Mesh
 */
JUPEDSIM_API void JPS_Mesh_Free(JPS_Mesh mesh);

/**
 * WARNING this is currently a NavMeshRoutingEngine! This does not account possible other types of
 * routing engines
 */
typedef struct JPS_RoutingEngine_t* JPS_RoutingEngine;

JUPEDSIM_API JPS_RoutingEngine JPS_RoutingEngine_Create(JPS_Geometry geometry);

JUPEDSIM_API JPS_Path
JPS_RoutingEngine_ComputeWaypoint(JPS_RoutingEngine handle, JPS_Point from, JPS_Point to);

JUPEDSIM_API bool JPS_RoutingEngine_IsRoutable(JPS_RoutingEngine handle, JPS_Point p);

JUPEDSIM_API JPS_Mesh JPS_RoutingEngine_Mesh(JPS_RoutingEngine handle);

JUPEDSIM_API void JPS_RoutingEngine_Free(JPS_RoutingEngine handle);

#ifdef __cplusplus
}
#endif
