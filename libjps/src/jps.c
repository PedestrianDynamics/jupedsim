#include "jps.h"

SimulationContext initialize(const char * geometry_file, Status * status)
{
    (void) geometry_file;
    (void) status;

    SimulationContext context = {.sim_context = NULL};
    return context;
}

extern int
add_agent(SimulationContext simulation_context, int starting_area, Route route, Status * status)
{
    (void) simulation_context;
    (void) starting_area;
    (void) route;
    (void) status;

    return -1;
}

void simulate(SimulationContext simulation_context, double simulation_time, Status * status)
{
    (void) simulation_context;
    (void) simulation_time;
    (void) status;
}

Agents get_agents(SimulationContext simulation_context, Status * status)
{
    (void) simulation_context;
    (void) status;

    Agents agents = {.agents = NULL, .size = 0};
    return agents;
}

Agents get_retrievable_agents(SimulationContext simulation_context, Status * status)
{
    (void) simulation_context;
    (void) status;

    Agents agents = {.agents = NULL, .size = 0};
    return agents;
}

void confirm_retrieved_agents(SimulationContext simulation_context, Agents agents, Status * status)
{
    (void) simulation_context;
    (void) agents;
    (void) status;
}

void finalize(SimulationContext simulation_context, Status * status)
{
    (void) simulation_context;
    (void) status;
}
