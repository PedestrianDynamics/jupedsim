// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef UNREACHABLE
#if defined(__GNUC__) // GCC, Clang, ICC
#define UNREACHABLE() __builtin_unreachable()
#elif defined(_MSC_VER) // MSVC
#define UNREACHABLE() __assume(false)
#endif
#endif
