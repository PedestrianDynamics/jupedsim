// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "AgentRemovalSystem.hpp"
#include "GenericAgent.hpp"
#include "Geometry/Geometry.hpp"
#include "Journey.hpp"
#include "NeighborhoodSearch.hpp"
#include "OperationalDecisionSystem.hpp"
#include "OperationalModel.hpp"
#include "OperationalModelType.hpp"
#include "Point.hpp"
#include "RoutingEngine3D.hpp"
#include "SimulationClock.hpp"
#include "Stage.hpp"
#include "StageDescription.hpp"
#include "StageManager.hpp"
#include "StageSystem.hpp"
#include "StrategicalDesicionSystem.hpp"
#include "TacticalDecisionSystem.hpp"
#include "Timing.hpp"

#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
#include <tuple>
#include <unordered_map>
#include <vector>

class Simulation
{
    SimulationClock _clock;
    StrategicalDecisionSystem _stategicalDecisionSystem{};
    TacticalDecisionSystem _tacticalDecisionSystem{};
    OperationalDecisionSystem _operationalDecisionSystem;
    AgentRemovalSystem<GenericAgent> _agentRemovalSystem{};
    StageManager _stageManager{};
    StageSystem _stageSystem{};
    NeighborhoodSearch<GenericAgent> _neighborhoodSearch{2.2};
    std::unique_ptr<Geometry> _geometry{};
    std::unique_ptr<RoutingEngine3D> _routingEngine{};
    AgentContainer<GenericAgent> _agents;
    std::vector<GenericAgent::ID> _removedAgentsInLastIteration;
    std::unordered_map<Journey::ID, std::unique_ptr<Journey>> _journeys;
    Timer _timer{};
    /// Set for the duration of Iterate(); mutating entry points must not run while the
    /// iteration pipeline works on the agent containers.
    bool _iterating{false};
    enum LogLevel { General = 1, Detailed = 2, Debug = 3 };

    void ThrowIfIterating(const char* operation) const;

public:
    /// Takes the geometry over: after this the caller no longer owns it. `Geo()` hands out a
    /// borrowed reference for as long as the simulation lives.
    Simulation(
        std::unique_ptr<OperationalModel>&& operationalModel,
        std::unique_ptr<Geometry>&& geometry,
        double dT);

    Simulation(const Simulation& other) = delete;
    Simulation& operator=(const Simulation& other) = delete;
    Simulation(Simulation&& other) = delete;
    Simulation& operator=(Simulation&& other) = delete;
    ~Simulation() = default;
    const SimulationClock& Clock() const;
    void SetTracing(bool on);
    void Iterate();
    Journey::ID AddJourney(const std::map<BaseStage::ID, TransitionDescription>& stages);
    /// @param z_hint "stage point" is the closest z on the surface related to @p z_hint
    BaseStage::ID AddStage(const StageDescription stageDescription, double z_hint = 0.0);
    void MarkAgentForRemoval(GenericAgent::ID id);
    const std::vector<GenericAgent::ID>& RemovedAgents() const;
    size_t AgentCount() const;
    double ElapsedTime() const;
    double DT() const;
    void
    SwitchAgentJourney(GenericAgent::ID agent_id, Journey::ID journey_id, BaseStage::ID stage_id);
    uint64_t Iteration() const;
    std::vector<GenericAgent::ID> AgentsInRange(Point p, double distance);
    /// Returns IDs of all agents inside the defined polygon
    /// @param polygon Required to be a simple convex polygon with CCW ordering.
    std::vector<GenericAgent::ID> AgentsInPolygon(const std::vector<Point>& polygon);
    /// @param z_hint Agent will land on the closest z on the surface matching @p position.
    GenericAgent::ID AddAgent(
        Journey::ID journeyId,
        BaseStage::ID stageId,
        Point position,
        OperationalModelState model,
        double z_hint = 0.0);
    /// The place at @p x, @p y on the sheet closest to @p z_hint. Raw coordinates become a
    /// place here and nowhere else: the geometry has to stand for a Location, and over stacked
    /// floors an (x, y) on its own does not say which one is meant.
    /// @throws SimulationError if no walkable sheet lies within the hint's tolerance.
    Location GetLocation(double x, double y, double z_hint = 0.0) const;
    /// Raycast 2D @p target along z-axis. The closest intersection with the geometry to agent's
    /// z coordinate is the one taken.
    void SetAgentTarget(GenericAgent::ID id, Point target);
    void SetAgentTarget(GenericAgent::ID id, const Location& target);
    const GenericAgent& Agent(GenericAgent::ID id) const;
    GenericAgent& Agent(GenericAgent::ID id);
    AgentContainer<GenericAgent>& Agents();
    OperationalModelType ModelType() const;
    StageProxy Stage(BaseStage::ID stageId);
    /// The geometry this simulation runs on. Borrowed: it lives as long as the simulation.
    const Geometry& Geo() const;
    void PushTimer(const std::string_view name, size_t probe_log_level = 0);
    void PopTimer(const std::string_view name);
    void SetTimerLogLevel(int level) { _timer.setLogLevel(level); };
    TimerEntry::duration_type GetTimerDuration(const std::string_view name) const;
    std::map<std::string, TimerEntry::duration_type> GetTimerDurations() const;
};
