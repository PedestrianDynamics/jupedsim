// SPDX-License-Identifier: LGPL-3.0-or-later
#include "CollisionGeometry.hpp"
#include "GenericAgent.hpp"
#include "NeighborQueries.hpp"
#include "NeighborhoodSearch.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <iterator>
#include <vector>

namespace
{
PolyWithHoles constructPolyFromPoints(const std::vector<Point>& points)
{
    using CGALPoint = PolyWithHoles::Polygon_2::Point_2;
    std::vector<CGALPoint> cgalPoints{};
    cgalPoints.reserve(points.size());
    std::transform(
        std::begin(points), std::end(points), std::back_inserter(cgalPoints), [](const auto& p) {
            return CGALPoint{p.x, p.y};
        });
    return PolyWithHoles(Poly{cgalPoints.begin(), cgalPoints.end()});
}

GenericAgent makeAgent(Point position)
{
    CollisionFreeSpeedModelState state{};
    state.position = position;
    return GenericAgent{
        GenericAgent::ID::Invalid,
        jps::UniqueID<Journey>::Invalid,
        jps::UniqueID<BaseStage>::Invalid,
        state};
}

bool containsId(const std::vector<GenericAgent>& agents, GenericAgent::ID id)
{
    return std::any_of(
        std::begin(agents), std::end(agents), [id](const auto& a) { return a.id == id; });
}
} // namespace

class NeighborQueryInUShapedRoom : public ::testing::Test
{
protected:
    /// Accessible area shaped like a "U": two arms (x in [0,4] and [6,10]) connected at the
    /// bottom (y in [0,2]); the slot between the arms blocks sight lines between them.
    CollisionGeometry geometry;
    AgentContainer<GenericAgent> agents{};
    NeighborhoodSearch<GenericAgent> neighborhoodSearch{2.2};

    GenericAgent::ID self{GenericAgent::ID::Invalid};
    GenericAgent::ID visibleNeighbor{GenericAgent::ID::Invalid};
    GenericAgent::ID occludedNeighbor{GenericAgent::ID::Invalid};

    const Point selfPosition{2, 9};

    NeighborQueryInUShapedRoom()
        : geometry(constructPolyFromPoints(
              {{0, 0}, {10, 0}, {10, 10}, {6, 10}, {6, 2}, {4, 2}, {4, 10}, {0, 10}}))
    {
        agents.push_back(makeAgent(selfPosition)); // left arm
        agents.push_back(makeAgent({2, 7})); // same arm, in sight
        agents.push_back(makeAgent({8, 9})); // right arm, behind the slot
        self = agents[0].id;
        visibleNeighbor = agents[1].id;
        occludedNeighbor = agents[2].id;
        neighborhoodSearch.Update(agents);
    }
};

TEST_F(NeighborQueryInUShapedRoom, VisibleQueryExcludesBoundAgent)
{
    const VisibleNeighborQuery query(self, geometry, neighborhoodSearch);
    const auto result = query(selfPosition, 8);
    EXPECT_FALSE(containsId(result, self));
}

TEST_F(NeighborQueryInUShapedRoom, VisibleQueryExcludesOccludedNeighbor)
{
    const VisibleNeighborQuery query(self, geometry, neighborhoodSearch);
    const auto result = query(selfPosition, 8);
    EXPECT_FALSE(containsId(result, occludedNeighbor));
}

TEST_F(NeighborQueryInUShapedRoom, VisibleQueryContainsVisibleNeighborInRange)
{
    const VisibleNeighborQuery query(self, geometry, neighborhoodSearch);
    const auto result = query(selfPosition, 8);
    EXPECT_TRUE(containsId(result, visibleNeighbor));
    EXPECT_EQ(result.size(), 1);
}

TEST_F(NeighborQueryInUShapedRoom, VisibleQueryRespectsRadius)
{
    const VisibleNeighborQuery query(self, geometry, neighborhoodSearch);
    // The visible neighbor is 2m away.
    const auto result = query(selfPosition, 1);
    EXPECT_TRUE(result.empty());
}

TEST_F(NeighborQueryInUShapedRoom, ProximityQueryContainsOccludedNeighbor)
{
    const ProximityNeighborQuery query(self, neighborhoodSearch);
    const auto result = query(selfPosition, 8);
    EXPECT_TRUE(containsId(result, occludedNeighbor));
    EXPECT_TRUE(containsId(result, visibleNeighbor));
    EXPECT_EQ(result.size(), 2);
}

TEST_F(NeighborQueryInUShapedRoom, ProximityQueryExcludesBoundAgent)
{
    const ProximityNeighborQuery query(self, neighborhoodSearch);
    const auto result = query(selfPosition, 8);
    EXPECT_FALSE(containsId(result, self));
}

TEST_F(NeighborQueryInUShapedRoom, ProximityQueryRespectsRadius)
{
    const ProximityNeighborQuery query(self, neighborhoodSearch);
    // The occluded neighbor is 6m away, the visible one 2m.
    const auto result = query(selfPosition, 3);
    EXPECT_FALSE(containsId(result, occludedNeighbor));
    EXPECT_TRUE(containsId(result, visibleNeighbor));
}

TEST_F(NeighborQueryInUShapedRoom, ProximityQueryWithInvalidIdExcludesNobody)
{
    const ProximityNeighborQuery query(GenericAgent::ID::Invalid, neighborhoodSearch);
    const auto result = query(selfPosition, 8);
    EXPECT_EQ(result.size(), agents.size());
}
