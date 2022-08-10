#include <jupedsim/jupedsim.h>

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

typedef struct Point {
    double x;
    double y;
} Point;

/*
 * TODO(kkratz): Extend example with error handling
 */

int main(int argc, char** argv)
{
    /* Disable output buffering */
    setbuf(stdout, NULL);
    JPS_GeometryBuilder geo_builder = JPS_GeometryBuilder_Create();

    double box1[] = {0, 0, 10, 0, 10, 10, 0, 10};
    JPS_GeometryBuilder_AddAccessibleArea(geo_builder, box1, 4);

    double box2[] = {10, 4, 20, 4, 20, 6, 10, 6};
    JPS_GeometryBuilder_AddAccessibleArea(geo_builder, box2, 4);

    JPS_Geometry geometry = JPS_GeometryBuilder_Build(geo_builder, NULL);

    JPS_GeometryBuilder_Free(geo_builder);

    JPS_AreasBuilder areas_builder = JPS_AreasBuilder_Create();

    const uint16_t destinationId = 1;
    double box[] = {18, 4, 20, 4, 20, 6, 18, 6};
    const char* labels[] = {"exit"};
    JPS_AreasBuilder_AddArea(
        areas_builder, destinationId, box, 4, labels, 1);

    JPS_Areas areas = JPS_AreasBuilder_Build(areas_builder, NULL);
    JPS_AreasBuilder_Free(areas_builder);

    JPS_OperationalModel model = JPS_OperationalModel_Create_VelocityModel(8, 0.1, 5, 0.02, NULL);

    JPS_Simulation simulation = JPS_Simulation_Create(model, geometry, areas, 0.01, NULL);

    JPS_OperationalModel_Free(model);
    JPS_Geometry_Free(geometry);
    JPS_Areas_Free(areas);

    JPS_AgentParameters agent_parameters;
    agent_parameters.v0 = 1.0;
    agent_parameters.AMin = 0.15;
    agent_parameters.BMax = 0.15;
    agent_parameters.BMin = 0.15;
    agent_parameters.Av = 0.53;
    agent_parameters.T = 1;
    agent_parameters.Tau = 0.5;
    agent_parameters.destinationAreaId = destinationId;
    agent_parameters.orientationX = 1.0;
    agent_parameters.orientationY = 0.0;
    agent_parameters.positionX = 0.0;
    agent_parameters.positionY = 0.0;

    Point positions[] = {{7, 7}, {1, 3}, {1, 5}, {1, 7}, {2, 7}};
    for(int index; index < 5; ++index) {
        agent_parameters.positionX = positions[index].x;
        agent_parameters.positionY = positions[index].y;
        JPS_Simulation_AddAgent(simulation, agent_parameters, NULL);
    }
    printf("Running simulation\n");
    size_t iteration = 0;
    while(JPS_Simulation_AgentCount(simulation) > 0) {
        JPS_Simulation_Iterate(simulation, NULL);
        iteration = JPS_Simulation_IterationCount(simulation);
        if(iteration % 100 == 0){
            printf("Iteration: %zu\n", iteration);
        }
    }
    JPS_Simulation_Free(simulation);
    printf("Simulation completed after %zu iterations\n", iteration);
    return 0;
}
