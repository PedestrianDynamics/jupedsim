#pragma once
#include "Point.hpp"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

class PythonModelUpdate
{
    using AnyType = std::variant<int, double, std::string, bool, Point>;
    std::unordered_map<std::string, AnyType> data;

public:
    void set(const std::string& key, const py::object& value)
    {
        if(py::isinstance<py::bool_>(value)) {
            data[key] = value.cast<bool>();
        } else if(py::isinstance<py::int_>(value)) {
            data[key] = value.cast<int>();
        } else if(py::isinstance<py::float_>(value)) {
            data[key] = value.cast<double>();
        } else if(py::isinstance<py::str>(value)) {
            data[key] = value.cast<std::string>();
        } else if(py::isinstance<py::tuple>(value)) {
            // Convert tuple of floats to Point
            auto tuple = value.cast<py::tuple>();
            if(tuple.size() == 2) {
                try {
                    double x = tuple[0].cast<double>();
                    double y = tuple[1].cast<double>();
                    data[key] = Point(x, y);
                } catch(const std::exception&) {
                    // If conversion fails, skip
                }
            }
        } else if(py::isinstance<py::object>(value)) {
            // Try to cast to Point if it's a Point object
            try {
                auto point = value.cast<Point>();
                data[key] = point;
            } catch(const std::exception&) {
                // If not a Point, ignore
            }
        }
    }

    py::object get(const std::string& key)
    {
        auto it = data.find(key);
        if(it == data.end())
            return py::none();

        const auto& value = it->second;
        if(std::holds_alternative<int>(value)) {
            return py::int_(std::get<int>(value));
        } else if(std::holds_alternative<double>(value)) {
            return py::float_(std::get<double>(value));
        } else if(std::holds_alternative<bool>(value)) {
            return py::bool_(std::get<bool>(value));
        } else if(std::holds_alternative<std::string>(value)) {
            return py::str(std::get<std::string>(value));
        } else if(std::holds_alternative<Point>(value)) {
            // Return Point as a tuple (x, y)
            const auto& point = std::get<Point>(value);
            return py::make_tuple(point.x, point.y);
        }
        return py::none();
    }
};