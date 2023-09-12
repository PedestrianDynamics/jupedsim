/* Copyright © 2012-2023 Forschungszentrum Jülich GmbH */
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
typedef enum JPS_ModelType { JPS_GCFMModel, JPS_VelocityModel } JPS_ModelType;

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
 * Opaque type for operational models describing how agents interact in the simulation.
 */
typedef struct JPS_OperationalModel_t* JPS_OperationalModel;

/**
 * Id of a model parameter profile.
 * Zero represents an invalid id.
 */
typedef uint64_t JPS_ModelParameterProfileId;

/**
 * Frees a JPS_OperationalModel
 * @param handle to the JPS_OperationalModel to free.
 */
JUPEDSIM_API void JPS_OperationalModel_Free(JPS_OperationalModel handle);

/**
 * Opaque type for a GCFM Model Builder
 */
typedef struct JPS_GCFMModelBuilder_t* JPS_GCFMModelBuilder;

/**
 * Creates a GCFM model builder.
 * @param nu_Ped
 * @param nu_Wall
 * @param dist_eff_Ped
 * @param dist_eff_Wall
 * @param intp_width_Ped
 * @param intp_width_Wall
 * @param maxf_Ped
 * @param maxf_Wall
 * @return the GCFM model builder
 */
JUPEDSIM_API JPS_GCFMModelBuilder JPS_GCFMModelBuilder_Create(
    double nu_Ped,
    double nu_Wall,
    double dist_eff_Ped,
    double dist_eff_Wall,
    double intp_width_Ped,
    double intp_width_Wall,
    double maxf_Ped,
    double maxf_Wall);

/**
 * Registeres a parameter profile for this model.
 * There has to be at least one model registered for the model to be considered valid.
 * @param handle of builder to operate on
 * @param id desired id of the parameter profile. If the id is already used by the model and
 *        previously added profile will be overwritten.
 * @param mass of the agents using this profile
 * @param tau of the agents using this profile
 * @param v0 of the agents using this profile
 * @param a_v sagital axis stretch factor
 * @param a_min minimum length of sagital axis in 'meters'
 * @param b_min minimum length of transversal axis in 'meters'
 * @param b_max maximum length of transversal axis in 'meters'
 */
JUPEDSIM_API void JPS_GCFMModelBuilder_AddParameterProfile(
    JPS_GCFMModelBuilder handle,
    JPS_ModelParameterProfileId id,
    double mass,
    double tau,
    double v0,
    double a_v,
    double a_min,
    double b_min,
    double b_max);

/**
 * Creates a JPS_OperationalModel of type GCFM Model from the JPS_GCFMModelBuilder.
 * @param handle the builder to operate on
 * @param[out] errorMessage if not NULL: will be set to a JPS_ErrorMessage in case of an error
 * @return a JPS_GCFMModel or NULL if an error occured.
 */
JUPEDSIM_API JPS_OperationalModel
JPS_GCFMModelBuilder_Build(JPS_GCFMModelBuilder handle, JPS_ErrorMessage* errorMessage);

/**
 * Frees a JPS_GCFMModelBuilder
 * @param handle to the JPS_GCFMModelBuilder to free.
 */
JUPEDSIM_API void JPS_GCFMModelBuilder_Free(JPS_GCFMModelBuilder handle);

/**
 * Opaque type for a Velocity Model Builder
 */
typedef struct JPS_VelocityModelBuilder_t* JPS_VelocityModelBuilder;

/**
 * Creates a Velocity Model builder.
 * @param aPed
 * @param DPed
 * @param aWall
 * @param DWall
 * @return the builder
 */
JUPEDSIM_API JPS_VelocityModelBuilder
JPS_VelocityModelBuilder_Create(double aPed, double DPed, double aWall, double DWall);

/**
 * Registeres a parameter profile for this model.
 * There has to be at least one model registered for the model to be considered valid.
 * @param handle of builder to operate on
 * @param id desired id of the parameter profile. If the id is already used by the model and
 *        previously added profile will be overwritten.
 * @param timeGap of the agents using this profile (T in the OV-function)
 * @param tau of the agents using this profile
 * @param radius of the agent in 'meters'
 * @param v0 of the agents using this profile (desired speed)
 */
JUPEDSIM_API void JPS_VelocityModelBuilder_AddParameterProfile(
    JPS_VelocityModelBuilder handle,
    JPS_ModelParameterProfileId id,
    double timeGap,
    double tau,
    double v0,
    double radius);

/**
 * Creates a JPS_OperationalModel of type Velocity Model from the JPS_GCFMModelBuilder.
 * @param handle the builder to operate on
 * @param[out] errorMessage if not NULL: will be set to a JPS_ErrorMessage in case of an error
 * @return a JPS_GCFMModel or NULL if an error occured.
 */
JUPEDSIM_API JPS_OperationalModel
JPS_VelocityModelBuilder_Build(JPS_VelocityModelBuilder handle, JPS_ErrorMessage* errorMessage);

/**
 * Frees a JPS_VelocityModelBuilder
 * @param handle to the JPS_VelocityModelBuilder to free.
 */
JUPEDSIM_API void JPS_VelocityModelBuilder_Free(JPS_VelocityModelBuilder handle);

/**
 * Opaque type that represents the geometry the simulation acts on.
 * This type is created from JPS_GeometryBuilder.
 */
typedef struct JPS_Geometry_t* JPS_Geometry;

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
 * Frees a JPS_Journey.
 * @param handle to the JPS_Journey to free.
 */
JUPEDSIM_API void JPS_JourneyDescription_Free(JPS_JourneyDescription handle);

/**
 * Opaque type of Generalized Centrifugal Force model state
 */
typedef struct JPS_GeneralizedCentrifugalForceModelState_t const*
    JPS_GeneralizedCentrifugalForceModelState;

JUPEDSIM_API double JPS_GeneralizedCentrifugalForceModelState_GetSpeed(
    JPS_GeneralizedCentrifugalForceModelState handle);

JUPEDSIM_API JPS_Point
JPS_GeneralizedCentrifugalForceModelState_GetE0(JPS_GeneralizedCentrifugalForceModelState handle);

/**
 * Opaque type of Velocity model state
 */
typedef struct JPS_VelocityModelState_t const* JPS_VelocityModelState;
JUPEDSIM_API JPS_Point JPS_VelocityModelState_GetE0(JPS_VelocityModelState handle);

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
typedef struct JPS_Agent_t const* JPS_Agent;

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
 * Access the index of the currently targeted stage in this agents journey.
 *
 * @param handle of the agent to access.
 * @return the index of the currently targeted stage in this agents journey
 */
JUPEDSIM_API JPS_StageIndex JPS_Agent_GetStageIndex(JPS_Agent handle);

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
 * Access Velocity model state.
 * Precondition: Agent needs to use Velocity model
 * @param handle of the agent to access.
 * @param[out] errorMessage if not NULL: will be set to a JPS_ErrorMessage in case of an error.
 * @return state or NULL on error
 */
JUPEDSIM_API JPS_VelocityModelState
JPS_Agent_GetVelocityModelState(JPS_Agent handle, JPS_ErrorMessage* errorMessage);

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
 * Describes parameters of an Agent in GCFMModel
 */
typedef struct JPS_GCFMModelAgentParameters {
    /**
     * Initial speed of the Agent
     */
    double speed;
    /**
     * Desired orientation.
     */
    JPS_Point e0;
    /**
     * Position of the agent.
     * The position needs to inside the accessible area.
     */
    JPS_Point position;
    /*
     * Orientation vector of the agent.
     * The orientation vector will internally be normalized.
     */
    JPS_Point orientation;
    /**
     * Defines the journey this agent will take use
     */
    JPS_JourneyId journeyId;
    /**
     * Defines the paramter profile this agents uses during the simulation
     */
    JPS_ModelParameterProfileId profileId;
    /**
     * Id of this agent.
     * If set to non zero value the simulation will use the id provided and check that no agent with
     * this id is present. Adding an agent with an already used id will result in an error.
     * If set to zero the simulation will create a unique id internally.
     */
    JPS_AgentId agentId;
} JPS_GCFMModelAgentParameters;

/**
 * Describes parameters of an Agent in GCFMModel
 */
typedef struct JPS_VelocityModelAgentParameters {
    /**
     * Desired orientation.
     */
    JPS_Point e0;
    /**
     * Position of the agent.
     * The position needs to inside the accessible area.
     */
    JPS_Point position;
    /*
     * Orientation vector of the agent.
     * The orientation vector will internally be normalized.
     */
    JPS_Point orientation;
    /**
     * Defines the journey this agent will take use
     */
    JPS_JourneyId journeyId;
    /**
     * Defines the paramter profile this agents uses during the simulation
     */
    JPS_ModelParameterProfileId profileId;
    /**
     * Id of this agent.
     * If set to non zero value the simulation will use the id provided and check that no agent with
     * this id is present. Adding an agent with an already used id will result in an error.
     * If set to zero the simulation will create a unique id internally.
     */
    JPS_AgentId agentId;
} JPS_VelocityModelAgentParameters;

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
JUPEDSIM_API JPS_AgentId JPS_Simulation_AddGCFMModelAgent(
    JPS_Simulation handle,
    JPS_GCFMModelAgentParameters parameters,
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
JUPEDSIM_API JPS_AgentId JPS_Simulation_AddVelocityModelAgent(
    JPS_Simulation handle,
    JPS_VelocityModelAgentParameters parameters,
    JPS_ErrorMessage* errorMessage);

/**
 * Removes an agent from the simuation.
 * @param handle to the simulation to act on
 * @param agentID to remove
 * @param[out] errorMessage if not NULL: will be set to a JPS_ErrorMessage in case of an error.
 * @return bool true if the agent existed and was removed otherwise false
 */
JUPEDSIM_API bool JPS_Simulation_RemoveAgent(
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
 * Switches the operational model parameter profile for an agent
 * @param handle of the Simulation to operate on
 * @param agentId id of the agent to modify
 * @param profileId to use from now on
 * @param[out] errorMessage if not NULL: will be set to a JPS_ErrorMessage in case of an error.
 * @return true on success, false on any error, e.g. unknown agent id or profile id
 */
JUPEDSIM_API bool JPS_Simulation_SwitchAgentProfile(
    JPS_Simulation handle,
    JPS_AgentId agentId,
    JPS_ModelParameterProfileId profileId,
    JPS_ErrorMessage* errorMessage);

/**
 * Switches the journey and currently selected stage of this agent
 * @param handle of the Simulation to operate on
 * @param agentId id of the agent to modify
 * @param journeyId of the journey to select
 * @param stageIdx of the stage to select
 * @param[out] errorMessage if not NULL: will be set to a JPS_ErrorMessage in case of an error.
 * @return true on success, false on any error, e.g. unknown agent id or profile id
 */
JUPEDSIM_API bool JPS_Simulation_SwitchAgentJourney(
    JPS_Simulation handle,
    JPS_AgentId agentId,
    JPS_JourneyId journeyId,
    JPS_StageIndex stageIdx,
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
