#include <IO/IniFileParser.hpp>

#include <tinyxml.h>

#include <gtest/gtest.h>

const std::string example = R"(
    <areas>
        <area id="1">
            <lables>
                <label>exit</label>
            </lables>
            <polygon>
                <vertex px="70" py="101" />
                <vertex px="70" py="103" />
                <vertex px="75" py="103" />
                <vertex px="75" py="101" />
            </polygon>
        </area>
    </areas>
)";

struct Fixture : ::testing::Test {

    TiXmlDocument doc{};

    void SetUp() override { doc.Parse(example.c_str()); }
    void TearDown() override {}
};

TEST_F(Fixture, TestA)
{
    const auto root = doc.RootElement();
    const auto areas = ParseAreas(root);
    ASSERT_EQ(areas.size(), 1);
    const auto& area1 = areas.find(1)->second;
    ASSERT_EQ(area1.lables.size(), 1);
    ASSERT_EQ(area1.lables.count("exit"), 1);
    const std::vector<Point> expectedPolygon = {{70, 101}, {70, 103}, {75, 103}, {75, 101}};
    ASSERT_EQ(area1.polygon.points[0], expectedPolygon[0]);
    ASSERT_EQ(area1.polygon.points[1], expectedPolygon[1]);
    ASSERT_EQ(area1.polygon.points[2], expectedPolygon[2]);
    ASSERT_EQ(area1.polygon.points[3], expectedPolygon[3]);
}
