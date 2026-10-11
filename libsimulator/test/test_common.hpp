// SPDX-License-Identifier: LGPL-3.0-or-later

/// Include this file in every translation unit with google tests.
/// To reliably customize google test printing all translation units use the same 'PrintTo'
/// otherwise the linker picks just the first one and this may be the default version from
/// googletest that prints bytes.
#include <fmt/format.h>

#include <ostream>

/// Allows googletest to print any type that is fmt::formattable.
/// NOTE: This will not work if the to-be-printed type is not in the global namespace. In
/// this case you need to extend this file and add a using decl for this function in the
/// desired namespace.
/// E.g.:
/// ```
/// namespace some::other::namespace {
///     using ::PrintTo;
/// }
/// ```
void PrintTo(const fmt::formattable auto& value, std::ostream* os)
{
    *os << fmt::format("{}", value);
}
