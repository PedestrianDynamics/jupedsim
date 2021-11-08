#pragma once

#include <chrono>
#include <variant>

class BaseEvent
{
protected:
    /// (min) time to trigger event
    std::chrono::nanoseconds _min_time;

public:
    std::chrono::nanoseconds MinTime() const { return _min_time; }
};


class CreatePedestrianEvent : public BaseEvent
{
};

using Event = std::variant<CreatePedestrianEvent>;


inline std::chrono::nanoseconds EventMinTime(Event event)
{
    auto call = [](auto const & obj) { return obj.MinTime(); };
    return std::visit(call, event);
}
