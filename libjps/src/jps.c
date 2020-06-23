#include "jps.h"

typedef struct JPS_SimulationContext {
    void * sim_context;
} JPS_SimulationContext;

JPS_SimulationContext * JPS_initialize(const char * geometry_file)
{
    (void) geometry_file;

    return NULL;
}

extern int
JPS_add_agent(JPS_SimulationContext * simulation_context, int starting_area, JPS_Route route)
{
    (void) simulation_context;
    (void) starting_area;
    (void) route;

    return -1;
}

void JPS_simulate(JPS_SimulationContext * simulation_context, double simulation_time)
{
    (void) simulation_context;
    (void) simulation_time;
}

JPS_Agents JPS_get_agents(JPS_SimulationContext * simulation_context)
{
    (void) simulation_context;

    JPS_Agents agents = {.agents = NULL, .size = 0};
    return agents;
}

JPS_Agents JPS_get_retrievable_agents(JPS_SimulationContext * simulation_context)
{
    (void) simulation_context;

    JPS_Agents agents = {.agents = NULL, .size = 0};
    return agents;
}

void JPS_confirm_retrieved_agents(JPS_SimulationContext * simulation_context, JPS_Agents agents)
{
    (void) simulation_context;
    (void) agents;
}

void JPS_finalize(JPS_SimulationContext * simulation_context)
{
    (void) simulation_context;
}
