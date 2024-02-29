/* Copyright © 2012-2024 Forschungszentrum Jülich GmbH */
/* SPDX-License-Identifier: LGPL-3.0-or-later */
#pragma once

#include "agent.h"
#include "collision_free_speed_model.h"
#include "collision_free_speed_model_individual.h"
#include "error.h"
#include "export.h"
#include "generalized_centrifugal_force_model.h"
#include "geometry.h"
#include "journey.h"
#include "operational_model.h"
#include "stage.h"
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

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
 * Adds a new direct steering stage to the simulation.
 *
 * This allows a direct control of the target of an agent.
 *
 * Important: A direct steering stage can only be used if it is the only stage in a Journey.
 *
 * @param handle to the simulation to act on
 * @param[out] errorMessage if not NULL: will be set to a JPS_ErrorMessage in case of an error.
 * @return Id of the stage
 */
JUPEDSIM_API JPS_StageId
JPS_Simulation_AddStageDirectSteering(JPS_Simulation handle, JPS_ErrorMessage* errorMessage);

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
JUPEDSIM_API JPS_AgentId JPS_Simulation_AddCollisionFreeSpeedModelIndividualAgent(
    JPS_Simulation handle,
    JPS_CollisionFreeSpeedModelIndividualAgentParameters parameters,
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

JUPEDSIM_API JPS_DirectSteeringProxy JPS_Simulation_GetDirectSteeringProxy(
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

#ifdef __cplusplus
}
#endif
