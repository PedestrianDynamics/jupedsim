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
    Position{agent->GetPos()},
    FinalDestination{agent->GetFinalDestination()},
    GroupId{agent->GetGroup()},
    RouterId{agent->GetRouter()->GetID()},
    RoomId{agent->GetRoomID()},
    SubroomId{agent->GetSubRoomID()},
    PatienceTime{agent->GetPatienceTime()},
    PremovementTime{agent->GetPremovementTime()},
    RiskTolerance{agent->GetRiskTolerance()},
    EllipsisA_V{agent->GetEllipse().GetAv()},
    EllipsisAMin{agent->GetEllipse().GetAmin()},
    EllipsisBMax{agent->GetEllipse().GetBmax()},
    EllipsisBMin{agent->GetEllipse().GetBmin()},
    EllipsisDoStretch{agent->GetEllipse().DoesStretch()},
    Tau{agent->GetTau()},
    T{agent->GetT()},
    V0{agent->GetEllipse().GetV0()},
    V0UpStairs{agent->GetV0UpStairsNorm()},
    V0DownStairs{agent->GetV0DownStairsNorm()},
    V0EscalatorUp{agent->GetV0EscalatorUpNorm()},
    V0EscalatorDown{agent->GetV0EscalatorDownNorm()},
    SmoothFactorUpStairs{agent->GetSmoothFactorUpStairs()},
    SmoothFactorDownStairs{agent->GetSmoothFactorDownStairs()},
    SmoothFactorEscalatorUp{agent->GetSmoothFactorUpEscalators()},
    SmoothFactorEscalatorDown{agent->GetSmoothFactorDownEscalators()}
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
