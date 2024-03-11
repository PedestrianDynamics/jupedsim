/* Copyright © 2012-2024 Forschungszentrum Jülich GmbH */
/* SPDX-License-Identifier: LGPL-3.0-or-later */
#pragma once

#include "collision_free_speed_model.h"
#include "collision_free_speed_model_v2.h"
#include "error.h"
#include "export.h"
#include "generalized_centrifugal_force_model.h"
#include "social_force_model.h"
#include "types.h"

#include <stdbool.h> /*NOLINT(modernize-deprecated-headers)*/

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Opaque type of an agent
 */
typedef struct JPS_Agent_t* JPS_Agent;

/**
 * Access the agents id.
 * @param handle of the agent to access.
 * @return the id
 */
JUPEDSIM_API JPS_AgentId JPS_Agent_GetId(JPS_Agent handle);

/**
 * Access the agents journey id.
 * @param handle of the agent to access.
 * @return the id of this agents journey
 */
JUPEDSIM_API JPS_JourneyId JPS_Agent_GetJourneyId(JPS_Agent handle);

/**
 * Access the agents currently targeted stage id.
 * @param handle of the agent to access.
 * @return the id of the stage currently targeted
 */
JUPEDSIM_API JPS_StageId JPS_Agent_GetStageId(JPS_Agent handle);

/**
 * Access the agents position.
 * @param handle of the agent to access.
 * @return position
 */
JUPEDSIM_API JPS_Point JPS_Agent_GetPosition(JPS_Agent handle);

/**
 * Access the agents current target.
 * @param handle of the agent to access.
 * @return target of the agent
 */
JUPEDSIM_API JPS_Point JPS_Agent_GetTarget(JPS_Agent handle);

JUPEDSIM_API bool
JPS_Agent_SetTarget(JPS_Agent handle, JPS_Point target, JPS_ErrorMessage* errorMessage);

/**
 * Access the agents orientation.
 * The orientation is unit length.
 * @param handle of the agent to access.
 * @return the orientation
 */
JUPEDSIM_API JPS_Point JPS_Agent_GetOrientation(JPS_Agent handle);

/**
 * Access the agets model type information.
 * @param handle of the agent to access.
 * @return type of model in use
 */
JUPEDSIM_API JPS_ModelType JPS_Agent_GetModelType(JPS_Agent handle);

/**
 * Access Generalized Centrifugal Force model state.
 * Precondition: Agent needs to use Generalized Centrifugal Force model
 * @param handle of the agent to access.
 * @param[out] errorMessage if not NULL: will be set to a JPS_ErrorMessage in case of an error.
 * @return state or NULL on error
 */
JUPEDSIM_API JPS_GeneralizedCentrifugalForceModelState
JPS_Agent_GetGeneralizedCentrifugalForceModelState(
    JPS_Agent handle,
    JPS_ErrorMessage* errorMessage);

/**
 * Access Collision Free Speed model state.
 * Precondition: Agent needs to use Collision Free Speed model
 * @param handle of the agent to access.
 * @param[out] errorMessage if not NULL: will be set to a JPS_ErrorMessage in case of an error.
 * @return state or NULL on error
 */
JUPEDSIM_API JPS_CollisionFreeSpeedModelState
JPS_Agent_GetCollisionFreeSpeedModelState(JPS_Agent handle, JPS_ErrorMessage* errorMessage);

/**
 * Access Social Force model state.
 * Precondition: Agent needs to use Social Force model
 * @param handle of the agent to access.
 * @param[out] errorMessage if not NULL: will be set to a JPS_ErrorMessage in case of an error.
 * @return state or NULL on error
 */
JUPEDSIM_API JPS_SocialForceModelState
JPS_Agent_GetSocialForceModelState(JPS_Agent handle, JPS_ErrorMessage* errorMessage);

/**
 * Access Collision Free Speed model v2 state.
 * Precondition: Agent needs to use Collision Free Speed model v2
 * @param handle of the agent to access.
 * @param[out] errorMessage if not NULL: will be set to a JPS_ErrorMessage in case of an error.
 * @return state or NULL on error
 */
JUPEDSIM_API JPS_CollisionFreeSpeedModelv2State
JPS_Agent_GetCollisionFreeSpeedModelv2State(JPS_Agent handle, JPS_ErrorMessage* errorMessage);

/**
 * Opaque type of an iterator over agents
 */
typedef struct JPS_AgentIterator_t* JPS_AgentIterator;

/**
 * Access the next element in the iterator.
 * Calling JPS_AgentIterator_Next repeatedly on a finished iterator is save.
 * @param handle of the iterator to advance and access
 * @return an agent or NULL if the iterator is at the end. The pointer returned does not need to be
 * freed and is invalidated on the next call to this function!
 */
JUPEDSIM_API JPS_Agent JPS_AgentIterator_Next(JPS_AgentIterator handle);

/**
 * Free the iterator.
 * @param handle to the JPS_AgentIterator to free.
 */
JUPEDSIM_API void JPS_AgentIterator_Free(JPS_AgentIterator handle);

/**
 * Opaque type of an iterator over agent ids
 */
typedef struct JPS_AgentIdIterator_t* JPS_AgentIdIterator;

/**
 * Access the next element in the iterator.
 * Calling JPS_AgentIterator_Next repeatedly on a finished iterator is save.
 * @param handle of the iterator to advance and access
 * @return an agentId, Zero in case the iterator has reachedits end.
 */
JUPEDSIM_API JPS_AgentId JPS_AgentIdIterator_Next(JPS_AgentIdIterator handle);

/**
 * Free the iterator.
 * @param handle to the JPS_AgentIterator to free.
 */
JUPEDSIM_API void JPS_AgentIdIterator_Free(JPS_AgentIdIterator handle);
#ifdef __cplusplus
}
#endif
