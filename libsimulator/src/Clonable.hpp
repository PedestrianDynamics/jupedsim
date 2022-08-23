#pragma once

#include <memory>

template <typename T>
class Clonable
{
public:
    virtual std::unique_ptr<T> Clone() const = 0;
};
