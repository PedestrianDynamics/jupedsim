#pragma once

#include "error.h"
#include "export.h"
#include "operational_model.h"
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Opaque type for a SocialForceModel Model Builder
 */
typedef struct JPS_SocialForceModelBuilder_t*
    JPS_SocialForceModelBuilder;

/**
 * Creates a SocialForceModel model builder.
 * @param test_value is a test parameter that needs to be replaced with actual values
 */

JUPEDSIM_API JPS_SocialForceModelBuilder JPS_SocialForceModelBuilder_Create(
    double test_value);

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
JUPEDSIM_API void JPS_SocialForceModelBuilder_Free(
    JPS_SocialForceModelBuilder handle);

/**
 * Opaque type of Social Force model state
 */
typedef struct JPS_SocialForceModelState_t*
    JPS_SocialForceModelState;

// no getters or setters implemented yet

/**
 * Describes parameters of an Agent in SocialForceModel
 */
typedef struct JPS_SocialForceModelAgentParameters {
    /**
     * Test Value
     */
    double test_value = 2.5;

/**
 * Read test value of this agent.
 * @param handle of the Agent to access.
 * @return test value without any meaning
 */
JUPEDSIM_API double JPS_SocialForceModelState_GetTestValue(
    JPS_SocialForceModelState handle);

/**
 * Write test value of this agent.
 * @param handle of the Agent to access.
 * @param test_value test value without any meaning
 */
JUPEDSIM_API void JPS_SocialForceModelState_SetTestValue(
    JPS_GeneralizedCentrifugalForceModelState handle,
    double test_value);


} JPS_SocialForceModelAgentParameters;
#ifdef __cplusplus
}
#endif