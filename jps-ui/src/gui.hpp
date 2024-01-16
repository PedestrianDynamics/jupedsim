// Copyright © 2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "wkt.hpp"

#include <imgui.h>

class Gui
{
private:
    std::unique_ptr<DrawableGEOS> geo{nullptr};
    bool should_exit = false;
    bool should_recenter = false;

public:
    Gui() = default;
    ~Gui() = default;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    void Draw();
    bool ShouldExit() const { return should_exit; }
    bool RecenterOnGeometry() const { return should_recenter; }
    const DrawableGEOS* Geometry() const { return geo.get(); }
};
