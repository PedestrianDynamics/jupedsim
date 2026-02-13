// SPDX-License-Identifier: LGPL-3.0-or-later
#include "GeneralizedCentrifugalForceModelBuilder.hpp"

#include <gtest/gtest.h>

#include <stdexcept>

// Valid baseline parameters:
//   nuped=0.3, nuwall=0.2,
//   dist_effPed=2.0, dist_effWall=2.0,
//   intp_widthped=0.1, intp_widthwall=0.1,
//   maxfped=3.0, maxfwall=3.0

TEST(GCFMParameterValidation, RejectsNeighborInteractionEqualToInterpolation)
{
    // dist_effPed == intp_widthped → dist_intpol_right = 0 → division by zero
    GeneralizedCentrifugalForceModelBuilder builder(0.3, 0.2, 1.0, 2.0, 1.0, 0.1, 3.0, 3.0);
    ASSERT_THROW(builder.Build(), std::invalid_argument);
}

TEST(GCFMParameterValidation, RejectsNeighborInteractionLessThanInterpolation)
{
    GeneralizedCentrifugalForceModelBuilder builder(0.3, 0.2, 0.5, 2.0, 1.0, 0.1, 3.0, 3.0);
    ASSERT_THROW(builder.Build(), std::invalid_argument);
}

TEST(GCFMParameterValidation, RejectsGeometryInteractionEqualToInterpolation)
{
    // dist_effWall == intp_widthwall
    GeneralizedCentrifugalForceModelBuilder builder(0.3, 0.2, 2.0, 1.0, 0.1, 1.0, 3.0, 3.0);
    ASSERT_THROW(builder.Build(), std::invalid_argument);
}

TEST(GCFMParameterValidation, RejectsGeometryInteractionLessThanInterpolation)
{
    GeneralizedCentrifugalForceModelBuilder builder(0.3, 0.2, 2.0, 0.5, 0.1, 1.0, 3.0, 3.0);
    ASSERT_THROW(builder.Build(), std::invalid_argument);
}

TEST(GCFMParameterValidation, AcceptsValidParameters)
{
    GeneralizedCentrifugalForceModelBuilder builder(0.3, 0.2, 2.0, 2.0, 0.1, 0.1, 3.0, 3.0);
    ASSERT_NO_THROW(builder.Build());
}
