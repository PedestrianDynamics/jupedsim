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
 * Opaque type for a Anticipation Velocity Model Builder
 */
typedef struct JPS_AnticipationVelocityModelBuilder_t* JPS_AnticipationVelocityModelBuilder;

/**
 * Creates a Anticipation Velocity Model builder.
 * @return the builder
 */
JUPEDSIM_API JPS_AnticipationVelocityModelBuilder JPS_AnticipationVelocityModelBuilder_Create();

/**
 * Creates a JPS_OperationalModel of type Anticipation Velocity Model from the
 * JPS_AnticipationVelocityModelBuilder.
 * @param handle the builder to operate on
 * @param[out] errorMessage if not NULL: will be set to a JPS_ErrorMessage in case of an error
 * @return a JPS_AnticipationVelocityModel or NULL if an error occured.
 */
JUPEDSIM_API JPS_OperationalModel JPS_AnticipationVelocityModelBuilder_Build(
    JPS_AnticipationVelocityModelBuilder handle,
    JPS_ErrorMessage* errorMessage);

/**
 * Frees a JPS_AnticipationVelocityModelBuilder
 * @param handle to the JPS_AnticipationVelocityModelBuilder to free.
 */
JUPEDSIM_API void
JPS_AnticipationVelocityModelBuilder_Free(JPS_AnticipationVelocityModelBuilder handle);

/**
 * Opaque type of Anticipation Velocity Model state
 */
typedef struct JPS_AnticipationVelocityModelState_t* JPS_AnticipationVelocityModelState;

/**
 * Read strength neighbor repulsion of this agent.
 * @param handle of the Agent to access.
 * @return strength neighbor repulsion of this agent
 */
JUPEDSIM_API double JPS_AnticipationVelocityModelState_GetStrengthNeighborRepulsion(
    JPS_AnticipationVelocityModelState handle);

/**
 * Write strength neighbor repulsion of this agent.
 * @param handle of the Agent to access.
 * @param strengthNeighborRepulsion of this agent.
 */
JUPEDSIM_API void JPS_AnticipationVelocityModelState_SetStrengthNeighborRepulsion(
    JPS_AnticipationVelocityModelState handle,
    double strengthNeighborRepulsion);

/**
 * Read range neighbor repulsion of this agent.
 * @param handle of the Agent to access.
 * @return range neighbor repulsion of this agent
 */
JUPEDSIM_API double JPS_AnticipationVelocityModelState_GetRangeNeighborRepulsion(
    JPS_AnticipationVelocityModelState handle);

/**
 * Write range neighbor repulsion of this agent.
 * @param handle of the Agent to access.
 * @param rangeNeighborRepulsion of this agent.
 */
JUPEDSIM_API void JPS_AnticipationVelocityModelState_SetRangeNeighborRepulsion(
    JPS_AnticipationVelocityModelState handle,
    double rangeNeighborRepulsion);

/**
 * Read strength geometry repulsion of this agent.
 * @param handle of the Agent to access.
 * @return strength geometry repulsion of this agent
 */
JUPEDSIM_API double JPS_AnticipationVelocityModelState_GetStrengthGeometryRepulsion(
    JPS_AnticipationVelocityModelState handle);

/**
 * Write strength geometry repulsion of this agent.
 * @param handle of the Agent to access.
 * @param strengthGeometryRepulsion of this agent.
 */
JUPEDSIM_API void JPS_AnticipationVelocityModelState_SetStrengthGeometryRepulsion(
    JPS_AnticipationVelocityModelState handle,
    double strengthGeometryRepulsion);

/**
 * Read range geometry repulsion of this agent.
 * @param handle of the Agent to access.
 * @return range geometry repulsion of this agent
 */
JUPEDSIM_API double JPS_AnticipationVelocityModelState_GetRangeGeometryRepulsion(
    JPS_AnticipationVelocityModelState handle);

/**
 * Write strength neighbor repulsion of this agent.
 * @param handle of the Agent to access.
 * @param rangeGeometryRepulsion of this agent.
 */
JUPEDSIM_API void JPS_AnticipationVelocityModelState_SetRangeGeometryRepulsion(
    JPS_AnticipationVelocityModelState handle,
    double rangeGeometryRepulsion);

/**
 * Read e0 of this agent.
 * @param handle of the Agent to access.
 * @return e0 of this agent
 */
JUPEDSIM_API JPS_Point
JPS_AnticipationVelocityModelState_GetE0(JPS_AnticipationVelocityModelState handle);

/**
 * Write e0 of this agent.
 * @param handle of the Agent to access.
 * @param e0 of this agent.
 */
JUPEDSIM_API void
JPS_AnticipationVelocityModelState_SetE0(JPS_AnticipationVelocityModelState handle, JPS_Point e0);

/**
 * Read time gap of this agent.
 * @param handle of the Agent to access.
 * @return time gap of this agent
 */
JUPEDSIM_API double
JPS_AnticipationVelocityModelState_GetTimeGap(JPS_AnticipationVelocityModelState handle);

/**
 * Write time gap of this agent.
 * @param handle of the Agent to access.
 * @param time_gap of this agent.
 */
JUPEDSIM_API void JPS_AnticipationVelocityModelState_SetTimeGap(
    JPS_AnticipationVelocityModelState handle,
    double time_gap);

/**
 * Read tau of this agent.
 * @param handle of the Agent to access.
 * @return tau of this agent
 */
JUPEDSIM_API double
JPS_AnticipationVelocityModelState_GetTau(JPS_AnticipationVelocityModelState handle);

/**
 * Write tau of this agent.
 * @param handle of the Agent to access.
 * @param tau of this agent.
 */
JUPEDSIM_API void
JPS_AnticipationVelocityModelState_SetTau(JPS_AnticipationVelocityModelState handle, double tau);

/**
 * Read v0 of this agent.
 * @param handle of the Agent to access.
 * @return v0 of this agent
 */
JUPEDSIM_API double
JPS_AnticipationVelocityModelState_GetV0(JPS_AnticipationVelocityModelState handle);

/**
 * Write v0 of this agent.
 * @param handle of the Agent to access.
 * @param v0 of this agent.
 */
JUPEDSIM_API void
JPS_AnticipationVelocityModelState_SetV0(JPS_AnticipationVelocityModelState handle, double v0);

/**
 * Read radius of this agent.
 * @param handle of the Agent to access.
 * @return radius of this agent
 */
JUPEDSIM_API double
JPS_AnticipationVelocityModelState_GetRadius(JPS_AnticipationVelocityModelState handle);

/**
 * Write radius of this agent in meters.
 * @param handle of the Agent to access.
 * @param radius (m) of this agent.
 */
JUPEDSIM_API void JPS_AnticipationVelocityModelState_SetRadius(
    JPS_AnticipationVelocityModelState handle,
    double radius);

/**
 * Describes parameters of an Agent in Anticipation Velocity Model
 */
typedef struct JPS_AnticipationVelocityModelAgentParameters {
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

} JPS_AnticipationVelocityModelAgentParameters;

#ifdef __cplusplus
}
#endif
