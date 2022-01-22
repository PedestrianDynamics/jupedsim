#include "util/UniqueID.hpp"

#include <fmt/format.h>
#include <gtest/gtest.h>
#include <memory>

using namespace fmt::literals;
using ::jps::UniqueID;

TEST(UniqueId, DefaultConstructedIDsAreNotIdentical)
{
    const auto first  = jps::UniqueID<void>{};
    const auto second = jps::UniqueID<void>{};
    const auto third  = jps::UniqueID<void>{};
    ASSERT_NE(first, second);
    ASSERT_NE(first, third);
    ASSERT_NE(second, third);
}

TEST(UniqueId, CanCompareLessThan)
{
    const auto id1 = jps::UniqueID<void>{};
    const auto id2 = jps::UniqueID<void>{};
    ASSERT_LT(id1, id2);
    ASSERT_FALSE(id2 < id1);
}

TEST(UniqueId, CanCompareGreaterThan)
{
    const auto id1 = jps::UniqueID<void>{};
    const auto id2 = jps::UniqueID<void>{};
    ASSERT_GT(id2, id1);
    ASSERT_FALSE(id1 > id2);
}

TEST(UniqueId, CanCompareLessOrEqual)
{
    const auto id1 = jps::UniqueID<void>{};
    const auto id2 = jps::UniqueID<void>{};
    ASSERT_LE(id1, id2);
    ASSERT_LE(id1, id1);
    ASSERT_FALSE(id2 <= id1);
}

TEST(UniqueId, CanCompareGreaterOrEqual)
{
    const auto id1 = jps::UniqueID<void>{};
    const auto id2 = jps::UniqueID<void>{};
    ASSERT_GT(id2, id1);
    ASSERT_GE(id2, id2);
    ASSERT_FALSE(id1 >= id2);
}

TEST(UniqueId, CanBeMovedAndCopied)
{
    auto first               = jps::UniqueID<void>{};
    const auto copy_of_first = first;
    const auto second        = jps::UniqueID<void>{};
    ASSERT_NE(first, second);
    ASSERT_NE(copy_of_first, second);
    ASSERT_EQ(first, copy_of_first);
    const auto moved_into{std::move(first)};
    ASSERT_NE(moved_into, second);
    ASSERT_NE(copy_of_first, second);
    ASSERT_EQ(moved_into, copy_of_first);
}

TEST(UniqueId, CanBeFormatted)
{
    // create this type to ensure the internal counter of id creation for UniqueId<Foo> starts at 0;
    struct Foo {
    };
    jps::UniqueID<Foo> id;
    ASSERT_EQ("1", "{}"_format(id));
}
