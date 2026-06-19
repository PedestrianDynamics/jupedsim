#pragma once
#include "CustomModelData.hpp"
#include "CustomModelUpdate.hpp"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <map>
#include <string>

namespace py = pybind11;

class PythonModelUpdate : public ICustomModelUpdateImpl
{
public:
    PythonModelUpdate(py::object impl) : impl(impl) {}
    ~PythonModelUpdate() = default;

    py::object impl;

    /// Extract key-value pairs from a Python object (dict, dataclass, or object with __dict__)
    /// Returns a map<string, py::object> of all attributes found.
    virtual std::any extract_attributes() const override
    {
        std::map<std::string, py::object> result;
        py::gil_scoped_acquire gil;

        // Try __dict__ first (fast path for objects with instance dictionary)
        if(py::hasattr(impl, "__dict__")) {
            try {
                py::dict d = impl.attr("__dict__");
                for(auto item : d) {
                    std::string key = py::cast<std::string>(item.first);
                    result[key] = py::cast<py::object>(item.second);
                }
                return result;
            } catch(const py::error_already_set&) {
                throw SimulationError("Error extracting attributes from object");
            }
        }

        // Try treating as dict-like (has .items() method)
        if(py::hasattr(impl, "items")) {
            try {
                py::object items_method = impl.attr("items");
                py::object items_result = items_method();
                for(auto item : items_result) {
                    py::tuple kv = py::cast<py::tuple>(item);
                    if(kv.size() == 2) {
                        std::string key = py::cast<std::string>(kv[0]);
                        result[key] = py::cast<py::object>(kv[1]);
                    }
                }
                return result;
            } catch(const py::error_already_set&) {
                throw SimulationError("Error extracting attributes from dict-like object");
            }
        }

        // Try dir() as last resort for anything with attributes
        try {
            py::object builtins = py::module_::import("builtins");
            py::list names = builtins.attr("dir")(impl);
            for(auto h : names) {
                std::string name = py::cast<std::string>(h);
                // Skip private/dunder and special attributes
                if(name[0] != '_') {
                    try {
                        py::object val = impl.attr(name.c_str());
                        // Skip callables (methods, functions, lambdas)
                        // Check if it's callable using Python's callable() built-in
                        py::object builtins_callable =
                            py::module_::import("builtins").attr("callable");
                        bool is_callable = py::cast<bool>(builtins_callable(val));
                        if(!is_callable) {
                            result[name] = val;
                        }
                    } catch(const py::error_already_set&) {
                        throw SimulationError("Error extracting attribute '{}' from object", name);
                    }
                }
            }
        } catch(const py::error_already_set&) {
            throw SimulationError("Error extracting attributes from object");
        }

        return result;
    }
};