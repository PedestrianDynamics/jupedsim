#include "util/UniqueID.h"

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
