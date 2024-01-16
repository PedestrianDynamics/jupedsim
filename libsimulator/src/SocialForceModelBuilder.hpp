#pragma once

#include "SocialForceModel.hpp"
class SocialForceModelBuilder
{
    double _testVal;

public:
    SocialForceModelBuilder(double test_value);
    SocialForceModel Build();
};