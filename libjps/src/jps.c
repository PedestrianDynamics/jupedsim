#include "jps.h"

typedef struct SimulationContext {
    void * sim_context;
} SimulationContext;

SimulationContext * initialize(const char * geometry_file)
{
    (void) geometry_file;

    return NULL;
}

extern int add_agent(SimulationContext * simulation_context, int starting_area, Route route)
{
    (void) simulation_context;
    (void) starting_area;
    (void) route;

    return -1;
}

void simulate(SimulationContext * simulation_context, double simulation_time)
{
    (void) simulation_context;
    (void) simulation_time;
}

Agents get_agents(SimulationContext * simulation_context)
{
    (void) simulation_context;

    Agents agents = {.agents = NULL, .size = 0};
    return agents;
}

Agents get_retrievable_agents(SimulationContext * simulation_context)
{
    (void) simulation_context;

    Agents agents = {.agents = NULL, .size = 0};
    return agents;
}

void confirm_retrieved_agents(SimulationContext * simulation_context, Agents agents)
{
    (void) simulation_context;
    (void) agents;
}

void finalize(SimulationContext * simulation_context)
{
    (void) simulation_context;
}
