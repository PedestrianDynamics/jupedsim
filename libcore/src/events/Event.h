#pragma once

#include <variant>


class CreatePedestrianEvent
{
};

using Event = std::variant<CreatePedestrianEvent>;
