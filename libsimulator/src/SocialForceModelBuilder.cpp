#include "SocialForceModelBuilder.hpp"
#include "SocialForceModel.hpp"

SocialForceModelBuilder::SocialForceModelBuilder(
    double bodyForce, double friction)
    : _bodyForce(bodyForce), _friction(friction)
{
}

SocialForceModel SocialForceModelBuilder::Build()
{
    return SocialForceModel(
        _bodyForce, _friction);
}
