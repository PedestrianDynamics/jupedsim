#pragma once
#include "RenderMode.hpp"

#include <QColor>
#include <QString>

struct Settings {
    RenderMode mode{RenderMode::MODE_2D};
    QColor bgColor{Qt::white};
    QColor floorColor{0, 0, 255};
    QColor wallsColor{180, 180, 180};
    QColor exitsColor{175, 175, 255};
    QString filesPrefix{""};
    bool showAgentsCaptions{false};
    bool showAgentDirections{false};
    bool showAgents{true};
    bool showGeometryCaptions{true};
    bool showGeometry{true};
    bool showFloor{true};
    // TODO(kkratz): How dooes this differ from show geometry
    bool showWalls{true};
    bool showExits{true};
    bool showTrajectories{false};
    bool showInfos{true};
    bool recordPNGsequence{false};
};
