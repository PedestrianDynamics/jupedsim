// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once
#include "Point.hpp"
#include "SimulationError.hpp"

#include <fmt/core.h>
#include <fmt/format.h>

#include <any>
#include <map>
#include <memory>
#include <string>

class ICustomModelUpdateImpl
{
public:
    virtual ~ICustomModelUpdateImpl() {};
    virtual std::any extract_attributes() const = 0;
};

class CustomModelUpdate
{
public:
    explicit CustomModelUpdate(std::shared_ptr<ICustomModelUpdateImpl> impl)
        : impl_(std::move(impl))
    {
    }
    ~CustomModelUpdate() = default;

    std::any extract_attributes() const
    {
        if(impl_) {
            return impl_->extract_attributes();
        }
        return {};
    }

private:
    std::shared_ptr<ICustomModelUpdateImpl> impl_;
};
