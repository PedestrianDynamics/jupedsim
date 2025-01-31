// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "error.h"
#include "export.h"
#include "operational_model.h"
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Opaque type for a Collision Free Speed Model Builder
 */
typedef struct JPS_CollisionFreeSpeedModelBuilder_t* JPS_CollisionFreeSpeedModelBuilder;

/**
 * Creates a Collision Free Speed Model builder.
 * @param strength_neighbor_repulsion describes the strength with which neighbors repulse each
 * other.
 * @param range_neighbor_repulsion describes the range at hich neighbors repulse each other.
 * @param strength_geometry_repulsion describes the strength with which neighbors are repules by
 * geometry.
 * @param range_geometry_repulsion describes the range at hich neighbors are repulsed by geometry.
 * @return the builder
 */
JUPEDSIM_API JPS_CollisionFreeSpeedModelBuilder JPS_CollisionFreeSpeedModelBuilder_Create(
    double strengthNeighborRepulsion,
    double rangeNeighborRepulsion,
    double strengthGeometryRepulsion,
    double rangeGeometryRepulsion);

/**
 * Creates a JPS_OperationalModel of type Collision Free Speed Model from the
 * JPS_GeneralizedCentrifugalForceModelBuilder.
 * @param handle the builder to operate on
 * @param[out] errorMessage if not NULL: will be set to a JPS_ErrorMessage in case of an error
 * @return a JPS_GeneralizedCentrifugalForceModel or NULL if an error occured.
 */
JUPEDSIM_API JPS_OperationalModel JPS_CollisionFreeSpeedModelBuilder_Build(
    JPS_CollisionFreeSpeedModelBuilder handle,
    JPS_ErrorMessage* errorMessage);

/**
 * Frees a JPS_CollisionFreeSpeedModelBuilder
 * @param handle to the JPS_CollisionFreeSpeedModelBuilder to free.
 */
JUPEDSIM_API void
JPS_CollisionFreeSpeedModelBuilder_Free(JPS_CollisionFreeSpeedModelBuilder handle);

/**
 * Opaque type of Collision Free Speed model state
 */
typedef struct JPS_CollisionFreeSpeedModelState_t* JPS_CollisionFreeSpeedModelState;

/**
 * Read e0 of this agent.
 * @param handle of the Agent to access.
 * @return e0 of this agent
 */
JUPEDSIM_API JPS_Point
JPS_CollisionFreeSpeedModelState_GetE0(JPS_CollisionFreeSpeedModelState handle);

/**
 * Write e0 of this agent.
 * @param handle of the Agent to access.
 * @param e0 of this agent.
 */
JUPEDSIM_API void
JPS_CollisionFreeSpeedModelState_SetE0(JPS_CollisionFreeSpeedModelState handle, JPS_Point e0);

/**
 * Read time gap of this agent.
 * @param handle of the Agent to access.
 * @return time gap of this agent
 */
JUPEDSIM_API double
JPS_CollisionFreeSpeedModelState_GetTimeGap(JPS_CollisionFreeSpeedModelState handle);

/**
 * Write time gap of this agent.
 * @param handle of the Agent to access.
 * @param time_gap of this agent.
 */
JUPEDSIM_API void JPS_CollisionFreeSpeedModelState_SetTimeGap(
    JPS_CollisionFreeSpeedModelState handle,
    double time_gap);

/**
 * Read tau of this agent.
 * @param handle of the Agent to access.
 * @return tau of this agent
 */
JUPEDSIM_API double
JPS_CollisionFreeSpeedModelState_GetTau(JPS_CollisionFreeSpeedModelState handle);

/**
 * Write tau of this agent.
 * @param handle of the Agent to access.
 * @param tau of this agent.
 */
JUPEDSIM_API void
JPS_CollisionFreeSpeedModelState_SetTau(JPS_CollisionFreeSpeedModelState handle, double tau);

/**
 * Read v0 of this agent.
 * @param handle of the Agent to access.
 * @return v0 of this agent
 */
JUPEDSIM_API double JPS_CollisionFreeSpeedModelState_GetV0(JPS_CollisionFreeSpeedModelState handle);

/**
 * Write v0 of this agent.
 * @param handle of the Agent to access.
 * @param v0 of this agent.
 */
JUPEDSIM_API void
JPS_CollisionFreeSpeedModelState_SetV0(JPS_CollisionFreeSpeedModelState handle, double v0);

/**
 * Read radius of this agent.
 * @param handle of the Agent to access.
 * @return radius of this agent
 */
JUPEDSIM_API double
JPS_CollisionFreeSpeedModelState_GetRadius(JPS_CollisionFreeSpeedModelState handle);

/**
 * Write radius of this agent in meters.
 * @param handle of the Agent to access.
 * @param radius (m) of this agent.
 */
JUPEDSIM_API void
JPS_CollisionFreeSpeedModelState_SetRadius(JPS_CollisionFreeSpeedModelState handle, double radius);
/**
 * Describes parameters of an Agent in GeneralizedCentrifugalForceModel
 */
typedef struct JPS_CollisionFreeSpeedModelAgentParameters {
    /**
     * Position of the agent.
     * The position needs to inside the accessible area.
     */
    JPS_Point position{0, 0};
    /**
     * Defines the journey this agent will take use
     */
    JPS_JourneyId journeyId = 0;
    /**
     * Defines the current stage of its journey
     */
    JPS_StageId stageId = 0;
    /**
     * @param time_gap of the agents using this profile (T in the OV-function)
     */
    double time_gap = 1.;
    /**
     *@param v0 of the agents using this profile(desired speed) double radius;
     */
    double v0 = 1.2;
    /**
     *@param radius of the agent in 'meters'
     */
    double radius = 0.2;
} JPS_CollisionFreeSpeedModelAgentParameters;

#ifdef __cplusplus
}
#endif
