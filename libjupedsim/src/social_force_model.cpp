#include "jupedsim/social_force_model.h"
#include "jupedsim/error.h"

#include "Conversion.hpp"
#include "ErrorMessage.hpp"

#include <SocialForceModel.hpp>
#include <SocialForceModelBuilder.hpp>
#include <SocialForceModelData.hpp>

using jupedsim::detail::intoJPS_Point;
using jupedsim::detail::intoPoint;
using jupedsim::detail::intoTuple;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// SocialForceModel Model Builder
////////////////////////////////////////////////////////////////////////////////////////////////////
JPS_SocialForceModelBuilder JPS_SocialForceModelBuilder_Create(
    double test_value)
{
    return reinterpret_cast<JPS_SocialForceModelBuilder>(
        new SocialForceModelBuilder(
            test_value));
}

JPS_OperationalModel JPS_SocialForceModelBuilder_Build(
    JPS_SocialForceModelBuilder handle,
    JPS_ErrorMessage* errorMessage)
{
    assert(handle != nullptr);
    auto builder = reinterpret_cast<SocialForceModelBuilder*>(handle);
    JPS_OperationalModel result{};
    try {
        result = reinterpret_cast<JPS_OperationalModel>(
            new SocialForceModel(builder->Build()));
    } catch(const std::exception& ex) {
        if(errorMessage) {
            *errorMessage = reinterpret_cast<JPS_ErrorMessage>(new JPS_ErrorMessage_t{ex.what()});
        }
    } catch(...) {
        if(errorMessage) {
            *errorMessage = reinterpret_cast<JPS_ErrorMessage>(
                new JPS_ErrorMessage_t{"Unknown internal error."});
        }
    }
    return result;
}

void JPS_SocialForceModelBuilder_Free(
    JPS_SocialForceModelBuilder handle)
{
    delete reinterpret_cast<SocialForceModelBuilder*>(handle);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// SocialForceModelState
////////////////////////////////////////////////////////////////////////////////////////////////////
double
JPS_SocialForceModelState_GetTestValue(JPS_SocialForceModelState handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const SocialForceModelData*>(handle);
    return state->test_value;
}
void JPS_SocialForceModelState_SetTestValue(
    JPS_SocialForceModelState handle,
    double test_value)
{
    assert(handle);
    const auto state = reinterpret_cast<SocialForceModelData*>(handle);
    state->test_value = test_value;
}