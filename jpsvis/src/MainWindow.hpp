/**
 * @headerfile travisto.h
 * @author   Ulrich Kemloh <kemlohulrich@gmail.com>
 * @version 0.1
 * Copyright (C) <2009-2010>
 *
 * @section LICENSE
 * This file is part of JuPedSim.
 *
 * JuPedSim is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * JuPedSim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with JuPedSim. If not, see <http://www.gnu.org/licenses/>.
 *
 * @section DESCRIPTION
 *
 * \brief main program class
 *
 *
 *
 *  Created on: 11.05.2009
 *
 */
#pragma once

#include "ApplicationState.hpp"
#include "Settings.hpp"
#include "TrajectoryData.hpp"
#include "Visualisation.hpp"
#include "myqtreeview.hpp"
#include "ui_mainwindow.h"

#include <QLabel>
#include <QMainWindow>
#include <QSettings>
#include <QSplitter>
#include <QStandardItem>
#include <QTreeWidget>
#include <filesystem>
#include <optional>
#include <vector>

class QString;
class QDomNode;
class FacilityGeometry;
class GeometryFactory;
class Building;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget * parent = 0, std::optional<std::filesystem::path> path = {});
    virtual ~MainWindow();

public slots:
    /// NEW SLOTS

    /// Open a trajectory file
    /// @return file could be opened successfully
    void slotOpenFile();

    /// Toggles replay of loaded data.
    /// @param checked, this is true if the button is pressed otherwise false.
    void slotTogglePlayback(bool checked);

    /// Resets frame index of currently active replay to zero
    void slotRewindFramesToBegin();

    /// Update the number of total frames
    void slotUpdateNumFrames(int num_frames);

    /// Sets the number of data frames that will be advanced per iteration on the renderer. I.e. a
    /// replay speed of 6 means only every 6th data frame will be shown.
    /// @frames_per_iteration number of frames that the data advances per render call.
    void slotSetReplaySpeed(int frames_per_iteration);

    /// OLD SLOTS
    /// display the help modus
    void slotHelpAbout();

    /// quit the program
    void slotExit();

    /// output an Error
    void slotErrorOutput(QString err);

    /// set the camera view angle to  TOP/FRONT/SIDE
    void slotSetCameraPerspectiveToTop();
    void slotSetCameraPerspectiveToTopRotate();
    void slotSetCameraPerspectiveToSideRotate();

    // controls visualisation
    void slotToggleRecording(bool checked);
    /// take a screenshot of the rendering window
    void slotTakeScreenShot();

    void slotFrameNumber(int frame);

    /// show/hides trajectories (leaving a trail) only
    void slotShowTrajectoryOnly();

    /// shows/hide geometry
    void slotShowGeometry();
    /// shows/hide geometry
    void slotShowHideExits();
    /// shows/hide geometry
    void slotShowHideWalls();
    /// shows/hide navigation lines
    void slotShowHideFloor();
    /// shows/hide geometry captions
    void slotShowHideGeometryCaptions();
    /// show hide the obstacles
    void slotShowHideObstacles();

    /// show pedestrians only without trail
    void slotShowPedestrianOnly();

    /// update the position slider
    void slotUpdateFrameSlider(int newValue);

    /// handle the frame by frame navigation
    void slotNextFrame();
    void slotPreviousFrame();

    /// enable/disable the pedestrian captions
    void slotShowPedestrianCaption();

    /// Toggle rendering of pedestrian directions.
    /// @param is_enabled
    void slotTogglePedestrianDirections(bool is_enabled);

    /// set visualisation mode to 2D
    void slotToogle2D();

    /// set visualisation mode to 3D
    void slotToogle3D();

    void slotToogleShowAxis();

    /// start/stop the recording process als png images sequences
    void slotRecordPNGsequence();

    /// render a PNG image sequence to an AVI video
    void slotRenderPNG2AVI();

    /// adjust the scene background color
    void slotChangeBackgroundColor();

    /// change the wall color
    void slotChangeWallsColor();

    /// change the exits color
    void slotChangeExitsColor();

    /// change the navigation lines color
    void slotChangeNavLinesColor();

    /// change the floor color
    void slotChangeFloorColor();

    /// change the obstacle color
    void slotChangeObstacleColor();

    /// show/hide onscreen information
    /// information include Time and number pedestrians left in the facility
    void slotShowOnScreenInfos();

    /// show the detailed structure of the geometry
    void slotShowGeometryStructure();

    void slotOnGeometryItemChanged(QStandardItem * item);

    void slotMousePositionUpdated(double x, double y, double z);

protected:
    virtual void closeEvent(QCloseEvent * event);
    void dragEnterEvent(QDragEnterEvent * event);

private:
    void startReplay();
    void stopReplay();
    void setEnablePlayerControls(bool state);
    void disablePlayerControls();
    void startRendering();
    void stopRendering();
    std::optional<std::filesystem::path> selectFileToLoad();

    /// load settings in the case the remember settings is checked.
    void loadAllSettings();

    /// save all system settings
    void saveAllSettings();

    /// performs the necessary cleaning before shutting down the program.
    void cleanUp();

    /// reset all graphic element to their initial(default) state
    void resetGraphicalElements();

    /// @return false if something went wrong.
    bool tryParseFile(const std::filesystem::path & path = {});
    void tryLoadFile(const std::filesystem::path & path);
    bool tryParseGeometry(const std::filesystem::path & path);
    bool tryParseTrajectory(const std::filesystem::path & path);

    /// return true if at least one dataset was loaded
    bool anyDatasetLoaded();

    /// Unload all data
    void unloadData();

    /// reset all dataset, to the beginning
    void resetAllFrameCursor();
    void SetAppInfos();

private:
    Ui::mainwindow ui;
    ApplicationState _state{ApplicationState::NoData};
    Settings _settings;
    TrajectoryData _trajectories;
    std::unique_ptr<Visualisation> _visualisation;
    QLabel labelFrameNumber;
    QLabel labelRecording;
    QLabel labelCurrentFile;
    QSplitter _splitter;
    MyQTreeView _geoStructure;
};
