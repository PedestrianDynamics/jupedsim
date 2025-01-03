/* Copyright © 2012-2024 Forschungszentrum Jülich GmbH */
/* SPDX-License-Identifier: LGPL-3.0-or-later */
#pragma once

#include "export.h"

#include <stddef.h> /*NOLINT(modernize-deprecated-headers)*/
#include <stdint.h> /*NOLINT(modernize-deprecated-headers)*/

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Contains basic performance trace information
 */
typedef struct JPS_Trace {
    /**
     * Duration of the iterate call in micorseconds
     */
    uint64_t iteration_duration;
    /**
     * Duration to compute updates of the operational decision level in micorseconds.
     * This is fully contained in iterate.
     */
    uint64_t operational_level_duration;
} JPS_Trace;

/**
 * A 2D coordinate. Units are 'meters'
 */
typedef struct JPS_Point {
    /**
     * x component in 'meters'
     */
    double x;
    /**
     * y component in 'meters'
     */
    double y;
} JPS_Point;

/**
 * Describes a waypoint.
 */
typedef struct JPS_Waypoint {
    /**
     * Position of the waypoint
     */
    JPS_Point position;
    /**
     * Distance in 'meters' at which this waypoint is considered to be reached.
     */
    double distance;
} JPS_Waypoint;

/**
 * Describes the pedestrian model used in the simulation.
 */
typedef enum JPS_ModelType {
    JPS_GeneralizedCentrifugalForceModel,
    JPS_CollisionFreeSpeedModel,
    JPS_CollisionFreeSpeedModelV2,
    JPS_AnticipationVelocityModel,
    JPS_SocialForceModel
} JPS_ModelType;

/**
 * Id of a journey.
 * Zero represents an invalid id.
 */
typedef uint64_t JPS_JourneyId;

/**
 * Id of a stage.
 * Zero represents an invalid id.
 */
typedef uint64_t JPS_StageId;

/**
 * Index of a stage within a journey.
 * Note that stage ids are only unique within the journey they refer to.
 */
typedef size_t JPS_StageIndex;

/**
 * Id of an agent.
 * Zero represents an invalid id.
 */
typedef uint64_t JPS_AgentId;

#ifdef __cplusplus
}
#endif
