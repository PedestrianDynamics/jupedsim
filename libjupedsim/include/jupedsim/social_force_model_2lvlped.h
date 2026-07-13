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
 * Opaque type for a SocialForceModel2LvlPed Builder
 */
typedef struct JPS_SocialForceModel2LvlPedBuilder_t* JPS_SocialForceModel2LvlPedBuilder;

/**
 * Creates a SocialForceModel2LvlPed builder.
 * The two-level model has no model-level parameters; all parameters are per-agent.
 */
JUPEDSIM_API JPS_SocialForceModel2LvlPedBuilder
JPS_SocialForceModel2LvlPedBuilder_Create();

/**
 * Creates a JPS_OperationalModel of type SocialForceModel2LvlPed Model from the
 * JPS_SocialForceModel2LvlPedBuilder.
 * @param handle the builder to operate on
 * @param[out] errorMessage if not NULL: will be set to a JPS_ErrorMessage in case of an error
 * @return a JPS_SocialForceModel2LvlPed or NULL if an error occured.
 */
JUPEDSIM_API JPS_OperationalModel JPS_SocialForceModel2LvlPedBuilder_Build(
    JPS_SocialForceModel2LvlPedBuilder handle,
    JPS_ErrorMessage* errorMessage);

/**
 * Frees a JPS_SocialForceModel2LvlPedBuilder
 * @param handle to the JPS_SocialForceModel2LvlPedBuilder to free.
 */
JUPEDSIM_API void JPS_SocialForceModel2LvlPedBuilder_Free(JPS_SocialForceModel2LvlPedBuilder handle);

/**
 * Opaque type of Social Force Model IPP state
 */
typedef struct JPS_SocialForceModel2LvlPedState_t* JPS_SocialForceModel2LvlPedState;

JUPEDSIM_API JPS_Point JPS_SocialForceModel2LvlPedState_GetVelocity(JPS_SocialForceModel2LvlPedState handle);
JUPEDSIM_API void
JPS_SocialForceModel2LvlPedState_SetVelocity(JPS_SocialForceModel2LvlPedState handle, JPS_Point velocity);

JUPEDSIM_API JPS_Point JPS_SocialForceModel2LvlPedState_GetGroundSupportPosition(JPS_SocialForceModel2LvlPedState handle);
JUPEDSIM_API void
JPS_SocialForceModel2LvlPedState_SetGroundSupportPosition(JPS_SocialForceModel2LvlPedState handle, JPS_Point ground_support_position);

JUPEDSIM_API JPS_Point JPS_SocialForceModel2LvlPedState_GetGroundSupportVelocity(JPS_SocialForceModel2LvlPedState handle);
JUPEDSIM_API void
JPS_SocialForceModel2LvlPedState_SetGroundSupportVelocity(JPS_SocialForceModel2LvlPedState handle, JPS_Point ground_support_velocity);

JUPEDSIM_API double JPS_SocialForceModel2LvlPedState_GetHeight(JPS_SocialForceModel2LvlPedState handle);
JUPEDSIM_API void JPS_SocialForceModel2LvlPedState_SetHeight(JPS_SocialForceModel2LvlPedState handle, double height);

JUPEDSIM_API double JPS_SocialForceModel2LvlPedState_GetDesiredSpeed(JPS_SocialForceModel2LvlPedState handle);
JUPEDSIM_API void
JPS_SocialForceModel2LvlPedState_SetDesiredSpeed(JPS_SocialForceModel2LvlPedState handle, double desiredSpeed);

JUPEDSIM_API double JPS_SocialForceModel2LvlPedState_GetReactionTime(JPS_SocialForceModel2LvlPedState handle);
JUPEDSIM_API void
JPS_SocialForceModel2LvlPedState_SetReactionTime(JPS_SocialForceModel2LvlPedState handle, double reactionTime);

JUPEDSIM_API double JPS_SocialForceModel2LvlPedState_GetLambdaU(JPS_SocialForceModel2LvlPedState handle);
JUPEDSIM_API void
JPS_SocialForceModel2LvlPedState_SetLambdaU(JPS_SocialForceModel2LvlPedState handle, double lambdaU);

JUPEDSIM_API double JPS_SocialForceModel2LvlPedState_GetLambdaB(JPS_SocialForceModel2LvlPedState handle);
JUPEDSIM_API void
JPS_SocialForceModel2LvlPedState_SetLambdaB(JPS_SocialForceModel2LvlPedState handle, double lambdaB);

JUPEDSIM_API double JPS_SocialForceModel2LvlPedState_GetBalanceSpeed(JPS_SocialForceModel2LvlPedState handle);
JUPEDSIM_API void
JPS_SocialForceModel2LvlPedState_SetBalanceSpeed(JPS_SocialForceModel2LvlPedState handle, double balanceSpeed);

JUPEDSIM_API double JPS_SocialForceModel2LvlPedState_GetDamping(JPS_SocialForceModel2LvlPedState handle);
JUPEDSIM_API void
JPS_SocialForceModel2LvlPedState_SetDamping(JPS_SocialForceModel2LvlPedState handle, double damping);

JUPEDSIM_API double JPS_SocialForceModel2LvlPedState_GetAgentScale(JPS_SocialForceModel2LvlPedState handle);
JUPEDSIM_API void
JPS_SocialForceModel2LvlPedState_SetAgentScale(JPS_SocialForceModel2LvlPedState handle, double agentScale);

JUPEDSIM_API double JPS_SocialForceModel2LvlPedState_GetObstacleScale(JPS_SocialForceModel2LvlPedState handle);
JUPEDSIM_API void
JPS_SocialForceModel2LvlPedState_SetObstacleScale(JPS_SocialForceModel2LvlPedState handle, double obstacleScale);

JUPEDSIM_API double JPS_SocialForceModel2LvlPedState_GetLegScale(JPS_SocialForceModel2LvlPedState handle);
JUPEDSIM_API void
JPS_SocialForceModel2LvlPedState_SetLegScale(JPS_SocialForceModel2LvlPedState handle, double legScale);

JUPEDSIM_API double JPS_SocialForceModel2LvlPedState_GetForceDistance(JPS_SocialForceModel2LvlPedState handle);
JUPEDSIM_API void
JPS_SocialForceModel2LvlPedState_SetForceDistance(JPS_SocialForceModel2LvlPedState handle, double forceDistance);

JUPEDSIM_API double JPS_SocialForceModel2LvlPedState_GetObstacleForceDistance(JPS_SocialForceModel2LvlPedState handle);
JUPEDSIM_API void
JPS_SocialForceModel2LvlPedState_SetObstacleForceDistance(JPS_SocialForceModel2LvlPedState handle, double obstacleForceDistance);

JUPEDSIM_API double JPS_SocialForceModel2LvlPedState_GetLegForceDistance(JPS_SocialForceModel2LvlPedState handle);
JUPEDSIM_API void
JPS_SocialForceModel2LvlPedState_SetLegForceDistance(JPS_SocialForceModel2LvlPedState handle, double legForceDistance);

JUPEDSIM_API double JPS_SocialForceModel2LvlPedState_GetRadius(JPS_SocialForceModel2LvlPedState handle);
JUPEDSIM_API void
JPS_SocialForceModel2LvlPedState_SetRadius(JPS_SocialForceModel2LvlPedState handle, double radius);

JUPEDSIM_API double JPS_SocialForceModel2LvlPedState_GetBodyForce(JPS_SocialForceModel2LvlPedState handle);
JUPEDSIM_API void
JPS_SocialForceModel2LvlPedState_SetBodyForce(JPS_SocialForceModel2LvlPedState handle, double bodyForce);

JUPEDSIM_API double JPS_SocialForceModel2LvlPedState_GetFriction(JPS_SocialForceModel2LvlPedState handle);
JUPEDSIM_API void
JPS_SocialForceModel2LvlPedState_SetFriction(JPS_SocialForceModel2LvlPedState handle, double friction);

/**
 * Describes parameters of an Agent in SocialForceModel2LvlPed
 */
typedef struct JPS_SocialForceModel2LvlPedAgentParameters {
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
    double legScale = 5.0;
    double forceDistance = 0.5;
    double obstacleForceDistance = 0.2;
    double legForceDistance = 0.3;
    double radius = 0.15;
    double bodyForce = 10000.0;
    double friction = 0.5;

} JPS_SocialForceModel2LvlPedAgentParameters;

#ifdef __cplusplus
}
#endif
