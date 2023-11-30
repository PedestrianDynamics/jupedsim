// Copyright © 2012-2023 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once
#include <jupedsim/jupedsim.h>

#define OWNED_WRAPPER(cls)                                                                         \
    struct cls##_Wrapper {                                                                         \
        cls handle;                                                                                \
        cls##_Wrapper(cls h) : handle(h)                                                           \
        {                                                                                          \
        }                                                                                          \
        ~cls##_Wrapper()                                                                           \
        {                                                                                          \
            cls##_Free(handle);                                                                    \
        }                                                                                          \
        cls##_Wrapper(const cls##_Wrapper&) = delete;                                              \
        cls##_Wrapper& operator=(const cls##_Wrapper&) = delete;                                   \
        cls##_Wrapper(cls##_Wrapper&&) = delete;                                                   \
        cls##_Wrapper& operator=(cls##_Wrapper&&) = delete;                                        \
    }

#define WRAPPER(cls)                                                                               \
    struct cls##_Wrapper {                                                                         \
        cls handle;                                                                                \
        cls##_Wrapper(cls h) : handle(h)                                                           \
        {                                                                                          \
        }                                                                                          \
        ~cls##_Wrapper() = default;                                                                \
        cls##_Wrapper(const cls##_Wrapper&) = delete;                                              \
        cls##_Wrapper& operator=(const cls##_Wrapper&) = delete;                                   \
        cls##_Wrapper(cls##_Wrapper&&) = delete;                                                   \
        cls##_Wrapper& operator=(cls##_Wrapper&&) = delete;                                        \
    }

// Public types
OWNED_WRAPPER(JPS_Geometry);
OWNED_WRAPPER(JPS_GeometryBuilder);
OWNED_WRAPPER(JPS_OperationalModel);
OWNED_WRAPPER(JPS_CollisionFreeSpeedModelBuilder);
OWNED_WRAPPER(JPS_GeneralizedCentrifugalForceModelBuilder);
OWNED_WRAPPER(JPS_JourneyDescription);
OWNED_WRAPPER(JPS_Transition);
OWNED_WRAPPER(JPS_Simulation);
OWNED_WRAPPER(JPS_AgentIterator);
OWNED_WRAPPER(JPS_AgentIdIterator);
OWNED_WRAPPER(JPS_RoutingEngine);
OWNED_WRAPPER(JPS_NotifiableQueueProxy);
OWNED_WRAPPER(JPS_WaitingSetProxy);
OWNED_WRAPPER(JPS_WaypointProxy);
OWNED_WRAPPER(JPS_ExitProxy);
OWNED_WRAPPER(JPS_DirectSteeringProxy);
WRAPPER(JPS_Agent);
WRAPPER(JPS_GeneralizedCentrifugalForceModelState);
WRAPPER(JPS_CollisionFreeSpeedModelState);
