/* Copyright © 2012-2024 Forschungszentrum Jülich GmbH */
/* SPDX-License-Identifier: LGPL-3.0-or-later */
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
typedef struct JPS_CollisionFreeSpeedModelIndividualBuilder_t*
    JPS_CollisionFreeSpeedModelIndividualBuilder;

/**
 * Creates a Collision Free Speed Model Individual builder.
 * @return the builder
 */
JUPEDSIM_API JPS_CollisionFreeSpeedModelIndividualBuilder
JPS_CollisionFreeSpeedModelIndividualBuilder_Create();

/**
 * Creates a JPS_OperationalModel of type Collision Free Speed Model Individual from the
 * JPS_CollisionFreeSpeedModelIndividualBuilder.
 * @param handle the builder to operate on
 * @param[out] errorMessage if not NULL: will be set to a JPS_ErrorMessage in case of an error
 * @return a JPS_CollisionFreeSpeedModelIndividual or NULL if an error occured.
 */
JUPEDSIM_API JPS_OperationalModel JPS_CollisionFreeSpeedModelIndividualBuilder_Build(
    JPS_CollisionFreeSpeedModelIndividualBuilder handle,
    JPS_ErrorMessage* errorMessage);

/**
 * Frees a JPS_CollisionFreeSpeedModelIndividualBuilder
 * @param handle to the JPS_CollisionFreeSpeedModelIndividualBuilder to free.
 */
JUPEDSIM_API void JPS_CollisionFreeSpeedModelIndividualBuilder_Free(
    JPS_CollisionFreeSpeedModelIndividualBuilder handle);

/**
 * Opaque type of Collision Free Speed model state
 */
typedef struct JPS_CollisionFreeSpeedModelIndividualState_t*
    JPS_CollisionFreeSpeedModelIndividualState;

/**
 * Read strength neighbor repulsion of this agent.
 * @param handle of the Agent to access.
 * @return strength neighbor repulsion of this agent
 */
JUPEDSIM_API double JPS_CollisionFreeSpeedModelIndividualState_GetStrengthNeighborRepulsion(
    JPS_CollisionFreeSpeedModelIndividualState handle);

/**
 * Write strength neighbor repulsion of this agent.
 * @param handle of the Agent to access.
 * @param strengthNeighborRepulsion of this agent.
 */
JUPEDSIM_API void JPS_CollisionFreeSpeedModelIndividualState_SetStrengthNeighborRepulsion(
    JPS_CollisionFreeSpeedModelIndividualState handle,
    double strengthNeighborRepulsion);

/**
 * Read range neighbor repulsion of this agent.
 * @param handle of the Agent to access.
 * @return range neighbor repulsion of this agent
 */
JUPEDSIM_API double JPS_CollisionFreeSpeedModelIndividualState_GetRangeNeighborRepulsion(
    JPS_CollisionFreeSpeedModelIndividualState handle);

/**
 * Write range neighbor repulsion of this agent.
 * @param handle of the Agent to access.
 * @param rangeNeighborRepulsion of this agent.
 */
JUPEDSIM_API void JPS_CollisionFreeSpeedModelIndividualState_SetRangeNeighborRepulsion(
    JPS_CollisionFreeSpeedModelIndividualState handle,
    double rangeNeighborRepulsion);

/**
 * Read strength geometry repulsion of this agent.
 * @param handle of the Agent to access.
 * @return strength geometry repulsion of this agent
 */
JUPEDSIM_API double JPS_CollisionFreeSpeedModelIndividualState_GetStrengthGeometryRepulsion(
    JPS_CollisionFreeSpeedModelIndividualState handle);

/**
 * Write strength geometry repulsion of this agent.
 * @param handle of the Agent to access.
 * @param strengthGeometryRepulsion of this agent.
 */
JUPEDSIM_API void JPS_CollisionFreeSpeedModelIndividualState_SetStrengthGeometryRepulsion(
    JPS_CollisionFreeSpeedModelIndividualState handle,
    double strengthGeometryRepulsion);

/**
 * Read range geometry repulsion of this agent.
 * @param handle of the Agent to access.
 * @return range geometry repulsion of this agent
 */
JUPEDSIM_API double JPS_CollisionFreeSpeedModelIndividualState_GetRangeGeometryRepulsion(
    JPS_CollisionFreeSpeedModelIndividualState handle);

/**
 * Write strength neighbor repulsion of this agent.
 * @param handle of the Agent to access.
 * @param rangeGeometryRepulsion of this agent.
 */
JUPEDSIM_API void JPS_CollisionFreeSpeedModelIndividualState_SetRangeGeometryRepulsion(
    JPS_CollisionFreeSpeedModelIndividualState handle,
    double rangeGeometryRepulsion);

/**
 * Read e0 of this agent.
 * @param handle of the Agent to access.
 * @return e0 of this agent
 */
JUPEDSIM_API JPS_Point
JPS_CollisionFreeSpeedModelIndividualState_GetE0(JPS_CollisionFreeSpeedModelIndividualState handle);

/**
 * Write e0 of this agent.
 * @param handle of the Agent to access.
 * @param e0 of this agent.
 */
JUPEDSIM_API void JPS_CollisionFreeSpeedModelIndividualState_SetE0(
    JPS_CollisionFreeSpeedModelIndividualState handle,
    JPS_Point e0);

/**
 * Read time gap of this agent.
 * @param handle of the Agent to access.
 * @return time gap of this agent
 */
JUPEDSIM_API double JPS_CollisionFreeSpeedModelIndividualState_GetTimeGap(
    JPS_CollisionFreeSpeedModelIndividualState handle);

/**
 * Write time gap of this agent.
 * @param handle of the Agent to access.
 * @param time_gap of this agent.
 */
JUPEDSIM_API void JPS_CollisionFreeSpeedModelIndividualState_SetTimeGap(
    JPS_CollisionFreeSpeedModelIndividualState handle,
    double time_gap);

/**
 * Read tau of this agent.
 * @param handle of the Agent to access.
 * @return tau of this agent
 */
JUPEDSIM_API double JPS_CollisionFreeSpeedModelIndividualState_GetTau(
    JPS_CollisionFreeSpeedModelIndividualState handle);

/**
 * Write tau of this agent.
 * @param handle of the Agent to access.
 * @param tau of this agent.
 */
JUPEDSIM_API void JPS_CollisionFreeSpeedModelIndividualState_SetTau(
    JPS_CollisionFreeSpeedModelIndividualState handle,
    double tau);

/**
 * Read v0 of this agent.
 * @param handle of the Agent to access.
 * @return v0 of this agent
 */
JUPEDSIM_API double
JPS_CollisionFreeSpeedModelIndividualState_GetV0(JPS_CollisionFreeSpeedModelIndividualState handle);

/**
 * Write v0 of this agent.
 * @param handle of the Agent to access.
 * @param v0 of this agent.
 */
JUPEDSIM_API void JPS_CollisionFreeSpeedModelIndividualState_SetV0(
    JPS_CollisionFreeSpeedModelIndividualState handle,
    double v0);

/**
 * Read radius of this agent.
 * @param handle of the Agent to access.
 * @return radius of this agent
 */
JUPEDSIM_API double JPS_CollisionFreeSpeedModelIndividualState_GetRadius(
    JPS_CollisionFreeSpeedModelIndividualState handle);

/**
 * Write radius of this agent in meters.
 * @param handle of the Agent to access.
 * @param radius (m) of this agent.
 */
JUPEDSIM_API void JPS_CollisionFreeSpeedModelIndividualState_SetRadius(
    JPS_CollisionFreeSpeedModelIndividualState handle,
    double radius);

/**
 * Describes parameters of an Agent in GeneralizedCentrifugalForceModel
 */
typedef struct JPS_CollisionFreeSpeedModelIndividualAgentParameters {
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

} JPS_CollisionFreeSpeedModelIndividualAgentParameters;

#ifdef __cplusplus
}
#endif
