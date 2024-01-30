#pragma once

#include "SocialForceModel.hpp"
class SocialForceModelBuilder
{
    double _bodyForce;
    double _friction;

public:
    SocialForceModelBuilder(double bodyForce, double friction);
    SocialForceModel Build();
};