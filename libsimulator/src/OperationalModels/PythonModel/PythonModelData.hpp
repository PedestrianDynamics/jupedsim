#pragma once

#include <pybind11/pybind11.h>

namespace py = pybind11;

class PythonModelData
{
public:
    PythonModelData(py::object _impl) : impl(_impl) {};
    ~PythonModelData() = default;

    py::object impl;
};

template <>
struct fmt::formatter<PythonModelData> : fmt::formatter<std::string> {
    template <typename FormatContext>
    auto format(const PythonModelData& data, FormatContext& ctx) const
    {
        py::gil_scoped_acquire gil;
        std::string repr = py::str(data.impl);
        return fmt::formatter<std::string>::format(repr, ctx);
    }
};