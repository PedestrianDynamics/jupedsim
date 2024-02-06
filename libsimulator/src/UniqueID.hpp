// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <atomic>
#include <fmt/format.h>
#include <type_traits>

namespace jps
{
/// UniqueId allows tagging any type with a unique id.
///
/// This allows for type-safe ids to be embedded in your type so that you do not need to fall back
/// on integers while retaining the same size.
///
/// To use this extend to to be uniquely identifiable class like this:
/// ```
/// UniqueId<MyClass> id;
/// ```
/// Thread Safety: Identifiable is designed to be thread safe.
template <typename Tag, typename Integer = uint64_t>
class UniqueID
{
public:
    using underlying_type = Integer;

private:
    /// Static count for the next available unique identifier.
    // As there is a bug in clang-tidy (https://bugs.llvm.org/show_bug.cgi?id=48040) the nolint is
    // needed for now NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
    inline static std::atomic<Integer> uid_counter{0};
    /// The actual unique identifier of the object
    Integer m_value{++uid_counter};

public:
    /// Represents an invalid id
    static UniqueID<Tag, Integer> Invalid;

    UniqueID() = default;

    UniqueID(Integer id) : m_value(id) {}

    /// UniqueIds are copyable.
    UniqueID(UniqueID const& p_other) = default;

    /// UniqueIds are copyable.
    UniqueID& operator=(UniqueID const& p_other) = default;

    /// UniqueIds are movable.
    UniqueID(UniqueID&& p_other) noexcept = default;

    /// UniqueIds are movable.
    UniqueID& operator=(UniqueID&& p_other) noexcept = default;

    ~UniqueID() noexcept = default;

    Integer getID() const noexcept { return m_value; }

    bool operator==(const UniqueID& p_other) const noexcept { return m_value == p_other.m_value; };

    bool operator!=(const UniqueID& p_other) const noexcept { return m_value != p_other.m_value; };

    bool operator<(const UniqueID& p_other) const noexcept { return m_value < p_other.m_value; };

    bool operator>(const UniqueID& p_other) const noexcept { return p_other < *this; };

    bool operator<=(const UniqueID& p_other) const noexcept { return !(*this > p_other); };

    bool operator>=(const UniqueID& p_other) const noexcept { return !(*this < p_other); };

    friend struct fmt::formatter<UniqueID<Tag>>;
};

template <typename Tag, typename Integer>
UniqueID<Tag, Integer> UniqueID<Tag, Integer>::Invalid{0};

} // namespace jps

namespace std
{
template <typename Tag, typename Integer>
struct hash<jps::UniqueID<Tag, Integer>> {
    size_t operator()(const jps::UniqueID<Tag, Integer>& x) const
    {
        return std::hash<Integer>{}(x.getID());
    }
};
} // namespace std

namespace fmt
{
template <typename Tag>
struct formatter<::jps::UniqueID<Tag>> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& p_ctx)
    {
        return p_ctx.begin();
    }

    template <typename FormatContext>
    auto format(::jps::UniqueID<Tag> const& p_id, FormatContext& p_ctx) const
    {
        return fmt::format_to(p_ctx.out(), "{}", p_id.m_value);
    }
};
} // namespace fmt
