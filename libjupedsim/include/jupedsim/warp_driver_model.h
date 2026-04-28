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
 * Opaque type for a WarpDriverModel Builder
 */
typedef struct JPS_WarpDriverModelBuilder_t* JPS_WarpDriverModelBuilder;

/**
 * Creates a WarpDriverModel builder.
 * @param timeHorizon look-ahead time T [s] for collision prediction. Larger
 *        values detect collisions earlier but increase the per-step cost.
 * @param stepSize gradient-descent step size used to deviate from the
 *        projected trajectory. Larger values yield stronger avoidance.
 * @param sigma Gaussian spread of the intrinsic field. Larger values
 *        produce smoother and wider collision zones.
 * @param timeUncertainty time uncertainty parameter. Spreads the collision
 *        field along the time axis so that collisions further in the future
 *        contribute less.
 * @param velocityUncertaintyX longitudinal velocity uncertainty. Compresses
 *        the collision field along the direction of motion.
 * @param velocityUncertaintyY lateral velocity uncertainty. Expands the
 *        collision field perpendicular to the direction of motion.
 * */
JUPEDSIM_API JPS_WarpDriverModelBuilder
JPS_WarpDriverModelBuilder_Create(double timeHorizon,
        double stepSize,
        double sigma,
        double timeUncertainty,
        double velocityUncertaintyX,
        double velocityUncertaintyY);

/**
 * Creates a JPS_OperationalModel of type WarpDriverModel Model from the
 * JPS_WarpDriverModelBuilder.
 * @param handle the builder to operate on
 * @param[out] errorMessage if not NULL: will be set to a JPS_ErrorMessage in case of an error
 * @return a JPS_WarpDriverModel or NULL if an error occured.
 */
JUPEDSIM_API JPS_OperationalModel JPS_WarpDriverModelBuilder_Build(
    JPS_WarpDriverModelBuilder handle,
    JPS_ErrorMessage* errorMessage);

/**
 * Frees a JPS_WarpDriverModelBuilder
 * @param handle to the JPS_WarpDriverModelBuilder to free.
 */
JUPEDSIM_API void JPS_WarpDriverModelBuilder_Free(JPS_WarpDriverModelBuilder handle);

/**
 * Opaque type of Warp Driver model state
 */
typedef struct JPS_WarpDriverModelState_t* JPS_WarpDriverModelState;

/**
 * Read radius of this agent.
 * @param handle of the Agent to access.
 * @return radius in m of this agent
 */
JUPEDSIM_API double JPS_WarpDriverModelState_GetRadius(JPS_WarpDriverModelState handle);

/**
 * Write radius of this agent.
 * @param handle of the Agent to access.
 * @param radius in m of this agent.
 */
JUPEDSIM_API void
JPS_WarpDriverModelState_SetRadius(JPS_WarpDriverModelState handle, double radius);

/**
 * Read v0 of this agent.
 * @param handle of the Agent to access.
 * @return v0 of this agent.
 */
JUPEDSIM_API double JPS_WarpDriverModelState_GetV0(JPS_WarpDriverModelState handle);

/**
 * Write Velocity of this agent.
 * @param handle of the Agent to access.
 * @param v0 V0 of this agent.
 */
JUPEDSIM_API void
JPS_WarpDriverModelState_SetV0(JPS_WarpDriverModelState handle, double v0);

/**
 * Describes parameters of an Agent in WarpDriverModel
 */
typedef struct JPS_WarpDriverModelAgentParameters {
    /**
     * Position of the agent.
     * The position needs to inside the accessible area.
     */
    JPS_Point position{0, 0};
    /*
     * Orientation vector of the agent.
     * The orientation vector will internally be normalized.
     */
    JPS_Point orientation{0, 0};
    /**
     * Defines the journey this agent will take use
     */
    JPS_JourneyId journeyId = 0;
    /**
     * Defines the current stage of its journey
     */
    JPS_StageId stageId = 0;
    /**
     * radius of the agent
     */
    double radius = 0.15;
    /**
     * V0 of the agent
     */
    double v0 = 1.2;

} JPS_WarpDriverModelAgentParameters;

#ifdef __cplusplus
}
#endif
