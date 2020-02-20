#ifndef _JPS_H_
#define _JPS_H_

#include <stdbool.h>
#include <stdlib.h>

typedef struct {
    void * sim_context;
} SimulationContext;

typedef struct {
    bool has_error;
    char * error_message;
} Status;

typedef struct {
    double x;
    double y;
} Coordinate;

typedef struct {
    int id;
    Coordinate coordinate;
    double velocity;
    double v_phi;
} Agent;

typedef struct {
    Agent * agents;
    size_t size;
} Agents;

typedef struct {
    int * goals;
    size_t num_golas;
} Route;

/**
 * @name JPSlib Functions.
 * @param [in]  simulation_context  The simulation context (owned by JPSlib).
 * @param [out] status              Status object containing error information and message.
 * @{
 **/

/**
 * Initialize the simulation
 * @param [in]  geometry_file The filepath to the geometry configuration file.
 * @return                    The simulation context.
 **/
extern SimulationContext initialize(const char * geometry_file, Status * status);

/**
 * Adds a new agent to the pedestrian simulation.
 *
 * @param [in] agent_id           The unique identifier for the newly created agent.
 * @param [in] starting_area      The unique identifier of the starting area to place the new agent.
 * @param [in] route              The goals of the pedestrian reflecting the desired route.
 * @return                        Bool whether the agent could be added to the designated area.
 **/
extern bool transfer_agent(
    SimulationContext simulation_context,
    int agent_id,
    int starting_area,
    Route route,
    Status * status);

/**
 * Computes the trajectories for all pedestrians for a given simulation_time.
 *
 * @param [in]  simulation_time Simulation time duration to compute.
 **/
extern void simulate(SimulationContext simulation_context, double simulation_time, Status * status);

/**
 * Receives the agents from the simulation containing position and velocity.
 *
 * @return Agents array containing coordinates and velocities.
 **/
extern Agents get_agents(SimulationContext simulation_context, Status * status);

/**
 * Retrieves the agents which are removed from the pedestrian simulation.
 *
 * @returns An array of agents.
 **/
extern Agents get_retrievable_agents(SimulationContext simulation_context, Status * status);

/**
 * Confirms the retrieved agents, not confirmed agents must be kept in the pedestrian simulation.
 *
 * @param [in] agents An array of agents which are retrieved successfully.
 **/
extern void
confirm_retrieved_agents(SimulationContext simulation_context, Agents agents, Status * status);

/**
 * Finalizes the simulation and cleans up memory.
 **/
extern void finalize(SimulationContext simulation_context, Status * status);
/** @} */
#endif /* _JPS_H_ */
