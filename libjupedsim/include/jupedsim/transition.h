// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "error.h"
#include "export.h"
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Opaque type for transition, describing route based decisions.
 */
typedef struct JPS_Transition_t* JPS_Transition;

/**
 * Create a fixed transition to stage
 *
 * A transition can be reused for many journeys (they are copied on assignment. After
 * they are added to the last journey they need to be freed.
 *
 * @param[out] errorMessage if not NULL: will be set to a JPS_ErrorMessage in case of an error.
 * @return Fixed transition to stage or NULL on any error.
 */
JUPEDSIM_API JPS_Transition
JPS_Transition_CreateFixedTransition(JPS_StageId stageId, JPS_ErrorMessage* errorMessage);

/**
 * Create a round robin transition to stages
 *
 * A transition can be reused for many journeys (they are copied on assignment. After
 * they are added to the last journey they need to be freed.
 *
 * @param stages target stages
 * @param weights weights of target stage
 * @param len length of stages and weights
 * @param[out] errorMessage if not NULL: will be set to a JPS_ErrorMessage in case of an error.
 * @return Round robin transition to stages or NULL on any error.
 */
JUPEDSIM_API JPS_Transition JPS_Transition_CreateRoundRobinTransition(
    JPS_StageId* stages,
    uint64_t* weights,
    size_t len_stages,
    JPS_ErrorMessage* errorMessage);

/**
 * Create a least targeted transition to stages
 *
 * A transition can be reused for many journeys (they are copied on assignment. After
 * they are added to the last journey they need to be freed.
 *
 * @param stages target stages
 * @param len length of stages
 * @param[out] errorMessage if not NULL: will be set to a JPS_ErrorMessage in case of an error.
 * @return least targeted transition to stages or NULL on any error.
 */
JUPEDSIM_API JPS_Transition JPS_Transition_CreateLeastTargetedTransition(
    JPS_StageId* stages,
    size_t len_stages,
    JPS_ErrorMessage* errorMessage);

/**
 * Frees a JPS_Transition
 * @param handle to the JPS_Transition to free.
 */
JUPEDSIM_API void JPS_Transition_Free(JPS_Transition handle);

#ifdef __cplusplus
}
#endif
