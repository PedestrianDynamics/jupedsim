// SPDX-License-Identifier: LGPL-3.0-or-later
#include "CollisionGeometry.hpp"
#include "GenericAgent.hpp"
#include "GeometricFunctions.hpp"
#include "LineSegment.hpp"
#include "NeighborQueries.hpp"
#include "NeighborhoodSearch.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <iterator>
#include <variant>
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

GenericAgent makeAgent(Point position, double v0)
{
    CollisionFreeSpeedModelState state{};
    state.position = position;
    state.v0 = v0;
    return GenericAgent{
        GenericAgent::ID::Invalid,
        jps::UniqueID<Journey>::Invalid,
        jps::UniqueID<BaseStage>::Invalid,
        state};
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
        // Distinct v0 values so state comparisons cover more than the position.
        agents.push_back(makeAgent(selfPosition, 1.0)); // left arm
        agents.push_back(makeAgent({2, 7}, 1.1)); // same arm, in sight
        agents.push_back(makeAgent({8, 9}, 1.3)); // right arm, behind the slot
        self = agents[0].id;
        visibleNeighbor = agents[1].id;
        occludedNeighbor = agents[2].id;
        neighborhoodSearch.Update(agents);
    }

    /// True when `result` contains the state of the fixture agent with `id`, matched by
    /// position (returned states carry no identity).
    bool containsId(const std::vector<OperationalModelState>& result, GenericAgent::ID id) const
    {
        const auto agent = std::find_if(
            std::begin(agents), std::end(agents), [id](const auto& a) { return a.id == id; });
        if(agent == std::end(agents)) {
            return false;
        }
        const auto& pos = agent->position();
        return std::any_of(std::begin(result), std::end(result), [&pos](const auto& state) {
            return Pos(state) == pos;
        });
    }

    /// Reference result: query the NeighborhoodSearch directly, filter manually (skip the
    /// bound agent, optionally drop occluded neighbors), and extract the model states.
    std::vector<OperationalModelState>
    manuallyFilteredStates(Point position, double radius, bool filterVisibility) const
    {
        auto reference = neighborhoodSearch.GetNeighboringAgents(position, radius);
        const auto& boundary = geometry.LineSegmentsInApproxDistanceTo(position);
        std::erase_if(reference, [&, this](const auto& neighbor) {
            if(neighbor.id == self) {
                return true;
            }
            if(!filterVisibility) {
                return false;
            }
            const auto sight_line = LineSegment(position, neighbor.position());
            return std::any_of(
                boundary.cbegin(), boundary.cend(), [&sight_line](const auto& boundary_segment) {
                    return intersects(sight_line, boundary_segment);
                });
        });
        std::vector<OperationalModelState> states;
        states.reserve(reference.size());
        std::transform(
            std::begin(reference),
            std::end(reference),
            std::back_inserter(states),
            [](const auto& agent) { return agent.model; });
        return states;
    }

    static void expectSameStates(
        const std::vector<OperationalModelState>& actual,
        const std::vector<OperationalModelState>& expected)
    {
        ASSERT_EQ(actual.size(), expected.size());
        for(size_t index = 0; index < actual.size(); ++index) {
            EXPECT_EQ(
                std::get<CollisionFreeSpeedModelState>(actual[index]),
                std::get<CollisionFreeSpeedModelState>(expected[index]))
                << "state mismatch at index " << index;
        }
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

TEST_F(NeighborQueryInUShapedRoom, VisibleQueryMatchesManuallyFilteredNeighborhoodSearch)
{
    const VisibleNeighborQuery query(self, geometry, neighborhoodSearch);
    for(const double radius : {1.0, 3.0, 8.0}) {
        expectSameStates(
            query(selfPosition, radius),
            manuallyFilteredStates(selfPosition, radius, /*filterVisibility=*/true));
    }
}

TEST_F(NeighborQueryInUShapedRoom, ProximityQueryMatchesManuallyFilteredNeighborhoodSearch)
{
    const ProximityNeighborQuery query(self, neighborhoodSearch);
    for(const double radius : {1.0, 3.0, 8.0}) {
        expectSameStates(
            query(selfPosition, radius),
            manuallyFilteredStates(selfPosition, radius, /*filterVisibility=*/false));
    }
}
