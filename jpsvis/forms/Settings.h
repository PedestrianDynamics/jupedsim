/**
* @file Settings.h
* @author   Ulrich Kemloh <kemlohulrich@gmail.com>
* @version 0.1
* Copyright (C) <2009-2010>
*
* @section LICENSE
* This file is part of OpenPedSim.
*
* OpenPedSim is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* OpenPedSim is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with OpenPedSim. If not, see <http://www.gnu.org/licenses/>.
*
* @section DESCRIPTION
*
* @brief
*
*
*  Created on: 05.05.2009
*
*/

#ifndef PEDESTRIANSHAPE_H
#define PEDESTRIANSHAPE_H

#include <QWidget>
#include "ui_settings.h"
#include "./src/SystemSettings.h"

extern bool extern_force_system_update;

class Settings : public QWidget {
    Q_OBJECT

public:
    enum Shape {DEFAULT=1,ELLIPSE,PINGUINS};
    Settings(QWidget *parent = 0);
    virtual ~Settings();

public Q_SLOTS:

    /// change the resolution of the ellipse representing the pedestrian
    void slotChangePedestrianShape();


    /// change the trails settings
    void slotUpdateTrailSettings();

    /// customize pedestrian color
    void slotPickPedestrianColor();

    void slotChangePedestrianColor(const QColor& color);

    /// set the width of the lines defining the vertices
    void slotChangeVerticesWidth();

    /// change the output directory
    void slotChangeOutputDir();

    /// change the size of the pedestrian caption
    /// The maximal size is limited to 200
    void slotChangeCaptionSize();


    ///change the caption orientation
    void slotChangeCaptionOrientation();

    ///change the caption auto rotation policy
    void slotChangeCaptionAutoRotation();

    /// change pedestrian color profile
    void slotChangePedestrianColorProfile();

    /// choose a new caption color
    void slotPickCaptionColor();

    ///change the caption color mode, auto or custom
    void slotChangeCaptionColorMode();

    /// execute a control sequence
    void slotControlSequence(QString msg);

protected:
    virtual void closeEvent(QCloseEvent* event);

private:
    void updateCaptionSetting();
    Ui::SettingsClass ui;
    static QColor tmpCaptionColor;
    void slotErrorOutput(QString err);
};

#endif // PEDESTRIANSHAPE_H
