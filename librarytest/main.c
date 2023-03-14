/* Copyright © 2012-2022 Forschungszentrum Jülich GmbH */
/* SPDX-License-Identifier: LGPL-3.0-or-later */
#include <jupedsim/jupedsim.h>

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

/*
 * If you want to enable logging in libjupedsim you will need to provide handler functions for each
 * log level.
 */
void handle_message(const char* prefix, const char* msg)
{
    printf("%s MESSAGE: \"%s\"\n", prefix, msg);
}

void handle_debug_message(const char* msg, void* userdata)
{
    handle_message("DEBUG", msg);
}

void handle_info_message(const char* msg, void* userdata)
{
    handle_message("INFO", msg);
}

void handle_warning_message(const char* msg, void* userdata)
{
    handle_message("WARNING", msg);
}

void handle_error_message(const char* msg, void* userdata)
{
    handle_message("ERROR", msg);
}

int main(int argc, char** argv)
{
    /* Disable output buffering */
    setbuf(stdout, NULL);

    JPS_ErrorMessage error_msg;

    /* Register log message handlers with libjupedsim */
    /*JPS_Logging_SetDebugCallback(handle_debug_message, NULL);*/
    JPS_Logging_SetInfoCallback(handle_info_message, NULL);
    JPS_Logging_SetWarningCallback(handle_warning_message, NULL);
    JPS_Logging_SetErrorCallback(handle_error_message, NULL);
    JPS_GeometryBuilder geo_builder = JPS_GeometryBuilder_Create();

    double box1[] = {0, 0, 10, 0, 10, 10, 0, 10};
    JPS_GeometryBuilder_AddAccessibleArea(geo_builder, box1, 4);

    double box2[] = {10, 4, 20, 4, 20, 6, 10, 6};
    JPS_GeometryBuilder_AddAccessibleArea(geo_builder, box2, 4);

    JPS_Geometry geometry = JPS_GeometryBuilder_Build(geo_builder, &error_msg);
    if(geometry == NULL) {
        printf("Error creating geometry: %s\n", JPS_ErrorMessage_GetMessage(error_msg));
        JPS_ErrorMessage_Free(error_msg);
        return -1;
    }

    JPS_GeometryBuilder_Free(geo_builder);

    JPS_AreasBuilder areas_builder = JPS_AreasBuilder_Create();

    const uint16_t destinationId = 1;
    double box[] = {18, 4, 20, 4, 20, 6, 18, 6};
    const char* labels[] = {"exit"};
    JPS_AreasBuilder_AddArea(areas_builder, destinationId, box, 4, labels, 1);

    JPS_Areas areas = JPS_AreasBuilder_Build(areas_builder, &error_msg);
    if(areas == NULL) {
        printf("Error creating areas: %s\n", JPS_ErrorMessage_GetMessage(error_msg));
        JPS_ErrorMessage_Free(error_msg);
        return -1;
    }
    JPS_AreasBuilder_Free(areas_builder);

    JPS_VelocityModelBuilder model_builder = JPS_VelocityModelBuilder_Create(8, 0.1, 5, 0.02);
    const u_int64_t profile_id = 1;
    JPS_VelocityModelBuilder_AddParameterProfile(model_builder, profile_id, 1, 0.5, 1.2, 0.3);

    JPS_OperationalModel model = JPS_VelocityModelBuilder_Build(model_builder, &error_msg);
    if(model == NULL) {
        printf("Error creating model: %s\n", JPS_ErrorMessage_GetMessage(error_msg));
        JPS_ErrorMessage_Free(error_msg);
        return -1;
    }

    JPS_Simulation simulation = JPS_Simulation_Create(model, geometry, areas, 0.01, &error_msg);
    if(areas == NULL) {
        printf("Error creating simuation: %s\n", JPS_ErrorMessage_GetMessage(error_msg));
        JPS_ErrorMessage_Free(error_msg);
        return -1;
    }

    const size_t num_waypoints = 1;
    JPS_Waypoint waypoints[] = {{{19.95, 5}, 0.4}};
    JPS_Journey journey = JPS_Journey_Create_SimpleJourney(waypoints, num_waypoints);
    JPS_JourneyId journey_id = JPS_Simulation_AddJourney(simulation, journey, &error_msg);
    if(journey_id == 0) {
        printf("Error creating journey: %s\n", JPS_ErrorMessage_GetMessage(error_msg));
        JPS_ErrorMessage_Free(error_msg);
        return -1;
    }

    JPS_OperationalModel_Free(model);
    JPS_Geometry_Free(geometry);
    JPS_Areas_Free(areas);

    JPS_VelocityModelAgentParameters agent_parameters;
    agent_parameters.e0 = (JPS_Point){0.0, 0.0};
    agent_parameters.position = (JPS_Point){0.0, 0.0};
    agent_parameters.orientation = (JPS_Point){1.0, 0.0};
    agent_parameters.journeyId = journey_id;
    agent_parameters.profileId = profile_id;

    JPS_Point positions[] = {{7, 7}, {1, 3}, {1, 5}, {1, 7}, {2, 7}};
    for(int index; index < 5; ++index) {
        agent_parameters.position = positions[index];
        if(!JPS_Simulation_AddVelocityModelAgent(simulation, agent_parameters, &error_msg)) {
            printf("Error creating areas: %s\n", JPS_ErrorMessage_GetMessage(error_msg));
            JPS_ErrorMessage_Free(error_msg);
            return -1;
        }
    }
    printf("Running simulation\n");
    size_t iteration = 0;
    while(JPS_Simulation_AgentCount(simulation) > 0) {
        if(!JPS_Simulation_Iterate(simulation, &error_msg)) {
            printf("Error running simulation: %s\n", JPS_ErrorMessage_GetMessage(error_msg));
            JPS_ErrorMessage_Free(error_msg);
            return -1;
        }
        iteration = JPS_Simulation_IterationCount(simulation);
        if(iteration % 100 == 0) {
            printf("Iteration: %zu\n", iteration);
        }
    }
    JPS_Simulation_Free(simulation);
    printf("Simulation completed after %zu iterations\n", iteration);
    return 0;
}
