#include "GenericAgent.hpp"
#include "NeighborhoodSearch.hpp"
#include "OperationalModel.hpp"
#include "PythonModelData.hpp"

class PythonModel : public OperationalModel
{
public:
    using OperationalModel::OperationalModel;
};
