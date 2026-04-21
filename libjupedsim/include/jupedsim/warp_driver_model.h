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
 * @param timeHorizon [Mohcine]...
 * @param stepSize [Mohcine]...
 * @param sigma [Mohcine]...
 * @param timeUncertainty [Mohcine]...
 * @param velocityUncertaintyX [Mohcine]...
 * @param velocityUncertaintyY [Mohcine]...
 * @param numSamples [Mohcine]...
 * @param rngSeed [Mohcine]...
 * */
JUPEDSIM_API JPS_WarpDriverModelBuilder
JPS_WarpDriverModelBuilder_Create(double timeHorizon,
        double stepSize,
        double sigma,
        double timeUncertainty,
        double velocityUncertaintyX,
        double velocityUncertaintyY,
        int numSamples,
        uint64_t rngSeed);

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
 * Read stuck Time (elapsed time since anchor was set) of this agent.
 * @param handle of the Agent to access.
 * @return stuck time of this agent
 */
JUPEDSIM_API double JPS_WarpDriverModelState_GetStuckTime(JPS_WarpDriverModelState handle);

/**
 * Write stuck Time (elapsed time since anchor was set) of this agent.
 * @param handle of the Agent to access.
 * @param stuckTime stuck Time (elapsed time since anchor was set) of this agent.
 */
JUPEDSIM_API void JPS_WarpDriverModelState_SetStuckTime(JPS_WarpDriverModelState handle, double stuckTime);

/**
 * Read anchorX (x position when stuck tracking began) of this agent.
 * @param handle of the Agent to access.
 * @return anchor x position of this agent
 */
JUPEDSIM_API double JPS_WarpDriverModelState_GetAnchorX(JPS_WarpDriverModelState handle);

/**
 * Write anchorX (x position when stuck tracking began) of this agent.
 * @param handle of the Agent to access.
 * @param anchorX anchor x position of this agent.
 */
JUPEDSIM_API void
JPS_WarpDriverModelState_SetAnchorX(JPS_WarpDriverModelState handle, double anchorX);

/**
 * Read anchorY (y position when stuck tracking began) of this agent.
 * @param handle of the Agent to access.
 * @return anchor y position of this agent
 */
JUPEDSIM_API double JPS_WarpDriverModelState_GetAnchorY(JPS_WarpDriverModelState handle);

/**
 * Write anchorY (y position when stuck tracking began) of this agent.
 * @param handle of the Agent to access.
 * @param anchorY anchor x position of this agent.
 */
JUPEDSIM_API void
JPS_WarpDriverModelState_SetAnchorY(JPS_WarpDriverModelState handle, double anchorY);

/**
 * Read detour Time of this agent.
 * @param handle of the Agent to access.
 * @return detour Time in s of this agent
 */
JUPEDSIM_API double JPS_WarpDriverModelState_GetDetourTime(JPS_WarpDriverModelState handle);

/**
 * Write detour Time of this agent.
 * @param handle of the Agent to access.
 * @param detourTime in s of this agent.
 */
JUPEDSIM_API void
JPS_WarpDriverModelState_SetDetourTime(JPS_WarpDriverModelState handle, double detourTime);

/**
 * Read detour side (+1 = left, -1 = right of desired direction) of this agent.
 * @param handle of the Agent to access.
 * @return agent Scale of this agent
 */
JUPEDSIM_API int JPS_WarpDriverModelState_GetDetourSide(JPS_WarpDriverModelState handle);

/**
 * Write detour side (+1 = left, -1 = right of desired direction) of this agent.
 * @param handle of the Agent to access.
 * @param detourSide detour side of this agent.
 */
JUPEDSIM_API void
JPS_WarpDriverModelState_SetDetourSide(JPS_WarpDriverModelState handle, int detourSide);

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
    /**
     * stuck time of the agent
     */
    double stuckTime = 0.0;
    /**
     * anchor X of the agent
     */
    double anchorX = 0.0;
    /**
     * anchor Y of the agent
     */
    double anchorY = 0.0;
    /**
     * detour Time the agent
     */
    double detourTime = 0.0;
    /**
     * detour side of the agent
     */
    int detourSide = 1;

} JPS_WarpDriverModelAgentParameters;

#ifdef __cplusplus
}
#endif
