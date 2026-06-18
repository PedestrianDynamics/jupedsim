#include "CollisionGeometry.hpp"
#include "CustomModelData.hpp"
#include "CustomModelUpdate.hpp"
#include "GenericAgent.hpp"
#include "NeighborhoodSearch.hpp"
#include "OperationalModel.hpp"
#include "python_model_data.hpp"
#include "python_model_update.hpp"

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

    using PythonModel::ApplyUpdate;

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
    py::class_<PythonModelData>(m, "PythonModelState")
        .def(py::init<py::object>(), py::arg("py_object"))
        .def_readwrite("py_object", &PythonModelData::impl);
    py::class_<CustomModelData>(m, "CustomModelData")
        .def(py::init([](PythonModelData data) {
            std::shared_ptr<ICustomModelDataImpl> impl = std::make_shared<PythonModelData>(data);
            return CustomModelData(std::move(impl));
        }))
        .def("repr", &CustomModelData::repr)
        .def("get_impl", &CustomModelData::get_impl);
    py::class_<CustomModelUpdate>(m, "CustomModelUpdate")
        .def(py::init([](PythonModelUpdate update) {
            std::shared_ptr<ICustomModelUpdateImpl> impl =
                std::make_shared<PythonModelUpdate>(update);
            return CustomModelUpdate(std::move(impl));
        }));
    py::class_<PythonModelUpdate>(m, "PythonModelUpdate")
        .def(py::init<py::object>(), py::arg("py_object"))
        .def_readwrite("py_object", &PythonModelUpdate::impl);
    py::class_<OperationalModel, PythonModelTramp, py::smart_holder>(m, "OperationalModel")
        .def(py::init<>())
        .def(
            "ComputeNewPosition",
            [](const OperationalModel& self,
               double dT,
               const GenericAgent& ped,
               const CollisionGeometry& geometry,
               const NeighborhoodSearch<GenericAgent>& neighborhood_search) {
                return self.ComputeNewPosition(dT, ped, geometry, neighborhood_search);
            },
            py::arg("dT"),
            py::arg("ped"),
            py::arg("geometry"),
            py::arg("neighborhood_search"))
        .def(
            "CheckModelConstraint",
            &OperationalModel::CheckModelConstraint,
            py::arg("ped"),
            py::arg("neighborhood_search"),
            py::arg("geometry"));
}
