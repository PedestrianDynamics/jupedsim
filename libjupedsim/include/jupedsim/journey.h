/* Copyright © 2012-2023 Forschungszentrum Jülich GmbH */
/* SPDX-License-Identifier: LGPL-3.0-or-later */
#pragma once

#include "error.h"
#include "export.h"
#include "transition.h"
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Opaque type that describes a journey
 */
typedef struct JPS_Journey_t* JPS_JourneyDescription;

/**
 * Creates an empty journey.
 */
JUPEDSIM_API JPS_JourneyDescription JPS_JourneyDescription_Create();

/**
 * Extends the journey with the stage specified by its ID.
 * @param handle of the JourneyDescription to extend.
 * @param id of the stage to extend the journey with.
 */
JUPEDSIM_API void JPS_JourneyDescription_AddStage(JPS_JourneyDescription handle, JPS_StageId id);

/**
 * Specifies the transition to the next stage, once this stage is completed.
 * @param handle of the JourneyDescription to modify.
 * @param id of the stage to set the transition for.
 * @param transition transition to the next stage.
 * @param[out] errorMessage if not NULL: will be set to a JPS_ErrorMessage in case of an error.
 * @return true if the transition for the stage could be added otherwise false
 */
JUPEDSIM_API bool JPS_JourneyDescription_SetTransitionForStage(
    JPS_JourneyDescription handle,
    JPS_StageId id,
    JPS_Transition transition,
    JPS_ErrorMessage* errorMessage);

/**
 * Frees a JPS_Journey.
 * @param handle to the JPS_Journey to free.
 */
JUPEDSIM_API void JPS_JourneyDescription_Free(JPS_JourneyDescription handle);

#ifdef __cplusplus
}
#endif
