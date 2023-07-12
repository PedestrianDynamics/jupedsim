/* Copyright © 2012-2022 Forschungszentrum Jülich GmbH */
/* SPDX-License-Identifier: LGPL-3.0-or-later */
#pragma once

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
    const char* git_commit_hash;
    const char* git_commit_date;
    const char* git_branch;
    const char* compiler;
    const char* compiler_version;
} JPS_BuildInfo;

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
    /** x component in 'meters' */
    double x;
    /** y component in 'meters' */
    double y;
} JPS_Point;

/**
 * Describes a waypoint.
 */
typedef struct JPS_Waypoint {
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
    JPS_Point* polygon,
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
    JPS_Point* polygon,
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
 * Extends the journey with a waypoint.
 * @param handle of the journey to extend.
 * @param position of the waypoint.
 * @param distance to the position to count this point as visited. Needs to be >= 0.
 * @param[out] stageIndex if not NULL: will be set to the stage index of this stage.
 * @param[out] errorMessage if not NULL: will be set to a JPS_ErrorMessage in case of an error.
 * @return sucess if an waypoint could be added to the journey.
 */
JUPEDSIM_API bool JPS_JourneyDescription_AddWaypoint(
    JPS_JourneyDescription handle,
    JPS_Point position,
    double distance,
    JPS_StageIndex* stageIndex,
    JPS_ErrorMessage* errorMessage);

/**
 * Extends the journey with an exit waypoint. When the agent enters the defined polygon and is
 * currently navigating towards this stage(!), the argent will be marked for removal an is removed
 * at the beginning of the next iteration.
 * @param handle of the journey to extend.
 * @param polygon A CCW convex polygon describing the exit area. Note that agents will move towards
 * the centroid of this area. Do not repeat the first point.
 * @param len_polygon, number of points in the polygon.
 * @param[out] stageIndex if not NULL: will be set to the stage index of this stage.
 * @param[out] errorMessage if not NULL: will be set to a JPS_ErrorMessage in case of an error.
 * @return sucess if an Exit could be added to the journey.
 */
JUPEDSIM_API bool JPS_JourneyDescription_AddExit(
    JPS_JourneyDescription handle,
    JPS_Point* polygon,
    size_t len_polygon,
    JPS_StageIndex* stageIndex,
    JPS_ErrorMessage* errorMessage);

/**
 * Extends the journey with a notifiable waiting set. The waiting set consists of a ordered list of
 * points. Agents waiting will steer towards the first empty slot in this list.
 * The waiting set has to contain at least 1 waiting point.
 * @param handle of the journey to extend.
 * @param waiting_points the ordered waiting points
 * @param len_waiting_points number of waiting points
 * @param[out] stageIndex if not NULL: will be set to the stage index of this stage.
 * @param[out] errorMessage if not NULL: will be set to a JPS_ErrorMessage in case of an error.
 * @return success if the stage could be added.
 */
JUPEDSIM_API bool JPS_JourneyDescription_AddNotifiableWaitingSet(
    JPS_JourneyDescription handle,
    JPS_Point* waiting_points,
    size_t len_waiting_points,
    JPS_StageIndex* stageIndex,
    JPS_ErrorMessage* errorMessage);

/**
 * Extends the journey with a notifiable queue. The queue consists of a ordered list of
 * points. Agents waiting will steer towards the first empty slot in this list.
 * The waiting set has to contain at least 1 waiting point.
 * @param handle of the journey to extend.
 * @param waiting_points the ordered waiting points
 * @param len_waiting_points number of waiting points
 * @param[out] stageIndex if not NULL: will be set to the stage index of this stage.
 * @param[out] errorMessage if not NULL: will be set to a JPS_ErrorMessage in case of an error.
 * @return success if the stage could be added.
 */
JUPEDSIM_API bool JPS_JourneyDescription_AddNotifiableQueue(
    JPS_JourneyDescription handle,
    JPS_Point* waiting_points,
    size_t len_waiting_points,
    JPS_StageIndex* stageIndex,
    JPS_ErrorMessage* errorMessage);

/**
 * Frees a JPS_Journey.
 * @param handle to the JPS_Journey to free.
 */
JUPEDSIM_API void JPS_JourneyDescription_Free(JPS_JourneyDescription handle);

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
 * Opaque type of an iterator over agents
 */
typedef struct JPS_GCFMModelAgentIterator_t* JPS_GCFMModelAgentIterator;

/**
 * Access the next element in the iterator.
 * Calling JPS_AgentIterator_Next repeatedly on a finished iterator is save.
 * @param handle of the iterator to advance and access
 * @return an agent or NULL if the iterator is at the end. The pointer returned does not need to be
 * freed and is invalidated on the next call to this function!
 */
JUPEDSIM_API const JPS_GCFMModelAgentParameters*
JPS_GCFMModelAgentIterator_Next(JPS_GCFMModelAgentIterator handle);

/**
 * Free the iterator.
 * @param handle to the JPS_AgentIterator to free.
 */
JUPEDSIM_API void JPS_GCFMModelAgentIterator_Free(JPS_GCFMModelAgentIterator handle);

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
 * Opaque type of an iterator over agents
 */
typedef struct JPS_VelocityModelAgentIterator_t* JPS_VelocityModelAgentIterator;

/**
 * Access the next element in the iterator.
 * Calling JPS_AgentIterator_Next repeatedly on a finished iterator is save.
 * @param handle of the iterator to advance and access
 * @return an agent or NULL if the iterator is at the end. The pointer returned does not need to be
 * freed and is invalidated on the next call to this function!
 */
JUPEDSIM_API const JPS_VelocityModelAgentParameters*
JPS_VelocityModelAgentIterator_Next(JPS_VelocityModelAgentIterator handle);

/**
 * Free the iterator.
 * @param handle to the JPS_AgentIterator to free.
 */
JUPEDSIM_API void JPS_VelocityModelAgentIterator_Free(JPS_VelocityModelAgentIterator handle);

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
 * Access the agent data.
 * @param handle to the simulation object
 * @param id of the agent to access
 * @param[out] agent_out: destination where parameters will be written to. May not be NULL,
 * @param[out] errorMessage if not NULL: will be set to a JPS_ErrorMessage in case of an error.
 * @return success of the operation, will fail on unknown agent id or NULL agent parameter
 */
JUPEDSIM_API bool JPS_Simulation_ReadGCFMModelAgent(
    JPS_Simulation handle,
    JPS_AgentId id,
    JPS_GCFMModelAgentParameters* agent_out,
    JPS_ErrorMessage* errorMessage);

/*
 * Access the agent data.
 * @param handle to the simulation object
 * @param id of the agent to access
 * @param[out] agent: destination where parameters will be written to. May not be NULL,
 * @param[out] errorMessage if not NULL: will be set to a JPS_ErrorMessage in case of an error.
 * @return success of the operation, will fail on unknown agent id or NULL agent parameter
 */
JUPEDSIM_API bool JPS_Simulation_ReadVelocityModelAgent(
    JPS_Simulation handle,
    JPS_AgentId id,
    JPS_VelocityModelAgentParameters* agent_out,
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
 * Returns time simulated per iteration call.
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
JUPEDSIM_API JPS_GCFMModelAgentIterator
JPS_Simulation_GCFMModelAgentIterator(JPS_Simulation handle);

/**
 * Returns an iterator over all agents in the simulation.
 * Notes:
 *   The iterator will be invalidated once JPS_Simulation_Iterate is called.
 *   The iterator needs to be freed after use.
 * @param handle of the simulation
 * @return iterator over agents in the simulation
 */
JUPEDSIM_API JPS_VelocityModelAgentIterator
JPS_Simulation_VelocityModelAgentIterator(JPS_Simulation handle);

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
JPS_Simulation_AgentsInPolygon(JPS_Simulation handle, JPS_Point* polygon, size_t len_polygon);

/**
 * Tell the simulation to change the state (active/inactive) of a waiting set.
 * If the specified journey does not contain a waiting set at the specified index the function
 * returns false and the errorMessage is filled.
 * @param handle of the Simulation to operate on
 * @param journeyId journey to modify
 * @param stageIdx stage to notify, this index has to point to a NotifiableWaitingSet
 * @param active, new state of the WaitingSet
 * @param[out] errorMessage if not NULL: will be set to a JPS_ErrorMessage in case of an error.
 * @return true on success, false on any error, e.g. unknown journeyId
 */
JUPEDSIM_API bool JPS_Simulation_ChangeWaitingSetState(
    JPS_Simulation handle,
    JPS_JourneyId journeyId,
    size_t stageIdx,
    bool active,
    JPS_ErrorMessage* errorMessage);

/**
 * Tell the simulation to release `count` many agents from the queue.
 * @param handle of the Simulation to operate on
 * @param journeyId journey to modify
 * @param stageIdx stage to notify, this index has to point to a NotifiableQueue
 * @param count of agents to release
 * @param[out] errorMessage if not NULL: will be set to a JPS_ErrorMessage in case of an error.
 * @return true on success, false on any error, e.g. unknown journeyId
 */
JUPEDSIM_API bool JPS_Simulation_PopAgentsFromQueue(
    JPS_Simulation handle,
    JPS_JourneyId journeyId,
    size_t stageIdx,
    size_t count,
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

#ifdef __cplusplus
}
#endif
