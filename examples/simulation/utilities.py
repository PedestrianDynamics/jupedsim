import py_jupedsim as jps
from configs import log_error, log_info


def build_geometry(accessible_areas) -> jps.GeometryBuilder:
    """build geometry object

    All points should be defined CCW
    :returns: a geometry builder

    """
    log_info("Init geometry")
    geo_builder = jps.GeometryBuilder()
    for accessible_area in accessible_areas:
        geo_builder.add_accessible_area(accessible_area)

    geometry = geo_builder.build()
    return geometry


def build_areas(destinations: dict, labels: list) -> jps.AreasBuilder:
    """Build destination areas with CCW-Polygon

    :returns: Area builder

    :param polygon: list of (x, y)

    """
    log_info("Init areas")
    areas_builder = jps.AreasBuilder()
    for destination_id, polygon in destinations.items():
        areas_builder.add_area(
            destination_id,
            polygon,
            labels,
        )

    areas = areas_builder.build()
    return areas


def build_gcfm_model(
    nu_ped: float,
    nu_wall: float,
    dist_eff_ped: float,
    disr_eff_wall: float,
    intp_width_ped: float,
    intp_width_wall: float,
    maxf_ped: float,
    maxf_wall: float,
    parameter_profiles: dict,
) -> jps.OperationalModel:
    """Initialize gcfm model with parameter values

    :param nu_ped:
    :param nu_wall:
    :param dist_eff_ped:
    :param disr_eff_wall:
    :param intp_width_ped:
    :param intp_width_wall:
    :param maxf_ped:
    :param maxf_wall:
    :param parameter_profiles:
    :returns: gcfm model

    """
    log_info("Init gcfm model")
    model_builder = jps.GCFMModelBuilder(
        nu_ped,
        nu_wall,
        dist_eff_ped,
        disr_eff_wall,
        intp_width_ped,
        intp_width_wall,
        maxf_ped,
        maxf_wall,
    )
    # define two different profiles
    for key, params in parameter_profiles.items():
        model_builder.add_parameter_profile(
            key, params[0], params[1], params[2]
        )
    model = model_builder.build()
    return model


def build_velocity_model(
    a_ped: float,
    D_ped: float,
    a_wall: float,
    D_wall: float,
    parameter_profiles: dict,
) -> jps.OperationalModel:
    """Initialize velocity model with parameter values

    :param a_ped:
    :param D_ped:
    :param a_wall:
    :param D_wall:
    :returns: velocity model

    """
    log_info("Init velocity model")
    model_builder = jps.VelocityModelBuilder(a_ped, D_ped, a_wall, D_wall)
    # define two different profiles
    for key, params in parameter_profiles.items():
        model_builder.add_parameter_profile(
            key, params[0], params[1], params[2]
        )
    model = model_builder.build()
    return model


def init_journey(simulation: jps.Simulation, way_points: list) -> int:
    """Init goals of agents to follow

    :param simulation:
    :param way_points: defined as a list of (point, distance)
    :returns:

    """
    log_info("Init journey")
    journey = jps.Journey.make_waypoint_journey(way_points)
    journey_id = simulation.add_journey(journey)
    return journey_id


def init_gcfm_agent_parameters(
    a_min: float,
    b_min: float,
    b_max: float,
    a_v: float,
    phi_x: float,
    phi_y: float,
    journey: jps.Journey,
    profile: int,
) -> jps.AgentParameters:
    """Init agent shape and parameters

    :param a_min:
    :param b_min:
    :param b_max:
    :param a_v:
    :param phi_x: direcion in x-axis
    :param phi_y: direction in y-axis
    :param journey: waypoints for agents to pass through
    :param profile: profile id
    :returns:

    """
    log_info("Create agents")
    agent_parameters = jps.AgentParameters()
    # Shape is a circle
    agent_parameters.a_min = a_min
    agent_parameters.b_min = b_min
    agent_parameters.b_max = b_max
    agent_parameters.a_v = a_v
    # ----- Profile
    agent_parameters.journey_id = journey
    agent_parameters.orientation_x = phi_x
    agent_parameters.orientation_y = phi_y
    agent_parameters.profile_id = profile
    return agent_parameters


def init_velocity_agent_parameters(
    radius: float,
    phi_x: float,
    phi_y: float,
    journey: jps.Journey,
    profile: int,
) -> jps.AgentParameters:
    """Init agent shape and parameters

    :param radius: radius of the circle
    :param phi_x: direcion in x-axis
    :param phi_y: direction in y-axis
    :param journey: waypoints for agents to pass through
    :param profile: profile id
    :returns:

    """
    log_info("Create agents")
    agent_parameters = jps.AgentParameters()
    # Shape is a circle
    agent_parameters.a_min = radius
    agent_parameters.b_max = radius
    agent_parameters.b_min = radius
    agent_parameters.a_v = 0.0
    # ----- Profile
    agent_parameters.journey_id = journey
    agent_parameters.orientation_x = phi_x
    agent_parameters.orientation_y = phi_y
    agent_parameters.profile_id = profile
    return agent_parameters


def distribute_and_add_agents(
    simulation: jps.Simulation,
    agent_parameters: jps.AgentParameters,
    positions: list,
) -> list:
    """Initialize positions of agents and insert them into the simulation

    :param simulation:
    :param agent_parameters:
    :param positions:
    :returns:

    """
    ped_ids = []
    for x, y in positions:
        agent_parameters.x = x
        agent_parameters.y = y
        ped_id = simulation.add_agent(agent_parameters)
        ped_ids.append(ped_id)

    return ped_ids