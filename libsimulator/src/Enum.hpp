// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <string>
#include <type_traits>

/// Provide a template specialization for this template method of the Enum you want to create.
/// For example you have the following enum:
/// enum class Foo {
///     A,
///     B
/// };
/// template<> Foo from_string(const std::string& str) {
///     if(str == "a"){
///         return Foo::A;
///     }
///     if(str == "b"){
///         return Foo::B;
///     }
/// }
/// This allows to use a uniform sythax when creating enums from string, e.g.
/// Foo my_enum = from_string<Foo>("a");
/// This is intentionally constrained to enums.
template <typename Enum, typename = std::enable_if_t<std::is_enum_v<Enum>>>
Enum from_string(const std::string&);

/// Replacement for C++23 to underlying, see
/// https://en.cppreference.com/w/cpp/utility/to_underlying
template <typename Enum, typename = std::enable_if_t<std::is_enum_v<Enum>>>
auto to_underlying(Enum e) -> typename std::underlying_type_t<Enum>
{
    return static_cast<std::underlying_type_t<Enum>>(e);
};
