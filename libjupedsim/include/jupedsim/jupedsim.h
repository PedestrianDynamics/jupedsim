#pragma once

#include <stddef.h> /*NOLINT(modernize-deprecated-headers)*/
#include <stdint.h> /*NOLINT(modernize-deprecated-headers)*/

/// C-Wrapper for JuPedSim simulation capabilities

#ifdef __cplusplus
extern "C" {
#endif

/// Logging
// enum JPS_LogLevel { NONE, ERROR, WARNING, INFO, DEBUG };

// typedef void (*JPS_log_callback)(JPS_LogLevel level, const char* msg);

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
const char* JPS_ErrorMessage_GetMessage(JPS_ErrorMessage handle);

/**
 * Frees a JPS_ErrorMessage
 * @param handle to the JPS_ErrorMessage to free.
 */
void JPS_ErrorMessage_Free(JPS_ErrorMessage handle);

/**
 * Opaque type for operational models describing how agents interact in the simulation.
 */
typedef struct JPS_OperationalModel_t* JPS_OperationalModel;
/**
 * Creates velocity model.
 * @param a_Ped
 * @param D_Ped
 * @param a_Wall
 * @param D_Wall
 * @param[out] errorMessage if not NULL: will be set to a JPS_ErrorMessage in case of an error.
 * @return the new model or NULL if an error occured during construction
 */
JPS_OperationalModel JPS_OperationalModel_Create_VelocityModel(
    double a_Ped,
    double D_Ped,
    double a_Wall,
    double D_Wall,
    JPS_ErrorMessage* errorMessage = NULL);
/**
 * Creates GCFM model.
 * @param nu_Ped
 * @param nu_Wall
 * @param dist_eff_Ped
 * @param dist_eff_Wall
 * @param intp_width_Ped
 * @param intp_width_Wall
 * @param maxf_Ped
 * @param maxf_Wall
 * @param[out] errorMessage if not NULL: will be set to a JPS_ErrorMessage in case of an error.
 * @return the new model or NULL if an error occured during construction
 */
JPS_OperationalModel JPS_OperationalModel_Create_GCFMModel(
    double nu_Ped,
    double nu_Wall,
    double dist_eff_Ped,
    double dist_eff_Wall,
    double intp_width_Ped,
    double intp_width_Wall,
    double maxf_Ped,
    double maxf_Wall,
    JPS_ErrorMessage* errorMessage = NULL);
/**
 * Frees a JPS_OperationalModel
 * @param handle to the JPS_OperationalModel to free.
 */
void JPS_OperationalModel_Free(JPS_OperationalModel handle);

/**
 * Opaque type that represents the geometry the simulation acts on.
 * This type is created from JPS_GeometryBuilder.
 */
typedef struct JPS_Geometry_t* JPS_Geometry;

/**
 * Frees a JPS_Geometry
 * @param handle to the JPS_Geometry to free.
 */
void JPS_Geometry_Free(JPS_Geometry geometry);

/**
 * Opaque type that is used to build geometry for the simulation.
 */
typedef struct JPS_GeometryBuilder_t* JPS_GeometryBuilder;

/**
 * Creates a new GeometryBuilder
 * @return the new builder.
 */
JPS_GeometryBuilder JPS_GeometryBuilder_Create();

/**
 * Adds an accessible area to the geometry.
 * The polygon described by points and pointCount must be simple convex polygon. The polygon my be
 * specified in clockwise or counter-clockwise order.
 * Multiple accessible areas may overlap, the final accessible area is created by forming the union
 * over all accessible areas.
 * @param points pointer to the x/y coordinates for the points describing the polygon. The number of
 *        double values in this array is expected to be 2*pointCount!
 * @param pointCount number of points the polygon consists of.
 */
void JPS_GeometryBuilder_AddAccessibleArea(
    JPS_GeometryBuilder handle,
    double* points,
    size_t pointCount);

/**
 * Adds an accessible area to the geometry.
 * The polygon described by points and pointCount must be simple convex polygon. The polygon my be
 * specified in clockwise or counter-clockwise order.
 * Multiple exclusion areas may overlap, the final exclusion area is created by forming the union
 * over all exclusion areas.
 * Exclusion areas are subtracted from accessible areas.
 * @param points pointer to the x/y coordinates for the points describing the polygon. The number of
 *        double values in this array is expected to be 2*pointCount!
 * @param pointCount number of points the polygon consists of.
 */
void JPS_GeometryBuilder_ExcludeFromAccessibleArea(
    JPS_GeometryBuilder handle,
    double* lineSegments,
    size_t lineSegmentsCount);

/**
 * Creates a JPS_Geometry from a JPS_GeometryBuilder. After this call the builder still has to be
 * freed with JPS_GeometryBuilder_Free.
 * @param handle to operate on.
 * @param[out] errorMessage if not NULL: will be set to a JPS_ErrorMessage in case of an error.
 * @return a JPS_Geometry handle on success or NULL on any error.
 */
JPS_Geometry
JPS_GeometryBuilder_Build(JPS_GeometryBuilder handle, JPS_ErrorMessage* errorMessage = NULL);

/**
 * Frees a JPS_GeometryBuilder.
 * @param handle to the JPS_GeometryBuilder to free.
 */
void JPS_GeometryBuilder_Free(JPS_GeometryBuilder handle);

/**
 * Opaque type that describes convex areas that agents navigate to / can be influced from.
 * JPS_Areas are created vis a JPS_AreasBuilder.
 * NOTE: Currently they only function as navigation targets / exit zones.
 */
typedef struct JPS_Areas_t* JPS_Areas;

/**
 * Frees a JPS_Areas.
 * @param handle to the JPS_Areas to free.
 */
void JPS_Areas_Free(JPS_Areas handle);

/**
 * Opaque type that builds a JPS_Areas object.
 */
typedef struct JPS_AreasBuilder_t* JPS_AreasBuilder;

/**
 * Creates a JPS_AreasBuilder.
 */
JPS_AreasBuilder JPS_AreasBuilder_Create();

/**
 * Adds a area with lables to the builder.
 * NOTE: Currently only the label "exit" is supported and marks the labeled area as an exit. Agents
 * that are located wothin any area labeled with "exit" are removed at the beginning of an
 * iteration.
 * Area ids can be used to instruct agents to navigate towards the center of this area.
 * WARNING: The polygon describing a area needs to be simple and convex!
 * @param handle of builder to operate on.
 * @param id of the area. Ids need to be unique over all areas.
 * @param points pointer to the x/y coordinates for the points describing the polygon. The number of
 *        double values in this array is expected to be 2*pointCount!
 * @param pointCount number of points the polygon consists of.
 * @param tags lables to attach to this area
 * @param tagCount number of lables
 */
void JPS_AreasBuilder_AddArea(
    JPS_AreasBuilder handle,
    uint64_t id,
    double* points,
    size_t pointCount,
    const char** tags,
    size_t tagCount);

/**
 * Builds a JPS_Areas object.
 * @param handle of the builder to operate on
 * @param[out] errorMessage if not NULL: will be set to a JPS_ErrorMessage in case of an error.
 */
JPS_Areas JPS_AreasBuilder_Build(JPS_AreasBuilder handle, JPS_ErrorMessage* errorMessage = NULL);

/**
 * Frees a JPS_AreasBuilder.
 * @param handle to the JPS_AreasBuilder to free.
 */
void JPS_AreasBuilder_Free(JPS_AreasBuilder handle);

/**
 * Id of an agent.
 */
typedef uint64_t JPS_AgentId;

/**
 * Opaque type of a agent in the simulation.
 */
typedef struct JPS_Agent_t* JPS_Agent;

/*
 * Access the agents positions x value.
 * @param handle of the agent.
 * @return x position
 */
double JPS_Agent_PositionX(JPS_Agent handle);

/*
 * Access the agents positions y value.
 * @param handle of the agent.
 * @return y position
 */
double JPS_Agent_PositionY(JPS_Agent handle);

/*
 * Access the agents orientation x value.
 * @param handle of the agent.
 * @return x orientation
 */
double JPS_Agent_OrientationX(JPS_Agent handle);

/*
 * Access the agents orientation y value.
 * @param handle of the agent.
 * @return y orientation
 */
double JPS_Agent_OrientationY(JPS_Agent handle);

/**
 * Describe all the parameters required to initialize an agent.
 * See the individual fields for more information.
 */
typedef struct JPS_AgentParameters {
    /*
     * positionX forms together with positionY the positional vector of the agent.
     * The position needs to inside the accessible area.
     */
    double positionX;
    /*
     * positionY forms together with positionX the positional vector of the agent.
     * The position needs to inside the accessible area.
     */
    double positionY;
    /**
     * orientationX forms together with orientationY the orientation vector of the agent.
     * The orientation vector will internally be normalized.
     */
    double orientationX;
    /*
     * orientationY forms together with orientationX the orientation vector of the agent.
     * The orientation vector will internally be normalized.
     */
    double orientationY;
    double v0;
    double T;
    double Tau;
    double Av;
    double AMin;
    double BMax;
    double BMin;
    /*
     * Id of the area the agent wants to reach.
     * NOTE: This field will change at some point before the next release
     */
    uint16_t destinationAreaId;
} AgentParameters;

/*
 * Opaque type to a Simulator object.
 */
typedef struct JPS_Simulation_t* JPS_Simulation;

/*
 * Creates a new JPS_Simulation object.
 * NOTE: JPS_Simulation_Create will take ownership of all indicated parameters even in case an error
 * occured.
 * @param model to use. Will take ownership.
 * @param geometry to use. Will take ownership.
 * @param areas to use. Will take ownership.
 * @param dT simulation timestep in seconds
 * @param[out] errorMessage if not NULL: will be set to a JPS_ErrorMessage in case of an error.
 * @return the Simulation
 */
JPS_Simulation JPS_Simulation_Create(
    JPS_OperationalModel model,
    JPS_Geometry geometry,
    JPS_Areas areas,
    double dT,
    JPS_ErrorMessage* errorMessage = NULL);

/**
 * Adds a new agent to the simulation.
 * This can be called at any time, i.e. agents can be added at any iteration.
 * NOTE: Currently there is no checking done to ensure the agent can be placed at the desired
 * location.
 * @param handle to the simulation to act on
 * @param parameters describing the new agent.
 * @param[out] errorMessage if not NULL: will be set to a JPS_ErrorMessage in case of an error.
 * @return id of the new agent or 0 if the agent could not be added due to an error.
 */
JPS_AgentId JPS_Simulation_AddAgent(
    JPS_Simulation handle,
    JPS_AgentParameters parameters,
    JPS_ErrorMessage* errorMessage = NULL);
/*
 * Access the agent data.
 * @param handle to the simulation object
 * @param id of the agent to access
 * @param[out] errorMessage if not NULL: will be set to a JPS_ErrorMessage in case of an error.
 * @return handle to the agent, the handle will be valid until the next iteration is started.
 */
JPS_Agent JPS_Simulation_ReadAgent(
    JPS_Simulation handle,
    JPS_AgentId id,
    JPS_ErrorMessage* errorMessage = NULL);

/*
 * Returns the ids of all agents that exited the simulation in the last iteration.
 * @param handle of the Simulation
 * @param[out] data pointer to the ids
 * @return numer of agents that have been removed in the last iteration
 */
size_t JPS_Simulation_RemovedAgents(JPS_Simulation handle, const JPS_AgentId** data);

/*
 * Advances the simulation by one step.
 * @param handle of the Simulation
 * @param[out] errorMessage if not NULL: will be set to a JPS_ErrorMessage in case of an error.
 * @return true if no errors occured
 */
bool JPS_Simulation_Iterate(JPS_Simulation handle, JPS_ErrorMessage* errorMessage = NULL);

/**
 * How many agents are in the simulation.
 * @param handle of the simulation
 * @return count agents in the simulation
 */
size_t JPS_Simulation_AgentCount(JPS_Simulation handle);

/**
 * Returns how many iterations have been simulated.
 * @param handle of the simulation
 * @return count of elapsed iterations
 */
uint64_t JPS_Simulation_IterationCount(JPS_Simulation handle);

/**
 * Frees a JPS_Simulation.
 * @param handle to the JPS_Simulation to free.
 */
void JPS_Simulation_Free(JPS_Simulation handle);

#ifdef __cplusplus
}
#endif
