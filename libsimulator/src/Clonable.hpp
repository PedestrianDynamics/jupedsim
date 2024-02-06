// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <memory>

template <typename T>
class Clonable
{
public:
    virtual std::unique_ptr<T> Clone() const = 0;
};
