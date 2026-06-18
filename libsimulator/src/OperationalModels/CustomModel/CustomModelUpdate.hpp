#pragma once
#include "Point.hpp"
#include "SimulationError.hpp"

#include <fmt/core.h>
#include <fmt/format.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <map>
#include <string>

namespace py = pybind11;

class ICustomModelUpdateImpl
{
public:
    virtual ~ICustomModelUpdateImpl() {};
    virtual std::map<std::string, py::object> extract_attributes() const = 0;
};

class CustomModelUpdate
{
public:
    explicit CustomModelUpdate(std::shared_ptr<ICustomModelUpdateImpl> impl)
        : impl_(std::move(impl))
    {
    }
    ~CustomModelUpdate() = default;

    std::map<std::string, py::object> extract_attributes() const
    {
        if(impl_) {
            return impl_->extract_attributes();
        }
        return {};
    }

private:
    std::shared_ptr<ICustomModelUpdateImpl> impl_;
};
