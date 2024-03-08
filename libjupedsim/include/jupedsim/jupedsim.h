/* Copyright © 2012-2024 Forschungszentrum Jülich GmbH */
/* SPDX-License-Identifier: LGPL-3.0-or-later */
#pragma once

#include <cstddef>
#ifdef _WIN32
#ifdef JUPEDSIM_API_EXPORTS
#define JUPEDSIM_API __declspec(dllexport)
#else
#define JUPEDSIM_API __declspec(dllimport)
#endif
#else
#define JUPEDSIM_API
#endif

#include <stdbool.h> /*NOLINT(modernize-deprecated-headers)*/
#include <stddef.h> /*NOLINT(modernize-deprecated-headers)*/
#include <stdint.h> /*NOLINT(modernize-deprecated-headers)*/

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
    JPS_CollisionFreeSpeedModelV2
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

/**
 * Opaque type for error regarding the geometry.
 */
typedef struct JPS_GeometryError_t* JPS_GeometryError;

/**
 * Opaque type for operational models describing how agents interact in the simulation.
 */
typedef struct JPS_OperationalModel_t* JPS_OperationalModel;

/**
 * Frees a JPS_OperationalModel
 * @param handle to the JPS_OperationalModel to free.
 */
JUPEDSIM_API void JPS_OperationalModel_Free(JPS_OperationalModel handle);

/**
 * Opaque type for a GeneralizedCentrifugalForceModel Model Builder
 */
typedef struct JPS_GeneralizedCentrifugalForceModelBuilder_t*
    JPS_GeneralizedCentrifugalForceModelBuilder;

/**
 * Creates a GeneralizedCentrifugalForceModel model builder.
 * @param strengthNeighborRepulsion  Strength of the repulsion from neighbors
 * @param strengthGeometryRepulsion  Strength of the repulsion from geometry boundaries
 * @param maxNeighborInteractionDistance  cut-off-radius for ped-ped repulsion (r_c in FIG. 7)
 * @param maxGeometryInteractionDistance  cut-off-radius for ped-wall repulsion (r_c in FIG. 7)
 * @param maxNeighborInterpolationDistance  distance of interpolation of repulsive force for
 * ped-ped interaction (r_eps in FIG. 7)
 * @param maxGeometryInterpolationDistance  distance of interpolation of repulsive force for
 * ped-wall interaction (r_eps in FIG. 7)
 * @param maxNeighborRepulsionForce  maximum of the repulsion force for ped-ped interaction by
 * contact of ellipses (f_m in FIG. 7)
 * @param maxGeometryRepulsionForce  maximum of the repulsion force for ped-wall interaction by
 * contact of ellipses (f_m in FIG. 7)
 * @return the GeneralizedCentrifugalForceModel model builder
 */
JUPEDSIM_API JPS_GeneralizedCentrifugalForceModelBuilder
JPS_GeneralizedCentrifugalForceModelBuilder_Create(
    double strengthNeighborRepulsion,
    double strengthGeometryRepulsion,
    double maxNeighborInteractionDistance,
    double maxGeometryInteractionDistance,
    double maxNeighborInterpolationDistance,
    double maxGeometryInterpolationDistance,
    double maxNeighborRepulsionForce,
    double maxGeometryRepulsionForce);

/**
 * Creates a JPS_OperationalModel of type GeneralizedCentrifugalForceModel Model from the
 * JPS_GeneralizedCentrifugalForceModelBuilder.
 * @param handle the builder to operate on
 * @param[out] errorMessage if not NULL: will be set to a JPS_ErrorMessage in case of an error
 * @return a JPS_GeneralizedCentrifugalForceModel or NULL if an error occured.
 */
JUPEDSIM_API JPS_OperationalModel JPS_GeneralizedCentrifugalForceModelBuilder_Build(
    JPS_GeneralizedCentrifugalForceModelBuilder handle,
    JPS_ErrorMessage* errorMessage);

/**
 * Frees a JPS_GeneralizedCentrifugalForceModelBuilder
 * @param handle to the JPS_GeneralizedCentrifugalForceModelBuilder to free.
 */
JUPEDSIM_API void JPS_GeneralizedCentrifugalForceModelBuilder_Free(
    JPS_GeneralizedCentrifugalForceModelBuilder handle);

/**
 * Opaque type for a Collision Free Speed Model Builder
 */
typedef struct JPS_CollisionFreeSpeedModelBuilder_t* JPS_CollisionFreeSpeedModelBuilder;

/**
 * Creates a Collision Free Speed Model builder.
 * @param strength_neighbor_repulsion describes the strength with which neighbors repulse each
 * other.
 * @param range_neighbor_repulsion describes the range at hich neighbors repulse each other.
 * @param strength_geometry_repulsion describes the strength with which neighbors are repules by
 * geometry.
 * @param range_geometry_repulsion describes the range at hich neighbors are repulsed by geometry.
 * @return the builder
 */
JUPEDSIM_API JPS_CollisionFreeSpeedModelBuilder JPS_CollisionFreeSpeedModelBuilder_Create(
    double strengthNeighborRepulsion,
    double rangeNeighborRepulsion,
    double strengthGeometryRepulsion,
    double rangeGeometryRepulsion);

/**
 * Creates a JPS_OperationalModel of type Collision Free Speed Model from the
 * JPS_GeneralizedCentrifugalForceModelBuilder.
 * @param handle the builder to operate on
 * @param[out] errorMessage if not NULL: will be set to a JPS_ErrorMessage in case of an error
 * @return a JPS_GeneralizedCentrifugalForceModel or NULL if an error occured.
 */
JUPEDSIM_API JPS_OperationalModel JPS_CollisionFreeSpeedModelBuilder_Build(
    JPS_CollisionFreeSpeedModelBuilder handle,
    JPS_ErrorMessage* errorMessage);

/**
 * Frees a JPS_CollisionFreeSpeedModelBuilder
 * @param handle to the JPS_CollisionFreeSpeedModelBuilder to free.
 */
JUPEDSIM_API void
JPS_CollisionFreeSpeedModelBuilder_Free(JPS_CollisionFreeSpeedModelBuilder handle);

/**
 * Opaque type for a Collision Free Speed Model V2 Builder
 */
typedef struct JPS_CollisionFreeSpeedModelV2Builder_t* JPS_CollisionFreeSpeedModelV2Builder;

/**
 * Creates a Collision Free Speed Model V2 builder.
 * @return the builder
 */
JUPEDSIM_API JPS_CollisionFreeSpeedModelV2Builder JPS_CollisionFreeSpeedModelV2Builder_Create();

/**
 * Creates a JPS_OperationalModel of type Collision Free Speed Model V2 from the
 * JPS_CollisionFreeSpeedModelV2Builder.
 * @param handle the builder to operate on
 * @param[out] errorMessage if not NULL: will be set to a JPS_ErrorMessage in case of an error
 * @return a JPS_CollisionFreeSpeedModelV2 or NULL if an error occured.
 */
JUPEDSIM_API JPS_OperationalModel JPS_CollisionFreeSpeedModelV2Builder_Build(
    JPS_CollisionFreeSpeedModelV2Builder handle,
    JPS_ErrorMessage* errorMessage);

/**
 * Frees a JPS_CollisionFreeSpeedModelV2Builder
 * @param handle to the JPS_CollisionFreeSpeedModelV2Builder to free.
 */
JUPEDSIM_API void
JPS_CollisionFreeSpeedModelV2Builder_Free(JPS_CollisionFreeSpeedModelV2Builder handle);

/**
 * Opaque type that represents the geometry the simulation acts on.
 * This type is created from JPS_GeometryBuilder.
 */
typedef struct JPS_Geometry_t const* JPS_Geometry;

/**
 * Returns the number of points that the outer boundary of the geometry consists of.
 * @param handle to the JPS_Geometry to operate on
 * @return Number of points in the outer boundary
 */
JUPEDSIM_API size_t JPS_Geometry_GetBoundarySize(JPS_Geometry handle);

/**
 * Returns the points the outer boundary consists of.
 * @param handle to the JPS_Geometry to operate on
 * @return the outer boundary. Do not free the returned pointer, it is still owned by JPS_Geometry.
 */
JUPEDSIM_API const JPS_Point* JPS_Geometry_GetBoundaryData(JPS_Geometry handle);

/**
 * Returns the number of holes in the geometry.
 * @param handle to the JPS_Geometry to operate on
 * @return Number of holes
 */
JUPEDSIM_API size_t JPS_Geometry_GetHoleCount(JPS_Geometry handle);

/**
 * Returns the number of points that hole consists of.
 * @param handle to the JPS_Geometry to operate on
 * @param index of the hole to access
 * @param[out] errorMessage if not NULL: will be set to a JPS_ErrorMessage in case of an error
 * @return Number of points in hole boundary or 0 on error.
 */
JUPEDSIM_API size_t
JPS_Geometry_GetHoleSize(JPS_Geometry handle, size_t hole_index, JPS_ErrorMessage* errorMessage);

/**
 * Returns the points the hole boundary consists of.
 * @param handle to the JPS_Geometry to operate on
 * @param index of the hole to access
 * @param[out] errorMessage if not NULL: will be set to a JPS_ErrorMessage in case of an error
 * @return points of the hole boundary or NULL on error.
 */
JUPEDSIM_API const JPS_Point*
JPS_Geometry_GetHoleData(JPS_Geometry handle, size_t hole_index, JPS_ErrorMessage* errorMessage);

/**
 * Frees a JPS_Geometry
 * @param handle to the JPS_Geometry to free.
 */
JUPEDSIM_API void JPS_Geometry_Free(JPS_Geometry geometry);

/**
 * Opaque type that is used to build geometry for the simulation.
 */
typedef struct JPS_GeometryBuilder_t* JPS_GeometryBuilder;

/**
 * Creates a new GeometryBuilder
 * @return the new builder.
 */
JUPEDSIM_API JPS_GeometryBuilder JPS_GeometryBuilder_Create();

/**
 * Adds an accessible area to the geometry.
 * The polygon described by points and pointCount must be a simple polygon. The polygon my be
 * specified in clockwise or counter-clockwise order.
 * Multiple accessible areas may overlap, the final accessible area is created by forming the union
 * over all accessible areas.
 * The Union over all accessible areas minus exclusions must form one polygon.
 * @param polygon describing the accessible area.
 * @param pointCount number of points the polygon consists of.
 */
JUPEDSIM_API void JPS_GeometryBuilder_AddAccessibleArea(
    JPS_GeometryBuilder handle,
    const JPS_Point* polygon,
    size_t lenPolygon);

/**
 * Adds an accessible area to the geometry.
 * The polygon described by points and pointCount must be a simple polygon. The polygon my be
 * specified in clockwise or counter-clockwise order.
 * Multiple exclusion areas may overlap, the final exclusion area is created by forming the union
 * over all exclusion areas.
 * Exclusion areas are subtracted from accessible areas.
 * The Union over all accessible areas minus exclusions must form a single polygon.
 * @param polygon describing the accessible area.
 * @param pointCount number of points the polygon consists of.
 */
JUPEDSIM_API void JPS_GeometryBuilder_ExcludeFromAccessibleArea(
    JPS_GeometryBuilder handle,
    const JPS_Point* polygon,
    size_t lenPolygon);

/**
 * Creates a JPS_Geometry from a JPS_GeometryBuilder. After this call the builder still has to be
 * freed with JPS_GeometryBuilder_Free.
 * @param handle to operate on.
 * @param[out] errorMessage if not NULL: will be set to a JPS_ErrorMessage in case of an error.
 * @return a JPS_Geometry handle on success or NULL on any error.
 */
JUPEDSIM_API JPS_Geometry
JPS_GeometryBuilder_Build(JPS_GeometryBuilder handle, JPS_ErrorMessage* errorMessage);

/**
 * Frees a JPS_GeometryBuilder.
 * @param handle to the JPS_GeometryBuilder to free.
 */
JUPEDSIM_API void JPS_GeometryBuilder_Free(JPS_GeometryBuilder handle);

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

/**
 * Opaque type of Generalized Centrifugal Force model state
 */
typedef struct JPS_GeneralizedCentrifugalForceModelState_t*
    JPS_GeneralizedCentrifugalForceModelState;

/**
 * Read speed of this agent.
 * @param handle of the Agent to access.
 * @return speed in m/s
 */
JUPEDSIM_API double JPS_GeneralizedCentrifugalForceModelState_GetSpeed(
    JPS_GeneralizedCentrifugalForceModelState handle);

/**
 * Write speed of this agent.
 * @param handle of the Agent to access.
 * @param speed in m/s
 */
JUPEDSIM_API void JPS_GeneralizedCentrifugalForceModelState_SetSpeed(
    JPS_GeneralizedCentrifugalForceModelState handle,
    double speed);

/**
 * Read desired orientation of this agent.
 * @param handle of the Agent to access.
 * @return 2D orientation vector
 */
JUPEDSIM_API JPS_Point
JPS_GeneralizedCentrifugalForceModelState_GetE0(JPS_GeneralizedCentrifugalForceModelState handle);

/**
 * Write desired direction of this agent.
 * @param handle of the Agent to access.
 * @param e0 desired orientation of this agent.
 */
JUPEDSIM_API void JPS_GeneralizedCentrifugalForceModelState_SetE0(
    JPS_GeneralizedCentrifugalForceModelState handle,
    JPS_Point e0);

/**
 * Read mass in Kg of this agent.
 * @param handle of the Agent to access.
 * @return mass (Kg) of this agent
 */
JUPEDSIM_API double
JPS_GeneralizedCentrifugalForceModelState_GetMass(JPS_GeneralizedCentrifugalForceModelState handle);

/**
 * Write mass in Kg of this agent.
 * @param handle of the Agent to access.
 * @param mass (Kg) of this agent.
 */
JUPEDSIM_API void JPS_GeneralizedCentrifugalForceModelState_SetMass(
    JPS_GeneralizedCentrifugalForceModelState handle,
    double mass);

/**
 * Read tau of this agent.
 * @param handle of the Agent to access.
 * @return tau of this agent
 */
JUPEDSIM_API double
JPS_GeneralizedCentrifugalForceModelState_GetTau(JPS_GeneralizedCentrifugalForceModelState handle);

/**
 * Write tau of this agent.
 * @param handle of the Agent to access.
 * @param tau of this agent.
 */
JUPEDSIM_API void JPS_GeneralizedCentrifugalForceModelState_SetTau(
    JPS_GeneralizedCentrifugalForceModelState handle,
    double tau);

/**
 * Read v0 of this agent.
 * @param handle of the Agent to access.
 * @return v0 of this agent
 */
JUPEDSIM_API double
JPS_GeneralizedCentrifugalForceModelState_GetV0(JPS_GeneralizedCentrifugalForceModelState handle);

/**
 * Write v0 of this agent.
 * @param handle of the Agent to access.
 * @param v0 of this agent.
 */
JUPEDSIM_API void JPS_GeneralizedCentrifugalForceModelState_SetV0(
    JPS_GeneralizedCentrifugalForceModelState handle,
    double v0);

/**
 * Read a_v of this agent.
 * @param handle of the Agent to access.
 * @return AV of this agent
 */
JUPEDSIM_API double
JPS_GeneralizedCentrifugalForceModelState_GetAV(JPS_GeneralizedCentrifugalForceModelState handle);

/**
 * Write a_v of this agent.
 * @param handle of the Agent to access.
 * @param a_v of this agent.
 */
JUPEDSIM_API void JPS_GeneralizedCentrifugalForceModelState_SetAV(
    JPS_GeneralizedCentrifugalForceModelState handle,
    double a_v);

/**
 * Read a_min of this agent.
 * @param handle of the Agent to access.
 * @return a_min of this agent
 */
JUPEDSIM_API double
JPS_GeneralizedCentrifugalForceModelState_GetAMin(JPS_GeneralizedCentrifugalForceModelState handle);

/**
 * Write a_min of this agent.
 * @param handle of the Agent to access.
 * @param a_min of this agent.
 */
JUPEDSIM_API void JPS_GeneralizedCentrifugalForceModelState_SetAMin(
    JPS_GeneralizedCentrifugalForceModelState handle,
    double a_min);

/**
 * Read b_min of this agent.
 * @param handle of the Agent to access.
 * @return b_min of this agent
 */
JUPEDSIM_API double
JPS_GeneralizedCentrifugalForceModelState_GetBMin(JPS_GeneralizedCentrifugalForceModelState handle);

/**
 * Write b_min of this agent.
 * @param handle of the Agent to access.
 * @param b_min of this agent.
 */
JUPEDSIM_API void JPS_GeneralizedCentrifugalForceModelState_SetBMin(
    JPS_GeneralizedCentrifugalForceModelState handle,
    double b_min);

/**
 * Read b_max of this agent.
 * @param handle of the Agent to access.
 * @return b_max of this agent
 */
JUPEDSIM_API double
JPS_GeneralizedCentrifugalForceModelState_GetBMax(JPS_GeneralizedCentrifugalForceModelState handle);

/**
 * Write b_max of this agent.
 * @param handle of the Agent to access.
 * @param a_min of this agent.
 */
JUPEDSIM_API void JPS_GeneralizedCentrifugalForceModelState_SetBMax(
    JPS_GeneralizedCentrifugalForceModelState handle,
    double b_max);

/**
 * Opaque type of Collision Free Speed model state
 */
typedef struct JPS_CollisionFreeSpeedModelState_t* JPS_CollisionFreeSpeedModelState;

/**
 * Read e0 of this agent.
 * @param handle of the Agent to access.
 * @return e0 of this agent
 */
JUPEDSIM_API JPS_Point
JPS_CollisionFreeSpeedModelState_GetE0(JPS_CollisionFreeSpeedModelState handle);

/**
 * Write e0 of this agent.
 * @param handle of the Agent to access.
 * @param e0 of this agent.
 */
JUPEDSIM_API void
JPS_CollisionFreeSpeedModelState_SetE0(JPS_CollisionFreeSpeedModelState handle, JPS_Point e0);

/**
 * Read time gap of this agent.
 * @param handle of the Agent to access.
 * @return time gap of this agent
 */
JUPEDSIM_API double
JPS_CollisionFreeSpeedModelState_GetTimeGap(JPS_CollisionFreeSpeedModelState handle);

/**
 * Write time gap of this agent.
 * @param handle of the Agent to access.
 * @param time_gap of this agent.
 */
JUPEDSIM_API void JPS_CollisionFreeSpeedModelState_SetTimeGap(
    JPS_CollisionFreeSpeedModelState handle,
    double time_gap);

/**
 * Read tau of this agent.
 * @param handle of the Agent to access.
 * @return tau of this agent
 */
JUPEDSIM_API double
JPS_CollisionFreeSpeedModelState_GetTau(JPS_CollisionFreeSpeedModelState handle);

/**
 * Write tau of this agent.
 * @param handle of the Agent to access.
 * @param tau of this agent.
 */
JUPEDSIM_API void
JPS_CollisionFreeSpeedModelState_SetTau(JPS_CollisionFreeSpeedModelState handle, double tau);

/**
 * Read v0 of this agent.
 * @param handle of the Agent to access.
 * @return v0 of this agent
 */
JUPEDSIM_API double JPS_CollisionFreeSpeedModelState_GetV0(JPS_CollisionFreeSpeedModelState handle);

/**
 * Write v0 of this agent.
 * @param handle of the Agent to access.
 * @param v0 of this agent.
 */
JUPEDSIM_API void
JPS_CollisionFreeSpeedModelState_SetV0(JPS_CollisionFreeSpeedModelState handle, double v0);

/**
 * Read radius of this agent.
 * @param handle of the Agent to access.
 * @return radius of this agent
 */
JUPEDSIM_API double
JPS_CollisionFreeSpeedModelState_GetRadius(JPS_CollisionFreeSpeedModelState handle);

/**
 * Write radius of this agent in meters.
 * @param handle of the Agent to access.
 * @param radius (m) of this agent.
 */
JUPEDSIM_API void
JPS_CollisionFreeSpeedModelState_SetRadius(JPS_CollisionFreeSpeedModelState handle, double radius);

/**
 * Opaque type of Collision Free Speed V2 model state
 */
typedef struct JPS_CollisionFreeSpeedModelV2State_t* JPS_CollisionFreeSpeedModelV2State;

/**
 * Read strength neighbor repulsion of this agent.
 * @param handle of the Agent to access.
 * @return strength neighbor repulsion of this agent
 */
JUPEDSIM_API double JPS_CollisionFreeSpeedModelV2State_GetStrengthNeighborRepulsion(
    JPS_CollisionFreeSpeedModelV2State handle);

/**
 * Write strength neighbor repulsion of this agent.
 * @param handle of the Agent to access.
 * @param strengthNeighborRepulsion of this agent.
 */
JUPEDSIM_API void JPS_CollisionFreeSpeedModelV2State_SetStrengthNeighborRepulsion(
    JPS_CollisionFreeSpeedModelV2State handle,
    double strengthNeighborRepulsion);

/**
 * Read range neighbor repulsion of this agent.
 * @param handle of the Agent to access.
 * @return range neighbor repulsion of this agent
 */
JUPEDSIM_API double JPS_CollisionFreeSpeedModelV2State_GetRangeNeighborRepulsion(
    JPS_CollisionFreeSpeedModelV2State handle);

/**
 * Write range neighbor repulsion of this agent.
 * @param handle of the Agent to access.
 * @param rangeNeighborRepulsion of this agent.
 */
JUPEDSIM_API void JPS_CollisionFreeSpeedModelV2State_SetRangeNeighborRepulsion(
    JPS_CollisionFreeSpeedModelV2State handle,
    double rangeNeighborRepulsion);

/**
 * Read strength geometry repulsion of this agent.
 * @param handle of the Agent to access.
 * @return strength geometry repulsion of this agent
 */
JUPEDSIM_API double JPS_CollisionFreeSpeedModelV2State_GetStrengthGeometryRepulsion(
    JPS_CollisionFreeSpeedModelV2State handle);

/**
 * Write strength geometry repulsion of this agent.
 * @param handle of the Agent to access.
 * @param strengthGeometryRepulsion of this agent.
 */
JUPEDSIM_API void JPS_CollisionFreeSpeedModelV2State_SetStrengthGeometryRepulsion(
    JPS_CollisionFreeSpeedModelV2State handle,
    double strengthGeometryRepulsion);

/**
 * Read range geometry repulsion of this agent.
 * @param handle of the Agent to access.
 * @return range geometry repulsion of this agent
 */
JUPEDSIM_API double JPS_CollisionFreeSpeedModelV2State_GetRangeGeometryRepulsion(
    JPS_CollisionFreeSpeedModelV2State handle);

/**
 * Write strength neighbor repulsion of this agent.
 * @param handle of the Agent to access.
 * @param rangeGeometryRepulsion of this agent.
 */
JUPEDSIM_API void JPS_CollisionFreeSpeedModelV2State_SetRangeGeometryRepulsion(
    JPS_CollisionFreeSpeedModelV2State handle,
    double rangeGeometryRepulsion);

/**
 * Read e0 of this agent.
 * @param handle of the Agent to access.
 * @return e0 of this agent
 */
JUPEDSIM_API JPS_Point
JPS_CollisionFreeSpeedModelV2State_GetE0(JPS_CollisionFreeSpeedModelV2State handle);

/**
 * Write e0 of this agent.
 * @param handle of the Agent to access.
 * @param e0 of this agent.
 */
JUPEDSIM_API void
JPS_CollisionFreeSpeedModelV2State_SetE0(JPS_CollisionFreeSpeedModelV2State handle, JPS_Point e0);

/**
 * Read time gap of this agent.
 * @param handle of the Agent to access.
 * @return time gap of this agent
 */
JUPEDSIM_API double
JPS_CollisionFreeSpeedModelV2State_GetTimeGap(JPS_CollisionFreeSpeedModelV2State handle);

/**
 * Write time gap of this agent.
 * @param handle of the Agent to access.
 * @param time_gap of this agent.
 */
JUPEDSIM_API void JPS_CollisionFreeSpeedModelV2State_SetTimeGap(
    JPS_CollisionFreeSpeedModelV2State handle,
    double time_gap);

/**
 * Read tau of this agent.
 * @param handle of the Agent to access.
 * @return tau of this agent
 */
JUPEDSIM_API double
JPS_CollisionFreeSpeedModelV2State_GetTau(JPS_CollisionFreeSpeedModelV2State handle);

/**
 * Write tau of this agent.
 * @param handle of the Agent to access.
 * @param tau of this agent.
 */
JUPEDSIM_API void
JPS_CollisionFreeSpeedModelV2State_SetTau(JPS_CollisionFreeSpeedModelV2State handle, double tau);

/**
 * Read v0 of this agent.
 * @param handle of the Agent to access.
 * @return v0 of this agent
 */
JUPEDSIM_API double
JPS_CollisionFreeSpeedModelV2State_GetV0(JPS_CollisionFreeSpeedModelV2State handle);

/**
 * Write v0 of this agent.
 * @param handle of the Agent to access.
 * @param v0 of this agent.
 */
JUPEDSIM_API void
JPS_CollisionFreeSpeedModelV2State_SetV0(JPS_CollisionFreeSpeedModelV2State handle, double v0);

/**
 * Read radius of this agent.
 * @param handle of the Agent to access.
 * @return radius of this agent
 */
JUPEDSIM_API double
JPS_CollisionFreeSpeedModelV2State_GetRadius(JPS_CollisionFreeSpeedModelV2State handle);

/**
 * Write radius of this agent in meters.
 * @param handle of the Agent to access.
 * @param radius (m) of this agent.
 */
JUPEDSIM_API void JPS_CollisionFreeSpeedModelV2State_SetRadius(
    JPS_CollisionFreeSpeedModelV2State handle,
    double radius);

/**
 * Identifies the type of stage
 */
enum JPS_StageType { JPS_NotifiableQueueType, JPS_WaitingSetType, JPS_WaypointType, JPS_ExitType };

/**
 * Opaque type of an NotifiableQueueProxy
 */
typedef struct JPS_NotifiableQueueProxy_t* JPS_NotifiableQueueProxy;

JUPEDSIM_API size_t JPS_NotifiableQueueProxy_GetCountTargeting(JPS_NotifiableQueueProxy handle);

JUPEDSIM_API size_t JPS_NotifiableQueueProxy_GetCountEnqueued(JPS_NotifiableQueueProxy handle);

JUPEDSIM_API void JPS_NotifiableQueueProxy_Pop(JPS_NotifiableQueueProxy handle, size_t count);

JUPEDSIM_API size_t
JPS_NotifiableQueueProxy_GetEnqueued(JPS_NotifiableQueueProxy handle, const JPS_AgentId** data);

JUPEDSIM_API void JPS_NotifiableQueueProxy_Free(JPS_NotifiableQueueProxy handle);

enum JPS_WaitingSetState { JPS_WaitingSet_Active, JPS_WaitingSet_Inactive };
typedef struct JPS_WaitingSetProxy_t* JPS_WaitingSetProxy;

JUPEDSIM_API size_t JPS_WaitingSetProxy_GetCountTargeting(JPS_WaitingSetProxy handle);

JUPEDSIM_API size_t JPS_WaitingSetProxy_GetCountWaiting(JPS_WaitingSetProxy handle);

JUPEDSIM_API size_t
JPS_WaitingSetProxy_GetWaiting(JPS_WaitingSetProxy handle, const JPS_AgentId** data);

JUPEDSIM_API void
JPS_WaitingSetProxy_SetWaitingSetState(JPS_WaitingSetProxy handle, JPS_WaitingSetState newState);

JUPEDSIM_API JPS_WaitingSetState JPS_WaitingSetProxy_GetWaitingSetState(JPS_WaitingSetProxy handle);

JUPEDSIM_API void JPS_WaitingSetProxy_Free(JPS_WaitingSetProxy handle);

typedef struct JPS_WaypointProxy_t* JPS_WaypointProxy;

JUPEDSIM_API size_t JPS_WaypointProxy_GetCountTargeting(JPS_WaypointProxy handle);

JUPEDSIM_API void JPS_WaypointProxy_Free(JPS_WaypointProxy handle);

typedef struct JPS_ExitProxy_t* JPS_ExitProxy;

JUPEDSIM_API size_t JPS_ExitProxy_GetCountTargeting(JPS_ExitProxy handle);

JUPEDSIM_API void JPS_ExitProxy_Free(JPS_ExitProxy handle);

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
 * Access Collision Free Speed model V2 state.
 * Precondition: Agent needs to use Collision Free Speed model V2
 * @param handle of the agent to access.
 * @param[out] errorMessage if not NULL: will be set to a JPS_ErrorMessage in case of an error.
 * @return state or NULL on error
 */
JUPEDSIM_API JPS_CollisionFreeSpeedModelV2State
JPS_Agent_GetCollisionFreeSpeedModelV2State(JPS_Agent handle, JPS_ErrorMessage* errorMessage);

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
 * Describes parameters of an Agent in GeneralizedCentrifugalForceModel
 */
typedef struct JPS_GeneralizedCentrifugalForceModelAgentParameters {
    /**
     * Initial speed of the Agent
     */
    double speed = 0.0;
    /**
     * Desired orientation.
     */
    JPS_Point e0{0, 0};
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
     * Mass of the agent
     */
    double mass = 1;
    /**
     * Tau of the agent
     */
    double tau = 0.5;
    /**
     * V0 of the agent
     */
    double v0 = 1.2;
    /**
     * a_v sagital axis stretch factor
     */
    double a_v = 1.;
    /**
     * b_min minimum length of transversal axis in 'meters'
     */
    double a_min = 0.2;
    /**
     * a_min minimum length of sagital axis in 'meters'
     */
    double b_min = 0.2;
    /**
     * b_max maximum length of transversal axis in 'meters'
     */
    double b_max = 0.4;
} JPS_GeneralizedCentrifugalForceModelAgentParameters;

/**
 * Describes parameters of an Agent in GeneralizedCentrifugalForceModel
 */
typedef struct JPS_CollisionFreeSpeedModelAgentParameters {
    /**
     * Position of the agent.
     * The position needs to inside the accessible area.
     */
    JPS_Point position{0, 0};
    /**
     * Defines the journey this agent will take use
     */
    JPS_JourneyId journeyId = 0;
    /**
     * Defines the current stage of its journey
     */
    JPS_StageId stageId = 0;
    /**
     * @param time_gap of the agents using this profile (T in the OV-function)
     */
    double time_gap = 1.;
    /**
     *@param v0 of the agents using this profile(desired speed) double radius;
     */
    double v0 = 1.2;
    /**
     *@param radius of the agent in 'meters'
     */
    double radius = 0.2;
} JPS_CollisionFreeSpeedModelAgentParameters;

/**
 * Describes parameters of an Agent in Collision Free Speed Model V2
 */
typedef struct JPS_CollisionFreeSpeedModelV2AgentParameters {
    /**
     * Position of the agent.
     * The position needs to inside the accessible area.
     */
    JPS_Point position{0, 0};
    /**
     * Defines the journey this agent will take use
     */
    JPS_JourneyId journeyId = 0;
    /**
     * Defines the current stage of its journey
     */
    JPS_StageId stageId = 0;

    /**
     * @param time_gap of the agents using this profile (T in the OV-function)
     */
    double time_gap = 1.;
    /**
     *@param v0 of the agents using this profile(desired speed) double radius;
     */
    double v0 = 1.2;
    /**
     *@param radius of the agent in 'meters'
     */
    double radius = 0.2;

    /**
     *  Strength of the repulsion from neighbors
     */
    double strengthNeighborRepulsion{8.0};

    /**
     * Range of the repulsion from neighbors
     */
    double rangeNeighborRepulsion{0.1};

    /**
     * Strength of the repulsion from geometry boundaries
     */
    double strengthGeometryRepulsion{5.0};

    /**
     * Range of the repulsion from geometry boundaries
     */
    double rangeGeometryRepulsion{0.02};

} JPS_CollisionFreeSpeedModelV2AgentParameters;

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

/*
 * Opaque type to a Simulator object.
 */
typedef struct JPS_Simulation_t* JPS_Simulation;

/*
 * Creates a new JPS_Simulation object.
 * NOTE: JPS_Simulation_Create will take ownership of all indicated parameters even in case an error
 * occured.
 * @param model to use. Will copy 'model', 'model' can be freed after this call or reused for
 * another simulation.
 * @param geometry to use. Will copy 'geometry', 'geometry' can be freed after this call or reused
 * for another simulation.
 * @param dT simulation timestep in seconds
 * @param[out] errorMessage if not NULL: will be set to a JPS_ErrorMessage in case of an error.
 * @return the Simulation
 */
JUPEDSIM_API JPS_Simulation JPS_Simulation_Create(
    JPS_OperationalModel model,
    JPS_Geometry geometry,
    double dT,
    JPS_ErrorMessage* errorMessage);

/**
 * Populates the Simulation with a Journey for agents to use.
 * @param handle to the simulation to act on
 * @param journey to add. Will copy 'journey', 'journey' needs to be freed after this call.
 * @param[out] errorMessage if not NULL: will be set to a JPS_ErrorMessage in case of an error.
 * @return Id of the journey
 */
JUPEDSIM_API JPS_JourneyId JPS_Simulation_AddJourney(
    JPS_Simulation handle,
    JPS_JourneyDescription journey,
    JPS_ErrorMessage* errorMessage);

/**
 * Adds a new waypoint stage to the simulation.
 * @param handle to the simulation to act on
 * @param position of the waypoiny
 * @parma required distance to check if the waypoint was reached
 * @param[out] errorMessage if not NULL: will be set to a JPS_ErrorMessage in case of an error.
 * @return Id of the stage
 */
JUPEDSIM_API JPS_StageId JPS_Simulation_AddStageWaypoint(
    JPS_Simulation handle,
    JPS_Point position,
    double distance,
    JPS_ErrorMessage* errorMessage);

/**
 * Adds a new exit stage to the simulation.
 * @param handle to the simulation to act on
 * @param polygon describing the exit
 * @param number of points in the polygon
 * @param[out] errorMessage if not NULL: will be set to a JPS_ErrorMessage in case of an error.
 * @return Id of the stage
 */
JUPEDSIM_API JPS_StageId JPS_Simulation_AddStageExit(
    JPS_Simulation handle,
    const JPS_Point* polygon,
    size_t len_polygon,
    JPS_ErrorMessage* errorMessage);

/**
 * Adds a new notifiable queue stage to the simulation.
 * The waiting positions supplied are the queue positions. The first point
 * defines the head of the queue and the last point defines the tail of the
 * queue. When there are more agents waiting in the queue than there are waiting
 * slots the overflowing agents will all try to wait at the tail end of the
 * queue.
 *
 * Agents can be popped from the queue by calling
 * 'JPS_Simulation_PopAgentsFromQueue'
 *
 * @param handle to the simulation to act on
 * @param waiting_positions of this queue
 * @param number of waiting positions
 * @param[out] errorMessage if not NULL: will be set to a JPS_ErrorMessage in case of an error.
 * @return Id of the stage
 */
JUPEDSIM_API JPS_StageId JPS_Simulation_AddStageNotifiableQueue(
    JPS_Simulation handle,
    const JPS_Point* waiting_positions,
    size_t len_waiting_positions,
    JPS_ErrorMessage* errorMessage);

/**
 * Adds a new notifiable waiting set to the simulation.
 * The waiting positions supplied are filled in order.
 *
 * A waiting set can be set active or inactive with
 * `JPS_Simulation_ChangeWaitingSetState` When active agents will wait in the
 * waiting set until the stage is set inactive. When inactive the stage
 * functions as a waypoint at the first position supplied.
 *
 * @param handle to the simulation to act on
 * @param waiting_positions of this waiting set
 * @param number of waiting positions
 * @param[out] errorMessage if not NULL: will be set to a JPS_ErrorMessage in case of an error.
 * @return Id of the stage
 */
JUPEDSIM_API JPS_StageId JPS_Simulation_AddStageWaitingSet(
    JPS_Simulation handle,
    const JPS_Point* waiting_positions,
    size_t len_waiting_positions,
    JPS_ErrorMessage* errorMessage);

/**
 * Adds a new agent to the simulation.
 * This can be called at any time, i.e. agents can be added at any iteration.
 * NOTE: Currently there is no checking done to ensure the agent can be placed at the desired
 * location.
 * @param handle to the simulation to act on
 * @param parameters describing the new agent.
 * @param[out] errorMessage if not NULL. Will contain address of JPS_ErrorMessage in case of an
 * error.
 * @return id of the new agent or 0 if the agent could not be added due to an error.
 */
JUPEDSIM_API JPS_AgentId JPS_Simulation_AddGeneralizedCentrifugalForceModelAgent(
    JPS_Simulation handle,
    JPS_GeneralizedCentrifugalForceModelAgentParameters parameters,
    JPS_ErrorMessage* errorMessage);

/**
 * Adds a new agent to the simulation.
 * This can be called at any time, i.e. agents can be added at any iteration.
 * NOTE: Currently there is no checking done to ensure the agent can be placed at the desired
 * location.
 * @param handle to the simulation to act on
 * @param parameters describing the new agent.
 * @param[out] errorMessage if not NULL. Will contain address of JPS_ErrorMessage in case of an
 * error.
 * @return id of the new agent or 0 if the agent could not be added due to an error.
 */
JUPEDSIM_API JPS_AgentId JPS_Simulation_AddCollisionFreeSpeedModelAgent(
    JPS_Simulation handle,
    JPS_CollisionFreeSpeedModelAgentParameters parameters,
    JPS_ErrorMessage* errorMessage);

/**
 * Adds a new agent to the simulation.
 * This can be called at any time, i.e. agents can be added at any iteration.
 * NOTE: Currently there is no checking done to ensure the agent can be placed at the desired
 * location.
 * @param handle to the simulation to act on
 * @param parameters describing the new agent.
 * @param[out] errorMessage if not NULL. Will contain address of JPS_ErrorMessage in case of an
 * error.
 * @return id of the new agent or 0 if the agent could not be added due to an error.
 */
JUPEDSIM_API JPS_AgentId JPS_Simulation_AddCollisionFreeSpeedModelV2Agent(
    JPS_Simulation handle,
    JPS_CollisionFreeSpeedModelV2AgentParameters parameters,
    JPS_ErrorMessage* errorMessage);

/**
 * Marks an agent from the simuation for removal.
 * The agent will be removed at the start of the next simulation iteration, before the interaction
 * are computed.
 * @param handle to the simulation to act on
 * @param agentID to remove
 * @param[out] errorMessage if not NULL: will be set to a JPS_ErrorMessage in case of an error.
 * @return bool true if the agent existed and was marked for removal otherwise false
 */
JUPEDSIM_API bool JPS_Simulation_MarkAgentForRemoval(
    JPS_Simulation handle,
    JPS_AgentId agentId,
    JPS_ErrorMessage* errorMessage);

/*
 * Returns the ids of all agents that exited the simulation in the last iteration.
 * @param handle of the Simulation
 * @param[out] data pointer to the ids
 * @return numer of agents that have been removed in the last iteration
 */
JUPEDSIM_API size_t JPS_Simulation_RemovedAgents(JPS_Simulation handle, const JPS_AgentId** data);

/*
 * Advances the simulation by one step.
 * @param handle of the Simulation
 * @param[out] errorMessage if not NULL: will be set to a JPS_ErrorMessage in case of an error.
 * @return true if no errors occured
 */
JUPEDSIM_API bool JPS_Simulation_Iterate(JPS_Simulation handle, JPS_ErrorMessage* errorMessage);

/**
 * How many agents are in the simulation.
 * @param handle of the simulation
 * @return count agents in the simulation
 */
JUPEDSIM_API size_t JPS_Simulation_AgentCount(JPS_Simulation handle);

/**
 * Returns time progressed in the simulation.
 * @param handle of the simulation
 * @return seconds elapsed
 */
JUPEDSIM_API double JPS_Simulation_ElapsedTime(JPS_Simulation handle);

/**
 * Returns time simulad per iteration call.
 * @param handle of the simulation
 * @return time simulated per iteration call in seconds
 */
JUPEDSIM_API double JPS_Simulation_DeltaTime(JPS_Simulation handle);

/**
 * Returns how many iterations have been simulated.
 * @param handle of the simulation
 * @return count of elapsed iterations
 */
JUPEDSIM_API uint64_t JPS_Simulation_IterationCount(JPS_Simulation handle);

/**
 * Returns an iterator over all agents in the simulation.
 * Notes:
 *   The iterator will be invalidated once JPS_Simulation_Iterate is called.
 *   The iterator needs to be freed after use.
 * @param handle of the simulation
 * @return iterator over agents in the simulation
 */
JUPEDSIM_API JPS_AgentIterator JPS_Simulation_AgentIterator(JPS_Simulation handle);

/**
 * Returns a specific agent of the simulation.
 * @param handle of the simulation
 * @param agentId Id of the agent to get
 * @return Agent with given Id
 */
JUPEDSIM_API JPS_Agent
JPS_Simulation_GetAgent(JPS_Simulation handle, JPS_AgentId agentId, JPS_ErrorMessage* errorMessage);

/**
 * Switches the journey and currently selected stage of this agent
 * @param handle of the Simulation to operate on
 * @param agentId id of the agent to modify
 * @param journeyId of the journey to select
 * @param stageId of the stage to select
 * @param[out] errorMessage if not NULL: will be set to a JPS_ErrorMessage in case of an error.
 * @return true on success, false on any error, e.g. unknown agent id or profile id
 */
JUPEDSIM_API bool JPS_Simulation_SwitchAgentJourney(
    JPS_Simulation handle,
    JPS_AgentId agentId,
    JPS_JourneyId journeyId,
    JPS_StageId stageId,
    JPS_ErrorMessage* errorMessage);

/**
 * Query the pedestrian model used by this simulation.
 * @return the type of pedestrian model used in this simulation instance.
 */
JUPEDSIM_API JPS_ModelType JPS_Simulation_ModelType(JPS_Simulation handle);

/**
 * Query the simulation for all agent ids in distance to this point;
 * @param handle of the Simulation to operate on
 * @param position to query
 * @param distance around position
 * @return iterator containing all agent ids in range
 */
JUPEDSIM_API JPS_AgentIdIterator
JPS_Simulation_AgentsInRange(JPS_Simulation handle, JPS_Point position, double distance);

/**
 * Query the simulation for all agent ids in the supplied polygon.
 * @param handle of the Simulation to operate on
 * @param polygon array of JPS_Points (CCW ordered convex polygon)
 * @param len_polygon number of points in the polygon
 * @return iterator over the Ids inside the polygon
 */
JUPEDSIM_API JPS_AgentIdIterator
JPS_Simulation_AgentsInPolygon(JPS_Simulation handle, const JPS_Point* polygon, size_t len_polygon);

JUPEDSIM_API JPS_StageType JPS_Simulation_GetStageType(JPS_Simulation handle, JPS_StageId id);

JUPEDSIM_API JPS_NotifiableQueueProxy JPS_Simulation_GetNotifiableQueueProxy(
    JPS_Simulation handle,
    JPS_StageId stageId,
    JPS_ErrorMessage* errorMessage);

JUPEDSIM_API JPS_WaitingSetProxy JPS_Simulation_GetWaitingSetProxy(
    JPS_Simulation handle,
    JPS_StageId stageId,
    JPS_ErrorMessage* errorMessage);

JUPEDSIM_API JPS_WaypointProxy JPS_Simulation_GetWaypointProxy(
    JPS_Simulation handle,
    JPS_StageId stageId,
    JPS_ErrorMessage* errorMessage);

JUPEDSIM_API JPS_ExitProxy JPS_Simulation_GetExitProxy(
    JPS_Simulation handle,
    JPS_StageId stageId,
    JPS_ErrorMessage* errorMessage);

/**
 * Enable / disable  collection of performance data.
 * @param handle of the Simulation to operate on
 * @param status new status to set
 */
JUPEDSIM_API void JPS_Simulation_SetTracing(JPS_Simulation handle, bool status);

/**
 * Read trace data from alst iteration. If tracing is disable all timings will be zero.
 * @param handle of the Simulation to operate on
 * @return trace data
 */
JUPEDSIM_API JPS_Trace JPS_Simulation_GetTrace(JPS_Simulation handle);

/**
 * Gain read access to the geometry used by this simulation.
 * @param handle of the Simulation to operate on
 * @return the geometry. Do not call JPS_Geometry_Free on this handle,
 * the geometry is still owned by the simulation!
 */
JUPEDSIM_API JPS_Geometry JPS_Simulation_GetGeometry(JPS_Simulation handle);

/**
 * Switch the geometry in which the agent move during the simulation.
 * @param handle of the Simulation to operate on
 * @param geometry the new geometry to be used in the simulation.
 * @param[out] faultyAgents iterator to all agents outside the new geometry, empty if successful
 * @param[out] errorMessage if not succcessful: will be set to a JPS_ErrorMessage in case of an
 * error.
 * @return could successfully change the simulation geometry
 */
JUPEDSIM_API bool JPS_Simulation_SwitchGeometry(
    JPS_Simulation handle,
    JPS_Geometry geometry,
    JPS_AgentIdIterator* faultyAgents,
    JPS_ErrorMessage* errorMessage);

/**
 * Frees a JPS_Simulation.
 * @param handle to the JPS_Simulation to free.
 */
JUPEDSIM_API void JPS_Simulation_Free(JPS_Simulation handle);

struct JPS_Path {
    size_t len;
    const JPS_Point* points;
};

JUPEDSIM_API void JPS_Path_Free(JPS_Path* path);

struct JPS_Triangle {
    JPS_Point points[3];
};

struct JPS_TriangleMesh {
    size_t len;
    JPS_Triangle* triangles;
};

JUPEDSIM_API void JPS_TriangleMesh_Free(JPS_TriangleMesh* mesh);

struct JPS_Line {
    JPS_Point points[2];
};

struct JPS_Lines {
    size_t len;
    JPS_Line* lines;
};

JUPEDSIM_API void JPS_Lines_Free(JPS_Lines* lines);

/**
 * WARNING this is currently a NavMeshRoutingEngine! This does not account possible other types of
 * routing engines
 */
typedef struct JPS_RoutingEngine_t* JPS_RoutingEngine;

JUPEDSIM_API JPS_RoutingEngine JPS_RoutingEngine_Create(JPS_Geometry geometry);

JUPEDSIM_API JPS_Path
JPS_RoutingEngine_ComputeWaypoint(JPS_RoutingEngine handle, JPS_Point from, JPS_Point to);

JUPEDSIM_API bool JPS_RoutingEngine_IsRoutable(JPS_RoutingEngine handle, JPS_Point p);

JUPEDSIM_API JPS_TriangleMesh JPS_RoutingEngine_Mesh(JPS_RoutingEngine handle);

/// Note: Currently the ID is the index of the vertex from 'JPS_RoutingEngine_Mesh'
JUPEDSIM_API JPS_Lines JPS_RoutingEngine_EdgesFor(JPS_RoutingEngine handle, uint32_t id);

JUPEDSIM_API void JPS_RoutingEngine_Free(JPS_RoutingEngine handle);

#ifdef __cplusplus
}
#endif
