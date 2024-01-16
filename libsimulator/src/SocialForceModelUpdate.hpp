#include "Point.hpp"

#include <optional>

struct SocialForceModelUpdate {
    std::optional<Point> position{};
    std::optional<Point> velocity{};
};