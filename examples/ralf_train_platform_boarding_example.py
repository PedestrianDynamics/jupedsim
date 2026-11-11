import jupedsim as jps
import shapely

# Das Szenorio, das implementiert werden soll:
# - A platform of 30 m x 8 m. 40 passengers start at the back.
# - They walk to a waiting area in the middle of the platform and wait there.
# - At t = 20 s the train arrives. The passengers are released and walk to the platform edge,
#   around (15, 6.5).
# - They board through two doors on the edge, at x = 8-10 m and x = 20-22 m, alternating 1:1
#   between the doors.
# - At t = 35 s door 1 jams. Anyone still heading for door 1 boards through door 2 instead.
# - At the end, print whether everyone is aboard, or how many passengers were left behind.

PLATFORM = shapely.box(0, 0, 30, 8)
BACK_OF_PLATFORM = shapely.box(1, 0.5, 29, 2.5)
WAITING_AREA = shapely.box(6, 3, 24, 5.5)
# Ich hielt das Szenario für unrealistisch. Man geht nicht am Bahnsteig in einen kleinen
# Bereich als "Bottleneck" und geht dann wieder zu den Türen. ich habe jetzt angenommen,
# dass alle gehen mehr oder weniger "nach vorne" gehen. Also nur y geändert bzgl. des
# Wartebereichs.
PLATFORM_EDGE = shapely.box(6, 6, 24, 7)
DOOR1 = shapely.box(8, 7.5, 10, 8)
DOOR2 = shapely.box(20, 7.5, 22, 8)

PASSENGER_COUNT = 40
TRAIN_ARRIVES = 20.0
DOOR1_JAMS = 35.0
# Da nicht gesagt ist, wann die Simulation endet, setze ich einfach mal T=100.0 sec ein.
TRAIN_DEPARTS = 100.0

geo_builder = jps.GeometryBuilder()
platform = geo_builder.add_region(polygon=PLATFORM, height=0, name="platform")

sim = jps.Simulation(
    model=jps.CollisionFreeSpeedModel(), geometry=geo_builder.build()
)

waiting_area = sim.add_routing_target(
    jps.Waypoint([(WAITING_AREA, platform)]), name="waiting area"
)
platform_edge = sim.add_routing_target(
    jps.Waypoint([(PLATFORM_EDGE, platform)]), name="platform edge"
)
door1 = sim.add_routing_target(jps.Exit([(DOOR1, platform)]), name="door 1")
door2 = sim.add_routing_target(jps.Exit([(DOOR2, platform)]), name="door 2")

to_waiting_area = sim.add_route(
    transitions=[jps.FixedTransition(jps.ENTER_ROUTE, waiting_area)],
    name="to waiting area",
)
# Ich habe für das Boarding "RoundRobin" gewählt, weil die Leute vielleicht durch eine
# bestimmte Tür gehen wollen wegen ihrer Sitzplatzreservierung. Man könnte auch die
# "nächste" Tür wollen. Dazu könnte man Mohcines jps.LeastTravelTime() verwenden.
boarding = sim.add_route(
    transitions=[
        jps.FixedTransition(jps.ENTER_ROUTE, platform_edge),
        jps.RoundRobinTransition(platform_edge, {door1: 1, door2: 1}),
    ],
    name="boarding",
)

# Der Wartebereich wird in zufällige Wartepunkte unterteilt, die dann per DirectSteering
# angesteuert werden.
waiting_spots = [
    platform.at(x, y)
    for x, y in jps.distribute_by_number(
        polygon=WAITING_AREA,
        number_of_agents=2 * PASSENGER_COUNT,
        distance_to_agents=0.5,
        distance_to_polygon=0.1,
        seed=42,
    )
]

for x, y in jps.distribute_by_number(
    polygon=BACK_OF_PLATFORM,
    number_of_agents=PASSENGER_COUNT,
    distance_to_agents=0.5,
    distance_to_polygon=0.2,
    seed=1,
):
    sim.add_agent(platform.at(x, y), route=to_waiting_area)

train_arrived = False
while sim.elapsed_time() < TRAIN_DEPARTS and sim.agent_count() > 0:
    sim.iterate()

    # Reroute nur bei denen, die die Wartezone schon erreicht haben, bevor der Zug kommt
    if sim.elapsed_time() < TRAIN_ARRIVES:
        for agent, target in sim.agents_completed_target().items():
            if target == waiting_area:
                agent.reroute(steer_to=waiting_spots.pop())
            # Momentan werden Agenten nicht gestoppt, wenn sie ihre Warteposition erreichen.
            # Ein "agent.stop()" könnte das machen, wenn auch DirectSteering-Targets in
            # sim.agents_completed_target() zurückgegeben werden.

    elif not train_arrived:
        train_arrived = True
        for agent in sim.agents():
            agent.reroute(route=boarding)

    # Every step, not once: the round robin at the platform edge keeps sending agents
    # to door 1.
    if sim.elapsed_time() >= DOOR1_JAMS:
        for agent in sim.agents():
            if agent.target == door1:
                agent.reroute(first_target=door2)

left_behind = sim.agent_count()
if left_behind == 0:
    print("Everyone is aboard.")
else:
    print(f"{left_behind} passengers were left behind.")
