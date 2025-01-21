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
 * Opaque type for a GeneralizedCentrifugalForceModel Model Builder
 */
typedef struct JPS_GeneralizedCentrifugalForceModelBuilder_t*
    JPS_GeneralizedCentrifugalForceModelBuilder;

/**
 * Creates a GeneralizedCentrifugalForceModel model builder.
 * @param strengthNeighborRepulsion  Strength of the repulsion from neighbors
 * @param strengthGeometryRepulsion  Strength of the repulsion from geometry boundaries
 * @param maxNeighborInteractionDistance  cut-off-radius for ped-ped repulsion (r_c in FIG. 7)
 * @param maxGeometryInteractionDistance  cut-off-radius for ped-wall repulsion (r_c in FIG. 7)
 * @param maxNeighborInterpolationDistance  distance of interpolation of repulsive force for
 * ped-ped interaction (r_eps in FIG. 7)
 * @param maxGeometryInterpolationDistance  distance of interpolation of repulsive force for
 * ped-wall interaction (r_eps in FIG. 7)
 * @param maxNeighborRepulsionForce  maximum of the repulsion force for ped-ped interaction by
 * contact of ellipses (f_m in FIG. 7)
 * @param maxGeometryRepulsionForce  maximum of the repulsion force for ped-wall interaction by
 * contact of ellipses (f_m in FIG. 7)
 * @return the GeneralizedCentrifugalForceModel model builder
 */
JUPEDSIM_API JPS_GeneralizedCentrifugalForceModelBuilder
JPS_GeneralizedCentrifugalForceModelBuilder_Create(
    double strengthNeighborRepulsion,
    double strengthGeometryRepulsion,
    double maxNeighborInteractionDistance,
    double maxGeometryInteractionDistance,
    double maxNeighborInterpolationDistance,
    double maxGeometryInterpolationDistance,
    double maxNeighborRepulsionForce,
    double maxGeometryRepulsionForce);

/**
 * Creates a JPS_OperationalModel of type GeneralizedCentrifugalForceModel Model from the
 * JPS_GeneralizedCentrifugalForceModelBuilder.
 * @param handle the builder to operate on
 * @param[out] errorMessage if not NULL: will be set to a JPS_ErrorMessage in case of an error
 * @return a JPS_GeneralizedCentrifugalForceModel or NULL if an error occured.
 */
JUPEDSIM_API JPS_OperationalModel JPS_GeneralizedCentrifugalForceModelBuilder_Build(
    JPS_GeneralizedCentrifugalForceModelBuilder handle,
    JPS_ErrorMessage* errorMessage);

/**
 * Frees a JPS_GeneralizedCentrifugalForceModelBuilder
 * @param handle to the JPS_GeneralizedCentrifugalForceModelBuilder to free.
 */
JUPEDSIM_API void JPS_GeneralizedCentrifugalForceModelBuilder_Free(
    JPS_GeneralizedCentrifugalForceModelBuilder handle);

/**
 * Opaque type of Generalized Centrifugal Force model state
 */
typedef struct JPS_GeneralizedCentrifugalForceModelState_t*
    JPS_GeneralizedCentrifugalForceModelState;

/**
 * Read speed of this agent.
 * @param handle of the Agent to access.
 * @return speed in m/s
 */
JUPEDSIM_API double JPS_GeneralizedCentrifugalForceModelState_GetSpeed(
    JPS_GeneralizedCentrifugalForceModelState handle);

/**
 * Write speed of this agent.
 * @param handle of the Agent to access.
 * @param speed in m/s
 */
JUPEDSIM_API void JPS_GeneralizedCentrifugalForceModelState_SetSpeed(
    JPS_GeneralizedCentrifugalForceModelState handle,
    double speed);

/**
 * Read desired orientation of this agent.
 * @param handle of the Agent to access.
 * @return 2D orientation vector
 */
JUPEDSIM_API JPS_Point
JPS_GeneralizedCentrifugalForceModelState_GetE0(JPS_GeneralizedCentrifugalForceModelState handle);

/**
 * Write desired direction of this agent.
 * @param handle of the Agent to access.
 * @param e0 desired orientation of this agent.
 */
JUPEDSIM_API void JPS_GeneralizedCentrifugalForceModelState_SetE0(
    JPS_GeneralizedCentrifugalForceModelState handle,
    JPS_Point e0);

/**
 * Read mass in Kg of this agent.
 * @param handle of the Agent to access.
 * @return mass (Kg) of this agent
 */
JUPEDSIM_API double
JPS_GeneralizedCentrifugalForceModelState_GetMass(JPS_GeneralizedCentrifugalForceModelState handle);

/**
 * Write mass in Kg of this agent.
 * @param handle of the Agent to access.
 * @param mass (Kg) of this agent.
 */
JUPEDSIM_API void JPS_GeneralizedCentrifugalForceModelState_SetMass(
    JPS_GeneralizedCentrifugalForceModelState handle,
    double mass);

/**
 * Read tau of this agent.
 * @param handle of the Agent to access.
 * @return tau of this agent
 */
JUPEDSIM_API double
JPS_GeneralizedCentrifugalForceModelState_GetTau(JPS_GeneralizedCentrifugalForceModelState handle);

/**
 * Write tau of this agent.
 * @param handle of the Agent to access.
 * @param tau of this agent.
 */
JUPEDSIM_API void JPS_GeneralizedCentrifugalForceModelState_SetTau(
    JPS_GeneralizedCentrifugalForceModelState handle,
    double tau);

/**
 * Read v0 of this agent.
 * @param handle of the Agent to access.
 * @return v0 of this agent
 */
JUPEDSIM_API double
JPS_GeneralizedCentrifugalForceModelState_GetV0(JPS_GeneralizedCentrifugalForceModelState handle);

/**
 * Write v0 of this agent.
 * @param handle of the Agent to access.
 * @param v0 of this agent.
 */
JUPEDSIM_API void JPS_GeneralizedCentrifugalForceModelState_SetV0(
    JPS_GeneralizedCentrifugalForceModelState handle,
    double v0);

/**
 * Read a_v of this agent.
 * @param handle of the Agent to access.
 * @return AV of this agent
 */
JUPEDSIM_API double
JPS_GeneralizedCentrifugalForceModelState_GetAV(JPS_GeneralizedCentrifugalForceModelState handle);

/**
 * Write a_v of this agent.
 * @param handle of the Agent to access.
 * @param a_v of this agent.
 */
JUPEDSIM_API void JPS_GeneralizedCentrifugalForceModelState_SetAV(
    JPS_GeneralizedCentrifugalForceModelState handle,
    double a_v);

/**
 * Read a_min of this agent.
 * @param handle of the Agent to access.
 * @return a_min of this agent
 */
JUPEDSIM_API double
JPS_GeneralizedCentrifugalForceModelState_GetAMin(JPS_GeneralizedCentrifugalForceModelState handle);

/**
 * Write a_min of this agent.
 * @param handle of the Agent to access.
 * @param a_min of this agent.
 */
JUPEDSIM_API void JPS_GeneralizedCentrifugalForceModelState_SetAMin(
    JPS_GeneralizedCentrifugalForceModelState handle,
    double a_min);

/**
 * Read b_min of this agent.
 * @param handle of the Agent to access.
 * @return b_min of this agent
 */
JUPEDSIM_API double
JPS_GeneralizedCentrifugalForceModelState_GetBMin(JPS_GeneralizedCentrifugalForceModelState handle);

/**
 * Write b_min of this agent.
 * @param handle of the Agent to access.
 * @param b_min of this agent.
 */
JUPEDSIM_API void JPS_GeneralizedCentrifugalForceModelState_SetBMin(
    JPS_GeneralizedCentrifugalForceModelState handle,
    double b_min);

/**
 * Read b_max of this agent.
 * @param handle of the Agent to access.
 * @return b_max of this agent
 */
JUPEDSIM_API double
JPS_GeneralizedCentrifugalForceModelState_GetBMax(JPS_GeneralizedCentrifugalForceModelState handle);

/**
 * Write b_max of this agent.
 * @param handle of the Agent to access.
 * @param a_min of this agent.
 */
JUPEDSIM_API void JPS_GeneralizedCentrifugalForceModelState_SetBMax(
    JPS_GeneralizedCentrifugalForceModelState handle,
    double b_max);

/**
 * Describes parameters of an Agent in GeneralizedCentrifugalForceModel
 */
typedef struct JPS_GeneralizedCentrifugalForceModelAgentParameters {
    /**
     * Initial speed of the Agent
     */
    double speed = 0.0;
    /**
     * Desired orientation.
     */
    JPS_Point e0{0, 0};
    /**
     * Position of the agent.
     * The position needs to inside the accessible area.
     */
    JPS_Point position{0, 0};
    /*
     * Orientation vector of the agent.
     * The orientation vector will internally be normalized.
     */
    JPS_Point orientation{1, 0};
    /**
     * Defines the journey this agent will take use
     */
    JPS_JourneyId journeyId = 0;
    /**
     * Defines the current stage of its journey
     */
    JPS_StageId stageId = 0;
    /**
     * Mass of the agent
     */
    double mass = 1;
    /**
     * Tau of the agent
     */
    double tau = 0.5;
    /**
     * V0 of the agent
     */
    double v0 = 1.2;
    /**
     * a_v sagital axis stretch factor
     */
    double a_v = 1.;
    /**
     * b_min minimum length of transversal axis in 'meters'
     */
    double a_min = 0.2;
    /**
     * a_min minimum length of sagital axis in 'meters'
     */
    double b_min = 0.2;
    /**
     * b_max maximum length of transversal axis in 'meters'
     */
    double b_max = 0.4;
} JPS_GeneralizedCentrifugalForceModelAgentParameters;
#ifdef __cplusplus
}
#endif
