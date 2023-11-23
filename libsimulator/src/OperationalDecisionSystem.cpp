#include "OperationalDecisionSystem.hpp"

std::unique_ptr<OperationalDecisionSystemInterface>
MakeOperationalDecisionSystem(std::unique_ptr<OperationalModel>&& model)
{
    if(dynamic_cast<OptimalStepsModel*>(model.get()) != nullptr) {
        return {};
    } else {
        return std::make_unique<OperationalDecisionSystem>(std::move(model));
    }
}
