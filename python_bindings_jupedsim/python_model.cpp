#include "CollisionGeometry.hpp"
#include "GenericAgent.hpp"
#include "NeighborhoodSearch.hpp"
#include "OperationalModel.hpp"
#include "PythonModel.hpp"
#include "PythonModelData.hpp"
#include "PythonModelUpdate.hpp"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

class PythonModelTramp : public PythonModel, public pybind11::trampoline_self_life_support
{
public:
    using PythonModel::PythonModel;

    virtual OperationalModelType Type() const override { return OperationalModelType::PYTHON; }
    virtual OperationalModelUpdate ComputeNewPosition(
        double dT,
        const GenericAgent& ped,
        const CollisionGeometry& geometry,
        const NeighborhoodSearch<GenericAgent>& neighborhoodSearch) const override
    {
        PYBIND11_OVERRIDE_PURE(
            OperationalModelUpdate, /* Return type */
            OperationalModel, /* Parent class */
            ComputeNewPosition, /* Name of function in C++ (must match Python name) */
            dT,
            ped,
            geometry,
            neighborhoodSearch /* Arguments */
        );
    }

    virtual void
    ApplyUpdate(const OperationalModelUpdate& update, GenericAgent& agent) const override
    {
        PYBIND11_OVERRIDE_PURE(
            void, /* Return type */
            OperationalModel, /* Parent class */
            ApplyUpdate, /* Name of function in C++ (must match Python name) */
            update,
            agent /* Arguments */
        );
    }

    virtual void CheckModelConstraint(
        const GenericAgent& agent,
        const NeighborhoodSearch<GenericAgent>& neighborhoodSearch,
        const CollisionGeometry& geometry) const override
    {
        PYBIND11_OVERRIDE_PURE(
            void, /* Return type */
            OperationalModel, /* Parent class */
            CheckModelConstraint, /* Name of function in C++ (must match Python name) */
            agent,
            neighborhoodSearch,
            geometry /* Arguments */
        );
    }

    std::unique_ptr<OperationalModel> Clone() const override
    {
        return std::make_unique<PythonModelTramp>(*this);
    }
};

void init_python_model(py::module_& m)
{
    py::class_<PythonModelData>(m, "PythonModelData").def(py::init<>());
    py::class_<PythonModelUpdate>(m, "PythonModelUpdate")
        .def(py::init<>())
        .def("set", &PythonModelUpdate::set)
        .def("get", &PythonModelUpdate::get);
    py::class_<OperationalModel, PythonModelTramp, py::smart_holder>(m, "OperationalModel")
        .def(py::init<>())
        .def(
            "ComputeNewPosition",
            &OperationalModel::ComputeNewPosition,
            py::arg("dT"),
            py::arg("ped"),
            py::arg("geometry"),
            py::arg("neighborhood_search"))
        .def(
            "ApplyUpdate",
            [](OperationalModel& self, const PythonModelUpdate& update, GenericAgent& agent) {
                self.ApplyUpdate(update, agent);
            })
        .def(
            "CheckModelConstraint",
            &OperationalModel::CheckModelConstraint,
            py::arg("ped"),
            py::arg("neighborhood_search"),
            py::arg("geometry"));
}
