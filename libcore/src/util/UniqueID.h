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
    /// Static count for the next available unique identifier.
    // As there is a bug in clang-tidy (https://bugs.llvm.org/show_bug.cgi?id=48040) the nolint is
    // needed for now NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
    inline static std::atomic<Integer> uid_counter{0};
    /// The actual unique identifier of the object
    Integer m_value{++uid_counter};

public:
    UniqueID() = default;

    /// UniqueIds are copyable.
    UniqueID(UniqueID const & p_other) = default;

    /// UniqueIds are copyable.
    auto operator=(UniqueID const & p_other) -> UniqueID & = default;

    /// UniqueIds are movable.
    UniqueID(UniqueID && p_other) noexcept = default;

    /// UniqueIds are movable.
    auto operator=(UniqueID && p_other) noexcept -> UniqueID & = default;

    ~UniqueID() noexcept = default;

    auto getID() const noexcept -> Integer { return m_value; }

    auto operator==(const UniqueID & p_other) const noexcept -> bool
    {
        return m_value == p_other.m_value;
    };

    auto operator!=(const UniqueID & p_other) const noexcept -> bool
    {
        return m_value != p_other.m_value;
    };

    friend struct fmt::formatter<UniqueID<Tag>>;
};
} // namespace jps


namespace fmt
{
template <typename Tag>
struct formatter<::jps::UniqueID<Tag>> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext & p_ctx)
    {
        return p_ctx.begin();
    }

    template <typename FormatContext>
    auto format(::jps::UniqueID<Tag> const & p_id, FormatContext & p_ctx)
    {
        return format_to(p_ctx.out(), "{}", p_id.m_value);
    }
};
} // namespace fmt
