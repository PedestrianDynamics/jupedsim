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
 * Opaque type for a Collision Free Speed Model V2 Builder
 */
typedef struct JPS_CollisionFreeSpeedModelV3Builder_t* JPS_CollisionFreeSpeedModelV3Builder;

/**
 * Creates a Collision Free Speed Model V2 builder.
 * @return the builder
 */
JUPEDSIM_API JPS_CollisionFreeSpeedModelV3Builder JPS_CollisionFreeSpeedModelV3Builder_Create();

/**
 * Creates a JPS_OperationalModel of type Collision Free Speed Model V2 from the
 * JPS_CollisionFreeSpeedModelV3Builder.
 * @param handle the builder to operate on
 * @param[out] errorMessage if not NULL: will be set to a JPS_ErrorMessage in case of an error
 * @return a JPS_CollisionFreeSpeedModelV3 or NULL if an error occured.
 */
JUPEDSIM_API JPS_OperationalModel JPS_CollisionFreeSpeedModelV3Builder_Build(
    JPS_CollisionFreeSpeedModelV3Builder handle,
    JPS_ErrorMessage* errorMessage);

/**
 * Frees a JPS_CollisionFreeSpeedModelV3Builder
 * @param handle to the JPS_CollisionFreeSpeedModelV3Builder to free.
 */
JUPEDSIM_API void
JPS_CollisionFreeSpeedModelV3Builder_Free(JPS_CollisionFreeSpeedModelV3Builder handle);

/**
 * Opaque type of Collision Free Speed V2 model state
 */
typedef struct JPS_CollisionFreeSpeedModelV3State_t* JPS_CollisionFreeSpeedModelV3State;

/**
 * Read strength neighbor repulsion of this agent.
 * @param handle of the Agent to access.
 * @return strength neighbor repulsion of this agent
 */
JUPEDSIM_API double JPS_CollisionFreeSpeedModelV3State_GetStrengthNeighborRepulsion(
    JPS_CollisionFreeSpeedModelV3State handle);

/**
 * Write strength neighbor repulsion of this agent.
 * @param handle of the Agent to access.
 * @param strengthNeighborRepulsion of this agent.
 */
JUPEDSIM_API void JPS_CollisionFreeSpeedModelV3State_SetStrengthNeighborRepulsion(
    JPS_CollisionFreeSpeedModelV3State handle,
    double strengthNeighborRepulsion);

/**
 * Read range neighbor repulsion of this agent.
 * @param handle of the Agent to access.
 * @return range neighbor repulsion of this agent
 */
JUPEDSIM_API double JPS_CollisionFreeSpeedModelV3State_GetRangeNeighborRepulsion(
    JPS_CollisionFreeSpeedModelV3State handle);

/**
 * Write range neighbor repulsion of this agent.
 * @param handle of the Agent to access.
 * @param rangeNeighborRepulsion of this agent.
 */
JUPEDSIM_API void JPS_CollisionFreeSpeedModelV3State_SetRangeNeighborRepulsion(
    JPS_CollisionFreeSpeedModelV3State handle,
    double rangeNeighborRepulsion);

/**
 * Read strength geometry repulsion of this agent.
 * @param handle of the Agent to access.
 * @return strength geometry repulsion of this agent
 */
JUPEDSIM_API double JPS_CollisionFreeSpeedModelV3State_GetStrengthGeometryRepulsion(
    JPS_CollisionFreeSpeedModelV3State handle);

/**
 * Write strength geometry repulsion of this agent.
 * @param handle of the Agent to access.
 * @param strengthGeometryRepulsion of this agent.
 */
JUPEDSIM_API void JPS_CollisionFreeSpeedModelV3State_SetStrengthGeometryRepulsion(
    JPS_CollisionFreeSpeedModelV3State handle,
    double strengthGeometryRepulsion);

/**
 * Read range geometry repulsion of this agent.
 * @param handle of the Agent to access.
 * @return range geometry repulsion of this agent
 */
JUPEDSIM_API double JPS_CollisionFreeSpeedModelV3State_GetRangeGeometryRepulsion(
    JPS_CollisionFreeSpeedModelV3State handle);

/**
 * Write strength neighbor repulsion of this agent.
 * @param handle of the Agent to access.
 * @param rangeGeometryRepulsion of this agent.
 */
JUPEDSIM_API void JPS_CollisionFreeSpeedModelV3State_SetRangeGeometryRepulsion(
    JPS_CollisionFreeSpeedModelV3State handle,
    double rangeGeometryRepulsion);

/**
 * Read rangeXScale of this agent.
 * @param handle of the Agent to access.
 * @return rangeXScale of this agent
 */
JUPEDSIM_API double
JPS_CollisionFreeSpeedModelV3State_GetRangeXScale(JPS_CollisionFreeSpeedModelV3State handle);

/**
 * Write rangeXScale of this agent.
 * @param handle of the Agent to access.
 * @param rangeXScale of this agent.
 */
JUPEDSIM_API void
JPS_CollisionFreeSpeedModelV3State_SetRangeXScale(JPS_CollisionFreeSpeedModelV3State handle, double rangeXScale);

/**
 * Read rangeYScale of this agent.
 * @param handle of the Agent to access.
 * @return rangeYScale of this agent
 */
JUPEDSIM_API double
JPS_CollisionFreeSpeedModelV3State_GetRangeYScale(JPS_CollisionFreeSpeedModelV3State handle);

/**
 * Write rangeYScale of this agent.
 * @param handle of the Agent to access.
 * @param rangeYScale of this agent.
 */
JUPEDSIM_API void
JPS_CollisionFreeSpeedModelV3State_SetRangeYScale(JPS_CollisionFreeSpeedModelV3State handle, double rangeYScale);

/**
 * Read thetaMaxUpperBound of this agent.
 * @param handle of the Agent to access.
 * @return thetaMaxUpperBound of this agent
 */
JUPEDSIM_API double
JPS_CollisionFreeSpeedModelV3State_GetThetaMaxUpperBound(JPS_CollisionFreeSpeedModelV3State handle);

/**
 * Write thetaMaxUpperBound of this agent.
 * @param handle of the Agent to access.
 * @param thetaMaxUpperBound of this agent.
 */
JUPEDSIM_API void
JPS_CollisionFreeSpeedModelV3State_SetThetaMaxUpperBound(JPS_CollisionFreeSpeedModelV3State handle, double thetaMaxUpperBound);

/**
 * Read agentBuffer of this agent.
 * @param handle of the Agent to access.
 * @return agentBuffer of this agent
 */
JUPEDSIM_API double
JPS_CollisionFreeSpeedModelV3State_GetAgentBuffer(JPS_CollisionFreeSpeedModelV3State handle);

/**
 * Write agentBuffer of this agent.
 * @param handle of the Agent to access.
 * @param agentBuffer of this agent.
 */
JUPEDSIM_API void
JPS_CollisionFreeSpeedModelV3State_SetAgentBuffer(JPS_CollisionFreeSpeedModelV3State handle, double agentBuffer);

/**
 * Read time gap of this agent.
 * @param handle of the Agent to access.
 * @return time gap of this agent
 */
JUPEDSIM_API double
JPS_CollisionFreeSpeedModelV3State_GetTimeGap(JPS_CollisionFreeSpeedModelV3State handle);

/**
 * Write time gap of this agent.
 * @param handle of the Agent to access.
 * @param time_gap of this agent.
 */
JUPEDSIM_API void JPS_CollisionFreeSpeedModelV3State_SetTimeGap(
    JPS_CollisionFreeSpeedModelV3State handle,
    double time_gap);

/**
 * Read v0 of this agent.
 * @param handle of the Agent to access.
 * @return v0 of this agent
 */
JUPEDSIM_API double
JPS_CollisionFreeSpeedModelV3State_GetV0(JPS_CollisionFreeSpeedModelV3State handle);

/**
 * Write v0 of this agent.
 * @param handle of the Agent to access.
 * @param v0 of this agent.
 */
JUPEDSIM_API void
JPS_CollisionFreeSpeedModelV3State_SetV0(JPS_CollisionFreeSpeedModelV3State handle, double v0);

/**
 * Read radius of this agent.
 * @param handle of the Agent to access.
 * @return radius of this agent
 */
JUPEDSIM_API double
JPS_CollisionFreeSpeedModelV3State_GetRadius(JPS_CollisionFreeSpeedModelV3State handle);

/**
 * Write radius of this agent in meters.
 * @param handle of the Agent to access.
 * @param radius (m) of this agent.
 */
JUPEDSIM_API void JPS_CollisionFreeSpeedModelV3State_SetRadius(
    JPS_CollisionFreeSpeedModelV3State handle,
    double radius);

/**
 * Describes parameters of an Agent in Collision Free Speed Model V2
 */
typedef struct JPS_CollisionFreeSpeedModelV3AgentParameters {
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
     *  Strength of the repulsion from neighbors
     */
    double strengthNeighborRepulsion{8.0};

    /**
     * Range of the repulsion from neighbors
     */
    double rangeNeighborRepulsion{0.1};

    /**
     * Strength of the repulsion from geometry boundaries
     */
    double strengthGeometryRepulsion{5.0};

    /**
     * Range of the repulsion from geometry boundaries
     */
    double rangeGeometryRepulsion{0.02};

    /**
     * @param rangeXScale forward interaction stretch multiplier
     */
    double rangeXScale = 20.;
    /**
     *@param rangeYScale lateral interaction stretch multiplier
     */
    double rangeYScale = 8.;
    /**
     *@param thetaMaxUpperBound [rad] hard cap on turn angle per update
     */
    double thetaMaxUpperBound = 1.57;
    /**
     * @param agentBuffer [m] stand-off used in speed law: v=0 at s<=buffer
     */
    double agentBuffer = 0.;

    /**
     * @param time_gap of the agents using this profile (T in the OV-function)
     */
    double time_gap = 1.;
    /**
     *@param v0 of the agents using this profile(desired speed) double radius
     */
    double v0 = 1.2;
    /**
     *@param radius of the agent in 'meters'
     */
    double radius = 0.2;

} JPS_CollisionFreeSpeedModelV3AgentParameters;

#ifdef __cplusplus
}
#endif
