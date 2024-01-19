/* Copyright © 2012-2023 Forschungszentrum Jülich GmbH */
/* SPDX-License-Identifier: LGPL-3.0-or-later */
#pragma once

#include "export.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Opaque type for error messages.
 * JPS_ErrorMessage will be returned via out parameters from functions that can fail.
 * JPS_ErrorMessage have to be deallocated with JPS_ErrorMessage_Free.
 */
typedef struct JPS_ErrorMessage_t* JPS_ErrorMessage;

/**
 * Access c-string representation of the error message.
 * Lifetime: the returned c-string will be valid until JPS_ErrorMessage_Free has been called on
 * this message.
 * @param handle to the JPS_ErrorMessage to inspect.
 * @return Error message as null terminated c string. This pointer is valid until
 * JPS_ErrorMessage_Free is called on this handle.
 */
JUPEDSIM_API const char* JPS_ErrorMessage_GetMessage(JPS_ErrorMessage handle);

/**
 * Frees a JPS_ErrorMessage
 * @param handle to the JPS_ErrorMessage to free.
 */
JUPEDSIM_API void JPS_ErrorMessage_Free(JPS_ErrorMessage handle);

typedef struct JPS_GeometryError_t* JPS_GeometryError;

#ifdef __cplusplus
}
#endif
