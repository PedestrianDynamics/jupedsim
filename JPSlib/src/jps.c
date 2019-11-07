#include "jps.h"

SimulationContext initialize(const char * geometry_file, Status * status)
{
    SimulationContext context = {.sim_context = NULL};
    return context;
}

extern bool transfer_agent(
    SimulationContext simulation_context,
    int agent_id,
    int starting_area,
    Route route,
    Status * status)
{
    return false;
}

void simulate(SimulationContext simulation_context, double simulation_time, Status * status) {}

Agents get_agents(SimulationContext simulation_context, Status * status)
{
    Agents agents = {.agents = NULL, .size = 0};
    return agents;
}

Agents get_retrievable_agents(SimulationContext simulation_context, Status * status)
{
    Agents agents = {.agents = NULL, .size = 0};
    return agents;
}

void confirm_retrieved_agents(SimulationContext simulation_context, Agents agents, Status * status)
{
}

void finalize(SimulationContext simulationContext, Status * status) {}
