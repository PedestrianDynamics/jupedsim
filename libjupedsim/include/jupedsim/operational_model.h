/* Copyright © 2012-2024 Forschungszentrum Jülich GmbH */
/* SPDX-License-Identifier: LGPL-3.0-or-later */
#pragma once

#include "export.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Opaque type for operational models describing how agents interact in the simulation.
 */
typedef struct JPS_OperationalModel_t* JPS_OperationalModel;

/**
 * Frees a JPS_OperationalModel
 * @param handle to the JPS_OperationalModel to free.
 */
JUPEDSIM_API void JPS_OperationalModel_Free(JPS_OperationalModel handle);

#ifdef __cplusplus
}
#endif
