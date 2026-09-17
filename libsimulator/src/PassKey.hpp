// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

// PassKey idiom.
// See https://playfulprogramming.com/posts/a-forgotten-idiom-revisited-pass-key/
// or https://accu.org/journals/overload/31/176/mertz/
template <typename T>
class PassKey
{
    friend T;
    explicit PassKey() = default;
};
