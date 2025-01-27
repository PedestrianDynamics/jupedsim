// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "export.h"

#ifdef __cplusplus
extern "C" {
#endif
/**
 * Callback type for logging
 */
typedef void (*JPS_LoggingCallBack)(const char*, void*);

/**
 * Register callback to receive debug level log messages.
 * To unregsiter a callback supply a NULL pointer as callback.
 * The optional userdata parameter is stored (non-owning) and always passed to the callback.
 * @param callback to call with debug message
 * @param userdata optional pointer to state needed by the callback
 */
JUPEDSIM_API void JPS_Logging_SetDebugCallback(JPS_LoggingCallBack callback, void* userdata);

/**
 * Register callback to receive info level log messages.
 * To unregsiter a callback supply a NULL pointer.
 * The optional userdata parameter is stored (non-owning) and always passed to the callback.
 * @param callback to call with debug message
 * @param userdata optional pointer to state needed by the callback
 */
JUPEDSIM_API void JPS_Logging_SetInfoCallback(JPS_LoggingCallBack callback, void* userdata);

/**
 * Register callback to receive warning level log messages.
 * To unregsiter a callback supply a NULL pointer.
 * The optional userdata parameter is stored (non-owning) and always passed to the callback.
 * @param callback to call with debug message
 * @param userdata optional pointer to state needed by the callback
 */
JUPEDSIM_API void JPS_Logging_SetWarningCallback(JPS_LoggingCallBack callback, void* userdata);

/**
 * Register callback to receive error level log messages.
 * To unregsiter a callback supply a NULL pointer.
 * The optional userdata parameter is stored (non-owning) and always passed to the callback.
 * @param callback to call with debug message
 * @param userdata optional pointer to state needed by the callback
 */
JUPEDSIM_API void JPS_Logging_SetErrorCallback(JPS_LoggingCallBack callback, void* userdata);

#ifdef __cplusplus
}
#endif
