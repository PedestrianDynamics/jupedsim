#pragma once

#include "error.h"
#include "export.h"
#include "operational_model.h"
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Opaque type for a SocialForceModel Builder
 */
typedef struct JPS_SocialForceModelBuilder_t* JPS_SocialForceModelBuilder;

/**
 * Creates a SocialForceModel builder.
 * @param bodyForce describes the strength with which an agent is influenced
 * by pushing forces from obstacles and neighbors in its direct proximity.
 * @param friction describes the strength with which an agent is influenced
 * by frictional forces from obstacles and neighbors in its direct proximity.
 * */
JUPEDSIM_API JPS_SocialForceModelBuilder
JPS_SocialForceModelBuilder_Create(double bodyForce, double friction);

/**
 * Creates a JPS_OperationalModel of type SocialForceModel Model from the
 * JPS_SocialForceModelBuilder.
 * @param handle the builder to operate on
 * @param[out] errorMessage if not NULL: will be set to a JPS_ErrorMessage in case of an error
 * @return a JPS_SocialForceModel or NULL if an error occured.
 */
JUPEDSIM_API JPS_OperationalModel JPS_SocialForceModelBuilder_Build(
    JPS_SocialForceModelBuilder handle,
    JPS_ErrorMessage* errorMessage);

/**
 * Frees a JPS_SocialForceModelBuilder
 * @param handle to the JPS_SocialForceModelBuilder to free.
 */
JUPEDSIM_API void JPS_SocialForceModelBuilder_Free(JPS_SocialForceModelBuilder handle);

/**
 * Opaque type of Social Force model state
 */
typedef struct JPS_SocialForceModelState_t* JPS_SocialForceModelState;

/**
 * Read Velocity of this agent.
 * @param handle of the Agent to access.
 * @return Velocity of this agent.
 */
JUPEDSIM_API JPS_Point JPS_SocialForceModelState_GetVelocity(JPS_SocialForceModelState handle);

/**
 * Write Velocity of this agent.
 * @param handle of the Agent to access.
 * @param velocity Velocity of this agent.
 */
JUPEDSIM_API void
JPS_SocialForceModelState_SetVelocity(JPS_SocialForceModelState handle, JPS_Point velocity);

/**
 * Read mass of this agent.
 * @param handle of the Agent to access.
 * @return mass in kg of this agent
 */
JUPEDSIM_API double JPS_SocialForceModelState_GetMass(JPS_SocialForceModelState handle);

/**
 * Write mass of this agent.
 * @param handle of the Agent to access.
 * @param mass in kg of this agent.
 */
JUPEDSIM_API void JPS_SocialForceModelState_SetMass(JPS_SocialForceModelState handle, double mass);

/**
 * Read desired Speed of this agent.
 * @param handle of the Agent to access.
 * @return desired Speed in m/s of this agent
 */
JUPEDSIM_API double JPS_SocialForceModelState_GetDesiredSpeed(JPS_SocialForceModelState handle);

/**
 * Write desired Speed of this agent.
 * @param handle of the Agent to access.
 * @param desiredSpeed in m/s of this agent.
 */
JUPEDSIM_API void
JPS_SocialForceModelState_SetDesiredSpeed(JPS_SocialForceModelState handle, double desiredSpeed);

/**
 * Read reaction Time of this agent.
 * @param handle of the Agent to access.
 * @return reaction Time in s of this agent
 */
JUPEDSIM_API double JPS_SocialForceModelState_GetReactionTime(JPS_SocialForceModelState handle);

/**
 * Write reaction Time of this agent.
 * @param handle of the Agent to access.
 * @param reactionTime in s of this agent.
 */
JUPEDSIM_API void
JPS_SocialForceModelState_SetReactionTime(JPS_SocialForceModelState handle, double reactionTime);

/**
 * Read agent Scale of this agent.
 * @param handle of the Agent to access.
 * @return agent Scale of this agent
 */
JUPEDSIM_API double JPS_SocialForceModelState_GetAgentScale(JPS_SocialForceModelState handle);

/**
 * Write agent Scale of this agent.
 * @param handle of the Agent to access.
 * @param agentScale of this agent.
 */
JUPEDSIM_API void
JPS_SocialForceModelState_SetAgentScale(JPS_SocialForceModelState handle, double agentScale);

/**
 * Read obstacle Scale of this agent.
 * @param handle of the Agent to access.
 * @return obstacle Scale of this agent
 */
JUPEDSIM_API double JPS_SocialForceModelState_GetObstacleScale(JPS_SocialForceModelState handle);

/**
 * Write obstacle Scale of this agent.
 * @param handle of the Agent to access.
 * @param obstacleScale of this agent.
 */
JUPEDSIM_API void
JPS_SocialForceModelState_SetObstacleScale(JPS_SocialForceModelState handle, double obstacleScale);

/**
 * Read force Distance of this agent.
 * @param handle of the Agent to access.
 * @return force Distance of this agent
 */
JUPEDSIM_API double JPS_SocialForceModelState_GetForceDistance(JPS_SocialForceModelState handle);

/**
 * Write force Distance of this agent.
 * @param handle of the Agent to access.
 * @param forceDistance of this agent.
 */
JUPEDSIM_API void
JPS_SocialForceModelState_SetForceDistance(JPS_SocialForceModelState handle, double forceDistance);

/**
 * Read radius of this agent.
 * @param handle of the Agent to access.
 * @return radius in m of this agent
 */
JUPEDSIM_API double JPS_SocialForceModelState_GetRadius(JPS_SocialForceModelState handle);

/**
 * Write radius of this agent.
 * @param handle of the Agent to access.
 * @param radius in m of this agent.
 */
JUPEDSIM_API void
JPS_SocialForceModelState_SetRadius(JPS_SocialForceModelState handle, double radius);

/**
 * Describes parameters of an Agent in SocialForceModel
 */
typedef struct JPS_SocialForceModelAgentParameters {
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
     * Initial velocity of the Agent
     */
    JPS_Point velocity = {0, 0};
    /**
     * Mass of the agent
     */
    double mass = 80;
    /**
     * desired Speed of the agent
     */
    double desiredSpeed = 0.8;
    /**
     * reaction Time of the agent
     */
    double reactionTime = 0.5;
    /**
     * agent Scale of the agent
     */
    double agentScale = 2000;
    /**
     * obstacle Scale of the agent
     */
    double obstacleScale = 2000;
    /**
     * force Distance of the agent
     */
    double forceDistance = 0.08;
    /**
     * radius of the agent
     */
    double radius = 0.3;

} JPS_SocialForceModelAgentParameters;

#ifdef __cplusplus
}
#endif