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

    ped->SetFinalDestination(event._final_destination);
    ped->SetGroup(event._group_id);
    ped->SetPremovementTime(event._premovement_time);

    JEllipse E = JEllipse();
    E.SetAv(event._ellipse_a_v);
    E.SetAmin(event._ellipse_a_min);
    E.SetBmax(event._ellipse_b_max);
    E.SetBmin(event._ellipse_b_min);
    E.DoStretch(event._ellipse_do_stretch);
    ped->SetEllipse(E);

    ped->SetTau(event._tau);
    ped->SetT(event._T);
    ped->SetV0Norm(
        event._v0,
        event._v0_up_stairs,
        event._v0_down_stairs,
        event._v0_escalator_up,
        event._v0_escalator_down);
    ped->SetSmoothFactorEscalatorUpStairs(event._smooth_factor_escalator_up);
    ped->SetSmoothFactorEscalatorDownStairs(event._smooth_factor_escalator_down);
    ped->SetSmoothFactorUpStairs(event._smooth_factor_up_stairs);
    ped->SetSmoothFactorDownStairs(event._smooth_factor_down_stairs);
    ped->SetPos(event._position);
    ped->SetRouterId(event._router_id);

    sim.AddAgent(std::move(ped));
}
void ProcessEvent(DummyEvent /*event*/, Simulation & /*sim*/) {}
