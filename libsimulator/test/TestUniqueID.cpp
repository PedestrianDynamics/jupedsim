// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "UniqueID.hpp"

#include <fmt/format.h>
#include <gtest/gtest.h>
#include <memory>

using namespace fmt::literals;
using ::jps::UniqueID;

TEST(UniqueId, CanBeConstructedFromUnderlyingType)
{
    const jps::UniqueID<void> first_sentinel{};
    const jps::UniqueID<void> a{666};
    const jps::UniqueID<void> b{667};
    const jps::UniqueID<void> second_sentinel{};
    ASSERT_NE(a, b);
    ASSERT_EQ(a.getID(), 666);
    ASSERT_EQ(b.getID(), 667);
    ASSERT_EQ(first_sentinel.getID() + 1, second_sentinel.getID());
}

TEST(UniqueId, DefaultConstructedIDsAreNotIdentical)
{
    const auto first = jps::UniqueID<void>{};
    const auto second = jps::UniqueID<void>{};
    const auto third = jps::UniqueID<void>{};
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
    auto first = jps::UniqueID<void>{};
    const auto copy_of_first = first;
    const auto second = jps::UniqueID<void>{};
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
    ASSERT_EQ("1", fmt::format("{}", id));
}

TEST(UniqueId, CanBeHashed)
{
    auto hasher = std::hash<jps::UniqueID<void>>{};
    ASSERT_NO_FATAL_FAILURE(hasher(jps::UniqueID<void>{}));
}

TEST(UniqueId, InvalidValueIsNotFirstID)
{
    struct InvalidValueIsNotFirstID_Type {
    };
    using UID = UniqueID<InvalidValueIsNotFirstID_Type>;
    auto first = UID{};
    ASSERT_NE(UID::Invalid, first);
}

TEST(UniqueId, InvalidValueComparesToItself)
{
    struct InvalidValueComparesToItself_Type {
    };
    using UID = UniqueID<InvalidValueComparesToItself_Type>;
    auto first = UID{};
    ASSERT_EQ(UID::Invalid, UID::Invalid);
}
