#pragma once
#include "CustomModelData.hpp"
#include "CustomModelUpdate.hpp"
#include "OperationalModel.hpp"
#include "Point.hpp"
#include "SimulationError.hpp"
#include "conversion.hpp"
#include "python_model_data.hpp"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

class PythonModelData : public ICustomModelDataImpl
{
public:
    PythonModelData(py::object impl) : impl(impl) {}
    virtual ~PythonModelData() override = default;

    py::object impl;

    std::string repr() const override
    {
        py::gil_scoped_acquire gil;
        try {
            py::object repr_obj = impl.attr("__repr__")();
            return py::cast<std::string>(repr_obj);
        } catch(const py::error_already_set&) {
            return "<PythonModelData: error in __repr__>";
        }
    }

    static void set_attributes(const std::map<std::string, py::object>& attrs, py::object& obj)
    {
        py::gil_scoped_acquire gil;
        for(const auto& [key, value] : attrs) {
            if(py::hasattr(obj, "__dict__")) {
                try {
                    obj.attr(py::str(key)) = value;
                } catch(const py::error_already_set&) {
                    throw SimulationError(
                        "Error applying update for attribute '{}': value cannot be set on "
                        "object",
                        key);
                } catch(const py::cast_error&) {
                    throw SimulationError(
                        "Error applying update for attribute '{}': value cannot be cast to "
                        "expected type",
                        key);
                }
            }

            // Try treating as dict-like (has .items() method)
            if(py::hasattr(obj, "keys")) {
                try {

                    obj[py::str(key)] = value;
                } catch(const py::error_already_set&) {
                    throw SimulationError(
                        "Error applying update for attribute '{}': value cannot be set on "
                        "dict-like object",
                        key);
                } catch(const py::cast_error&) {
                    throw SimulationError(
                        "Error applying update for attribute '{}': value cannot be cast to "
                        "expected type",
                        key);
                }
            }
        }
    }

    void apply_update(const CustomModelUpdate& upd) override
    {
        set_attributes(upd.extract_attributes(), impl);
    }
};
