#include "EventVisitors.h"

#include "events/Event.h"
#include "pedestrian/Pedestrian.h"

#include <memory>
#include <variant>

std::chrono::nanoseconds EventMinTime(Event event)
{
    auto call = [](auto const & obj) { return obj.MinTime(); };
    return std::visit(call, event);
}


void ProcessEvent(CreatePedestrianEvent event, Simulation & sim)
{
    std::unique_ptr<Pedestrian> ped = std::make_unique<Pedestrian>();

    ped->SetFinalDestination(event.FinalDestination);
    ped->SetGroup(event.GroupId);
    ped->SetPatienceTime(event.PatienceTime);
    ped->SetPremovementTime(event.PremovementTime);
    ped->SetRiskTolerance(event.RiskTolerance);

    JEllipse E = JEllipse();
    E.SetAv(event.EllipsisA_V);
    E.SetAmin(event.EllipsisAMin);
    E.SetBmax(event.EllipsisBMax);
    E.SetBmin(event.EllipsisBMin);
    E.DoStretch(event.EllipsisDoStretch);
    ped->SetEllipse(E);

    ped->SetTau(event.Tau);
    ped->SetT(event.T);
    ped->SetV0Norm(
        event.V0, event.V0UpStairs, event.V0DownStairs, event.V0EscalatorUp, event.V0EscalatorDown);
    ped->SetSmoothFactorEscalatorUpStairs(event.SmoothFactorEscalatorUp);
    ped->SetSmoothFactorEscalatorDownStairs(event.SmoothFactorEscalatorDown);
    ped->SetSmoothFactorUpStairs(event.SmoothFactorUpStairs);
    ped->SetSmoothFactorDownStairs(event.SmoothFactorDownStairs);
    ped->SetPos(event.Position);
    ped->SetRouterId(event.RouterId);

    sim.AddAgent(std::move(ped));
}
void ProcessEvent(DummyEvent event, Simulation & sim) {}
