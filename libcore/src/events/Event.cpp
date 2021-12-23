#include "Event.h"

#include "Simulation.h"
#include "pedestrian/Pedestrian.h"

#include <chrono>
#include <memory>


BaseEvent::BaseEvent(std::chrono::nanoseconds min_time) : _min_time(min_time) {}


CreatePedestrianEvent::CreatePedestrianEvent(
    Pedestrian const * agent,
    std::chrono::nanoseconds min_time) :
    BaseEvent(min_time),
    _position{agent->GetPos()},
    _final_destination{agent->GetFinalDestination()},
    _group_id{agent->GetGroup()},
    _router_id{agent->GetRouterID()},
    _premovement_time{agent->GetPremovementTime()},
    _ellipse_a_v{agent->GetEllipse().GetAv()},
    _ellipse_a_min{agent->GetEllipse().GetAmin()},
    _ellipse_b_max{agent->GetEllipse().GetBmax()},
    _ellipse_b_min{agent->GetEllipse().GetBmin()},
    _ellipse_do_stretch{agent->GetEllipse().DoesStretch()},
    _tau{agent->GetTau()},
    _T{agent->GetT()},
    _v0{agent->GetEllipse().GetV0()},
    _v0_up_stairs{agent->GetV0UpStairsNorm()},
    _v0_down_stairs{agent->GetV0DownStairsNorm()},
    _v0_escalator_up{agent->GetV0EscalatorUpNorm()},
    _v0_escalator_down{agent->GetV0EscalatorDownNorm()},
    _smooth_factor_up_stairs{agent->GetSmoothFactorUpStairs()},
    _smooth_factor_down_stairs{agent->GetSmoothFactorDownStairs()},
    _smooth_factor_escalator_up{agent->GetSmoothFactorUpEscalators()},
    _smooth_factor_escalator_down{agent->GetSmoothFactorDownEscalators()}
{
}

std::vector<Event> CreateEventsFromAgents(
    std::vector<std::unique_ptr<Pedestrian>> const & agents,
    std::chrono::nanoseconds t)
{
    std::vector<Event> events;
    events.reserve(agents.size());
    for(auto const & agent : agents) {
        events.emplace_back(CreatePedestrianEvent(agent.get(), t));
    }
    return events;
}
