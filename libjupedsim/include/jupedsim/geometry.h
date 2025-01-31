// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "error.h"
#include "export.h"
#include "operational_model.h"
#include "types.h"

#include <stddef.h> /*NOLINT(modernize-deprecated-headers)*/

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Opaque type that represents the geometry the simulation acts on.
 * This type is created from JPS_GeometryBuilder.
 */
typedef struct JPS_Geometry_t const* JPS_Geometry;

/**
 * Returns the number of points that the outer boundary of the geometry consists of.
 * @param handle to the JPS_Geometry to operate on
 * @return Number of points in the outer boundary
 */
JUPEDSIM_API size_t JPS_Geometry_GetBoundarySize(JPS_Geometry handle);

/**
 * Returns the points the outer boundary consists of.
 * @param handle to the JPS_Geometry to operate on
 * @return the outer boundary. Do not free the returned pointer, it is still owned by JPS_Geometry.
 */
JUPEDSIM_API const JPS_Point* JPS_Geometry_GetBoundaryData(JPS_Geometry handle);

/**
 * Returns the number of holes in the geometry.
 * @param handle to the JPS_Geometry to operate on
 * @return Number of holes
 */
JUPEDSIM_API size_t JPS_Geometry_GetHoleCount(JPS_Geometry handle);

/**
 * Returns the number of points that hole consists of.
 * @param handle to the JPS_Geometry to operate on
 * @param index of the hole to access
 * @param[out] errorMessage if not NULL: will be set to a JPS_ErrorMessage in case of an error
 * @return Number of points in hole boundary or 0 on error.
 */
JUPEDSIM_API size_t
JPS_Geometry_GetHoleSize(JPS_Geometry handle, size_t hole_index, JPS_ErrorMessage* errorMessage);

/**
 * Returns the points the hole boundary consists of.
 * @param handle to the JPS_Geometry to operate on
 * @param index of the hole to access
 * @param[out] errorMessage if not NULL: will be set to a JPS_ErrorMessage in case of an error
 * @return points of the hole boundary or NULL on error.
 */
JUPEDSIM_API const JPS_Point*
JPS_Geometry_GetHoleData(JPS_Geometry handle, size_t hole_index, JPS_ErrorMessage* errorMessage);

/**
 * Frees a JPS_Geometry
 * @param handle to the JPS_Geometry to free.
 */
JUPEDSIM_API void JPS_Geometry_Free(JPS_Geometry geometry);

/**
 * Opaque type that is used to build geometry for the simulation.
 */
typedef struct JPS_GeometryBuilder_t* JPS_GeometryBuilder;

/**
 * Creates a new GeometryBuilder
 * @return the new builder.
 */
JUPEDSIM_API JPS_GeometryBuilder JPS_GeometryBuilder_Create();

/**
 * Adds an accessible area to the geometry.
 * The polygon described by points and pointCount must be a simple polygon. The polygon my be
 * specified in clockwise or counter-clockwise order.
 * Multiple accessible areas may overlap, the final accessible area is created by forming the union
 * over all accessible areas.
 * The Union over all accessible areas minus exclusions must form one polygon.
 * @param polygon describing the accessible area.
 * @param pointCount number of points the polygon consists of.
 */
JUPEDSIM_API void JPS_GeometryBuilder_AddAccessibleArea(
    JPS_GeometryBuilder handle,
    const JPS_Point* polygon,
    size_t lenPolygon);

/**
 * Adds an accessible area to the geometry.
 * The polygon described by points and pointCount must be a simple polygon. The polygon my be
 * specified in clockwise or counter-clockwise order.
 * Multiple exclusion areas may overlap, the final exclusion area is created by forming the union
 * over all exclusion areas.
 * Exclusion areas are subtracted from accessible areas.
 * The Union over all accessible areas minus exclusions must form a single polygon.
 * @param polygon describing the accessible area.
 * @param pointCount number of points the polygon consists of.
 */
JUPEDSIM_API void JPS_GeometryBuilder_ExcludeFromAccessibleArea(
    JPS_GeometryBuilder handle,
    const JPS_Point* polygon,
    size_t lenPolygon);

/**
 * Creates a JPS_Geometry from a JPS_GeometryBuilder. After this call the builder still has to be
 * freed with JPS_GeometryBuilder_Free.
 * @param handle to operate on.
 * @param[out] errorMessage if not NULL: will be set to a JPS_ErrorMessage in case of an error.
 * @return a JPS_Geometry handle on success or NULL on any error.
 */
JUPEDSIM_API JPS_Geometry
JPS_GeometryBuilder_Build(JPS_GeometryBuilder handle, JPS_ErrorMessage* errorMessage);

/**
 * Frees a JPS_GeometryBuilder.
 * @param handle to the JPS_GeometryBuilder to free.
 */
JUPEDSIM_API void JPS_GeometryBuilder_Free(JPS_GeometryBuilder handle);

#ifdef __cplusplus
}
#endif
