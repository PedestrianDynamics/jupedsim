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
 * Opaque type for a SocialForceModelIPP Builder
 */
typedef struct JPS_SocialForceModelIPPBuilder_t* JPS_SocialForceModelIPPBuilder;

/**
 * Creates a SocialForceModelIPP builder.
 * The two-level model has no model-level parameters; all parameters are per-agent.
 */
JUPEDSIM_API JPS_SocialForceModelIPPBuilder
JPS_SocialForceModelIPPBuilder_Create();

/**
 * Creates a JPS_OperationalModel of type SocialForceModelIPP Model from the
 * JPS_SocialForceModelIPPBuilder.
 * @param handle the builder to operate on
 * @param[out] errorMessage if not NULL: will be set to a JPS_ErrorMessage in case of an error
 * @return a JPS_SocialForceModelIPP or NULL if an error occured.
 */
JUPEDSIM_API JPS_OperationalModel JPS_SocialForceModelIPPBuilder_Build(
    JPS_SocialForceModelIPPBuilder handle,
    JPS_ErrorMessage* errorMessage);

/**
 * Frees a JPS_SocialForceModelIPPBuilder
 * @param handle to the JPS_SocialForceModelIPPBuilder to free.
 */
JUPEDSIM_API void JPS_SocialForceModelIPPBuilder_Free(JPS_SocialForceModelIPPBuilder handle);

/**
 * Opaque type of Social Force Model IPP state
 */
typedef struct JPS_SocialForceModelIPPState_t* JPS_SocialForceModelIPPState;

JUPEDSIM_API JPS_Point JPS_SocialForceModelIPPState_GetVelocity(JPS_SocialForceModelIPPState handle);
JUPEDSIM_API void
JPS_SocialForceModelIPPState_SetVelocity(JPS_SocialForceModelIPPState handle, JPS_Point velocity);

JUPEDSIM_API JPS_Point JPS_SocialForceModelIPPState_GetGroundSupportPosition(JPS_SocialForceModelIPPState handle);
JUPEDSIM_API void
JPS_SocialForceModelIPPState_SetGroundSupportPosition(JPS_SocialForceModelIPPState handle, JPS_Point ground_support_position);

JUPEDSIM_API JPS_Point JPS_SocialForceModelIPPState_GetGroundSupportVelocity(JPS_SocialForceModelIPPState handle);
JUPEDSIM_API void
JPS_SocialForceModelIPPState_SetGroundSupportVelocity(JPS_SocialForceModelIPPState handle, JPS_Point ground_support_velocity);

JUPEDSIM_API double JPS_SocialForceModelIPPState_GetHeight(JPS_SocialForceModelIPPState handle);
JUPEDSIM_API void JPS_SocialForceModelIPPState_SetHeight(JPS_SocialForceModelIPPState handle, double height);

JUPEDSIM_API double JPS_SocialForceModelIPPState_GetDesiredSpeed(JPS_SocialForceModelIPPState handle);
JUPEDSIM_API void
JPS_SocialForceModelIPPState_SetDesiredSpeed(JPS_SocialForceModelIPPState handle, double desiredSpeed);

JUPEDSIM_API double JPS_SocialForceModelIPPState_GetReactionTime(JPS_SocialForceModelIPPState handle);
JUPEDSIM_API void
JPS_SocialForceModelIPPState_SetReactionTime(JPS_SocialForceModelIPPState handle, double reactionTime);

JUPEDSIM_API double JPS_SocialForceModelIPPState_GetLambdaU(JPS_SocialForceModelIPPState handle);
JUPEDSIM_API void
JPS_SocialForceModelIPPState_SetLambdaU(JPS_SocialForceModelIPPState handle, double lambdaU);

JUPEDSIM_API double JPS_SocialForceModelIPPState_GetLambdaB(JPS_SocialForceModelIPPState handle);
JUPEDSIM_API void
JPS_SocialForceModelIPPState_SetLambdaB(JPS_SocialForceModelIPPState handle, double lambdaB);

JUPEDSIM_API double JPS_SocialForceModelIPPState_GetBalanceSpeed(JPS_SocialForceModelIPPState handle);
JUPEDSIM_API void
JPS_SocialForceModelIPPState_SetBalanceSpeed(JPS_SocialForceModelIPPState handle, double balanceSpeed);

JUPEDSIM_API double JPS_SocialForceModelIPPState_GetDamping(JPS_SocialForceModelIPPState handle);
JUPEDSIM_API void
JPS_SocialForceModelIPPState_SetDamping(JPS_SocialForceModelIPPState handle, double damping);

JUPEDSIM_API double JPS_SocialForceModelIPPState_GetAgentScale(JPS_SocialForceModelIPPState handle);
JUPEDSIM_API void
JPS_SocialForceModelIPPState_SetAgentScale(JPS_SocialForceModelIPPState handle, double agentScale);

JUPEDSIM_API double JPS_SocialForceModelIPPState_GetObstacleScale(JPS_SocialForceModelIPPState handle);
JUPEDSIM_API void
JPS_SocialForceModelIPPState_SetObstacleScale(JPS_SocialForceModelIPPState handle, double obstacleScale);

JUPEDSIM_API double JPS_SocialForceModelIPPState_GetForceDistance(JPS_SocialForceModelIPPState handle);
JUPEDSIM_API void
JPS_SocialForceModelIPPState_SetForceDistance(JPS_SocialForceModelIPPState handle, double forceDistance);

JUPEDSIM_API double JPS_SocialForceModelIPPState_GetObstacleForceDistance(JPS_SocialForceModelIPPState handle);
JUPEDSIM_API void
JPS_SocialForceModelIPPState_SetObstacleForceDistance(JPS_SocialForceModelIPPState handle, double obstacleForceDistance);

JUPEDSIM_API double JPS_SocialForceModelIPPState_GetLegForceDistance(JPS_SocialForceModelIPPState handle);
JUPEDSIM_API void
JPS_SocialForceModelIPPState_SetLegForceDistance(JPS_SocialForceModelIPPState handle, double legForceDistance);

JUPEDSIM_API double JPS_SocialForceModelIPPState_GetRadius(JPS_SocialForceModelIPPState handle);
JUPEDSIM_API void
JPS_SocialForceModelIPPState_SetRadius(JPS_SocialForceModelIPPState handle, double radius);

JUPEDSIM_API double JPS_SocialForceModelIPPState_GetBodyForce(JPS_SocialForceModelIPPState handle);
JUPEDSIM_API void
JPS_SocialForceModelIPPState_SetBodyForce(JPS_SocialForceModelIPPState handle, double bodyForce);

JUPEDSIM_API double JPS_SocialForceModelIPPState_GetFriction(JPS_SocialForceModelIPPState handle);
JUPEDSIM_API void
JPS_SocialForceModelIPPState_SetFriction(JPS_SocialForceModelIPPState handle, double friction);

/**
 * Describes parameters of an Agent in SocialForceModelIPP
 */
typedef struct JPS_SocialForceModelIPPAgentParameters {
    JPS_Point position{0, 0};
    JPS_Point orientation{0, 0};
    JPS_JourneyId journeyId = 0;
    JPS_StageId stageId = 0;
    JPS_Point velocity = {0, 0};
    JPS_Point ground_support_position = {0, 0};
    JPS_Point ground_support_velocity = {0, 0};
    double height = 1.75;
    double desiredSpeed = 1.34;
    double reactionTime = 0.5;
    double lambdaU = 0.5;
    double lambdaB = 1.0;
    double balanceSpeed = 1.0;
    double damping = 1.0;
    double agentScale = 5.0;
    double obstacleScale = 5.0;
    double forceDistance = 0.5;
    double obstacleForceDistance = 0.2;
    double legForceDistance = 0.3;
    double radius = 0.15;
    double bodyForce = 10000.0;
    double friction = 0.5;

} JPS_SocialForceModelIPPAgentParameters;

#ifdef __cplusplus
}
#endif
