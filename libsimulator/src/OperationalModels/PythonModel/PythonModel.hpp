#include "GenericAgent.hpp"
#include "NeighborhoodSearch.hpp"
#include "OperationalModel.hpp"
#include "PythonModelData.hpp"
#include "PythonModelUpdate.hpp"
#include "SimulationError.hpp"

#include <fmt/core.h>
#include <fmt/format.h>

inline Point intoPoint(const std::tuple<double, double>& tup)
{
    return Point{std::get<0>(tup), std::get<1>(tup)};
}

class PythonModel : public OperationalModel
{
public:
    using OperationalModel::OperationalModel;

    virtual void
    ApplyUpdate(const OperationalModelUpdate& update, GenericAgent& agent) const override
    {
        const auto& upd = std::get<PythonModelUpdate>(update);
        auto& model = std::get<PythonModelData>(agent.model);

        // Extract all attributes from the update object (handles dicts, dataclasses, objects)
        auto attrs = PythonModelUpdate::extract_attributes(upd.impl);
        // Merge modelAttrs into attrs, giving precedence to update attributes

        // Apply each attribute to both the model data and the agent
        for(const auto& [key, val] : attrs) {
            try {
                if(key == "position") {
                    agent.pos = intoPoint(pybind11::cast<std::tuple<double, double>>(val));
                } else if(key == "orientation") {
                    agent.orientation = intoPoint(pybind11::cast<std::tuple<double, double>>(val));
                }
            } catch(const pybind11::error_already_set&) {
                throw SimulationError("Error applying update for attribute {} ", key);

                // Silently ignore attributes that can't be cast or set
            } catch(const pybind11::cast_error&) {
                // Silently ignore cast errors for non-convertible types
                throw SimulationError(
                    "Error applying update for attribute {} : value cannot be cast to expected "
                    "type",
                    key);
            }
        }
        PythonModelUpdate::set_attributes(attrs, model.impl);
    }
};
