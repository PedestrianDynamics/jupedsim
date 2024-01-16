#include "SocialForceModelBuilder.hpp"
#include "SocialForceModel.hpp"

SocialForceModelBuilder::SocialForceModelBuilder(
    double test_value)
    : _testVal(test_value)
{
}

SocialForceModel SocialForceModelBuilder::Build()
{
    return SocialForceModel(
        _testVal);
}
