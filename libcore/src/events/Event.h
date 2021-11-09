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

/// This is just a Dummy to avoid special solutions for a single type variant
class DummyEvent : public BaseEvent
{
};


using Event = std::variant<CreatePedestrianEvent, DummyEvent>;
