/* Copyright © 2012-2022 Forschungszentrum Jülich GmbH */
/* SPDX-License-Identifier: LGPL-3.0-or-later */
#pragma once

#ifdef _WIN32
#ifdef JUPEDSIM_API_EXPORTS
#define JUPEDSIM_API __declspec(dllexport)
#else
#define JUPEDSIM_API __declspec(dllimport)
#endif
#else
#define JUPEDSIM_API
#endif
/**
 * WARNING!
 * THIS API IS EXPERIMENTAL AND MAY BE MODIFIED WITH OUT ANY WARNING!
 */

#include "jupedsim.h"

#include <stdbool.h> /*NOLINT(modernize-deprecated-headers)*/
#include <stddef.h> /*NOLINT(modernize-deprecated-headers)*/
#include <stdint.h> /*NOLINT(modernize-deprecated-headers)*/

#ifdef __cplusplus
extern "C" {
#endif

struct JPS_Path {
    size_t len;
    JPS_Point* points;
};

JUPEDSIM_API void JPS_Path_Free(JPS_Path* path);

struct JPS_Triangle {
    JPS_Point points[3];
};

struct JPS_TriangleMesh {
    size_t len;
    JPS_Triangle* triangles;
};

JUPEDSIM_API void JPS_TriangleMesh_Free(JPS_TriangleMesh* mesh);

struct JPS_Line {
    JPS_Point points[2];
};

struct JPS_Lines {
    size_t len;
    JPS_Line* lines;
};

JUPEDSIM_API void JPS_Lines_Free(JPS_Lines* lines);

/**
 * WARNING this is currently a NavMeshRoutingEngine! This does not account possible other types of
 * routing engines
 */
typedef struct JPS_RoutingEngine_t* JPS_RoutingEngine;

JUPEDSIM_API JPS_RoutingEngine JPS_RoutingEngine_Create(JPS_Geometry geometry);

JUPEDSIM_API JPS_Path
JPS_RoutingEngine_ComputeWaypoint(JPS_RoutingEngine handle, JPS_Point from, JPS_Point to);

JUPEDSIM_API bool JPS_RoutingEngine_IsRoutable(JPS_RoutingEngine handle, JPS_Point p);

JUPEDSIM_API JPS_TriangleMesh JPS_RoutingEngine_Mesh(JPS_RoutingEngine handle);

/// Note: Currently the ID is the index of the vertex from 'JPS_RoutingEngine_Mesh'
JUPEDSIM_API JPS_Lines JPS_RoutingEngine_EdgesFor(JPS_RoutingEngine handle, uint32_t id);

JUPEDSIM_API void JPS_RoutingEngine_Free(JPS_RoutingEngine handle);

#ifdef __cplusplus
}
#endif
