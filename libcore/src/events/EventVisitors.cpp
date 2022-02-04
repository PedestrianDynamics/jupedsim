#include "EventVisitors.hpp"

#include "events/Event.hpp"
#include "pedestrian/Pedestrian.hpp"

#include <memory>
#include <variant>

std::chrono::nanoseconds EventMinTime(const Event & event)
{
    auto call = [](auto const & obj) { return obj.execute_at; };
    return std::visit(call, event);
}


void ProcessEvent(const CreatePedestrianEvent & event, Simulation & sim)
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

void ProcessEvent(const DoorEvent & event, Simulation & sim)
{
    switch(event.type) {
        case DoorEvent::Type::OPEN:
            sim.OpenDoor(event.doorId);
            break;
        case DoorEvent::Type::TEMP_CLOSE:
            sim.TempCloseDoor(event.doorId);
            break;
        case DoorEvent::Type::CLOSE:
            sim.CloseDoor(event.doorId);
            break;
        case DoorEvent::Type::RESET:
            sim.ResetDoor(event.doorId);
            break;
    }
}
void ProcessEvent(const TrainEvent & event, Simulation & sim)
{
    switch(event.type) {
        case TrainEvent::Type::ARRIVAL:
            sim.ActivateTrain(
                event.trainID,
                event.trackID,
                event.trainType,
                event.trainStartOffset,
                event.reversed);
            break;
        case TrainEvent::Type::DEPARTURE:
            sim.DeactivateTrain(event.trainID, event.trackID);
            break;
    }
}
