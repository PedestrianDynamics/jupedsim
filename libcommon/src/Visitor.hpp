// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

template <class... Ts>
struct Overloaded : Ts... {
    using Ts::operator()...;
};

template <class... Ts>
Overloaded(Ts...) -> Overloaded<Ts...>;
