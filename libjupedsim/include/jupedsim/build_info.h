/* Copyright © 2012-2023 Forschungszentrum Jülich GmbH */
/* SPDX-License-Identifier: LGPL-3.0-or-later */
#pragma once

#include "export.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Contains build information about this library
 */
typedef struct JPS_BuildInfo {
    /**
     * Shortened commit id from which this version was build
     */
    const char* git_commit_hash;
    /**
     * Date the last commit was made
     */
    const char* git_commit_date;
    /**
     * Branch name from which this libabry was build
     */
    const char* git_branch;
    /**
     * Compiler identifier used to build this library
     */
    const char* compiler;
    /**
     * Compiler version used to build this library
     */
    const char* compiler_version;
    /**
     * Version of this library
     */
    const char* library_version;
} JPS_BuildInfo;

/**
 * Access meta information about the library.
 * @return build information about this library
 */
JUPEDSIM_API JPS_BuildInfo JPS_GetBuildInfo();

#ifdef __cplusplus
}
#endif
