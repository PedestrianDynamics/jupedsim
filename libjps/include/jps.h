#ifndef _JPS_H_
#define _JPS_H_

#include <stdbool.h>
#include <stdlib.h>

typedef struct JPS_SimulationContext JPS_SimulationContext;

typedef struct {
    int id;
    double x;
    double y;
    double velocity;
    double v_phi;
} JPS_Agent;

typedef struct {
    JPS_Agent * agents;
    size_t size;
} JPS_Agents;

typedef struct {
    int * goals;
    size_t num_goals;
} JPS_Route;

/**
 * @name JPSlib Functions.
 * @param [in]  simulation_context  The simulation context (owned by JPSlib).
 * @{
 **/

/**
 * Initialize the simulation
 * @param [in]  geometry_file The filepath to the geometry configuration file.
 * @return                    The simulation context.
 **/
extern JPS_SimulationContext * JPS_initialize(const char * geometry_file);

/**
 * Adds a new agent to the pedestrian simulation.
 *
 * @param [in] starting_area      The unique identifier of the starting area to place the new agent.
 * @param [in] route              The goals of the pedestrian reflecting the desired route.
 * @return                        Returns the ID of the newly added agent or -1 if agent could not be added.
 **/
extern int
JPS_add_agent(JPS_SimulationContext * simulation_context, int starting_area, JPS_Route route);

/**
 * Computes the trajectories for all pedestrians for a given simulation_time.
 *
 * @param [in]  simulation_time Simulation time duration to compute.
 **/
extern void JPS_simulate(JPS_SimulationContext * simulation_context, double simulation_time);

/**
 * Receives the agents from the simulation containing position and velocity.
 *
 * @return Agents array containing coordinates and velocities.
 **/
extern JPS_Agents JPS_get_agents(JPS_SimulationContext * simulation_context);

/**
 * Retrieves the agents which are removed from the pedestrian simulation.
 *
 * @returns An array of agents.
 **/
extern JPS_Agents JPS_get_retrievable_agents(JPS_SimulationContext * simulation_context);

/**
 * Confirms the retrieved agents, not confirmed agents must be kept in the pedestrian simulation.
 *
 * @param [in] agents An array of agents which are retrieved successfully.
 **/
extern void
JPS_confirm_retrieved_agents(JPS_SimulationContext * simulation_context, JPS_Agents agents);

/**
 * Finalizes the simulation and cleans up memory.
 **/
extern void JPS_finalize(JPS_SimulationContext * simulation_context);
/** @} */
#endif /* _JPS_H_ */
