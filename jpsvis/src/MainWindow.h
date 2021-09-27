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

#include "ApplicationState.h"
#include "myqtreeview.h"
#include "ui_mainwindow.h"

#include <QLabel>
#include <QMainWindow>
#include <QSettings>
#include <QSplitter>
#include <QStandardItem>
#include <QTreeWidget>
#include <vector>

class vtkWindowToImageFilter;
class SyncData;
class Visualisation;
class QString;
class QDomNode;
class FacilityGeometry;
class GeometryFactory;
class Building;
class Message;
class Settings;


extern int extern_update_step;
extern int extern_screen_contrast;
extern bool extern_is_pause;
extern bool extern_launch_recording;
extern bool extern_recording_enable;
extern bool extern_shutdown_visual_thread;
extern bool extern_fullscreen_enable;

extern bool extern_pedestrians_firstSet_visible;

extern SyncData extern_trajectories_firstSet;

// states if the datasets are loaded.
extern bool extern_first_dataset_loaded;

// states whether the loaded datasets are visible
extern bool extern_first_dataset_visible;


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget * parent = 0);
    virtual ~MainWindow();


public Q_SLOTS:
    /// NEW SLOTS

    /// Open a trajectory file
    /// @return file could be opened successfully
    bool slotOpenFile();

    /// Toggles replay of loaded data.
    /// @param checked, this is true if the button is pressed otherwise false.
    void slotToggleReplay(bool checked);

    /// Resets frame index of currently active replay to zero
    void slotRewindFramesToBegin();


    /// OLD SLOTS
    /// display the help modus
    void slotHelpAbout();

    /// quit the program
    void slotExit();

    /// output an Error
    void slotErrorOutput(QString err);

    /// clear all previously added/loaded datasets
    void slotClearAllDataset();

    /// set the camera view angle to  TOP/FRONT/SIDE
    // TODO: high priority
    void slotSetCameraPerspectiveToTop();
    void slotSetCameraPerspectiveToTopRotate();
    void slotSetCameraPerspectiveToSideRotate();
    void slotSetCameraPerspectiveToVirtualAgent();

    // controls visualisation
    void slotRecord();
    void slotReset();
    /// take a screenshot of the rendering window
    void slotTakeScreenShot();

    /// update the status message
    void slotCurrentAction(QString msg);
    void slotFrameNumber(unsigned long timems, unsigned long minFrame);
    void slotRunningTime(unsigned long timems);
    void slotRenderingTime(int fps);

    /// load a geometry file and display it
    void slotClearGeometry();

    /// enable/disable the first pedestrian group
    void slotToggleFirstPedestrianGroup();

    /// show/hides trajectories (leaving a trail) only
    void slotShowTrajectoryOnly();

    /// shows/hide geometry
    void slotShowGeometry();
    /// shows/hide geometry
    void slotShowHideExits();
    /// shows/hide geometry
    void slotShowHideWalls();
    /// shows/hide navigation lines
    void slotShowHideNavLines();
    /// shows/hide navigation lines
    void slotShowHideFloor();
    /// shows/hide geometry captions
    void slotShowHideGeometryCaptions();
    /// show hide the obstacles
    void slotShowHideObstacles();
    /// show/hide the gradient field
    void slotShowHideGradientField();

    /// show pedestrians only without trail
    void slotShowPedestrianOnly();

    /// update the position slider
    void slotUpdateFrameSlider(int newValue);
    void slotFrameSliderPressed();
    void slotFrameSliderReleased();

    /// handle the frame by frame navigation
    void slotNextFrame();
    void slotPreviousFrame();

    /// enable/disable the pedestrian captions
    void slotShowPedestrianCaption();

    /// enable/disable the pedestrian Directions
    void slotShowDirections();

    /// set visualisation mode to 2D
    void slotToogle2D();

    /// set visualisation mode to 3D
    void slotToogle3D();

    /// show / hide the legend
    void slotToogleShowLegend();

    void slotToogleShowAxis();

    /// change, choose the pedestrian shape
    void slotChangePedestrianShape();

    /// start/stop the recording process als png images sequences
    void slotRecordPNGsequence();

    /// render a PNG image sequence to an AVI video
    void slotRenderPNG2AVI();

    /// adjust the scene background color
    void slotChangeBackgroundColor();

    /// change the pedestrian caption mode to automatic
    void slotCaptionColorAuto();

    /// change he pedestrian caption color to manual
    void slotCaptionColorCustom();

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

private:
Q_SIGNALS:
    void signal_controlSequence(QString);

protected:
    virtual void closeEvent(QCloseEvent * event);
    void dragEnterEvent(QDragEnterEvent * event);
    void dropEvent(QDropEvent * event);

private:
    void startReplay();
    void stopReplay();
    void enablePlayerControls();
    void disablePlayerControls();
    void startRendering();
    void stopRendering();

    /// load settings in the case the remember settings is checked.
    void loadAllSettings();

    /// save all system settings
    void saveAllSettings();

    /// performs the necessary cleaning before shutting down the program.
    void cleanUp();

    /// reset all graphic element to their initial(default) state
    void resetGraphicalElements();

    /// add a second/third dataset to the visualization data
    ///  groupID may be 2 or 3
    /// @return false if something went wrong.
    bool addPedestrianGroup(int groupID, QString fileName = "");

    /// return true if at least one dataset was loaded
    bool anyDatasetLoaded();

    /// clear the corresponding dataset if it exists
    void clearDataSet(int ID);

    /// reset all dataset, to the beginning
    void resetAllFrameCursor();

    /// wait for visualisation thread to terminate
    void waitForVisioThread();

private:
    Ui::mainwindow ui;
    ApplicationState _state{ApplicationState::NoData};

    bool isPlaying;
    bool isPaused;
    bool frameSliderHold;
    int numberOfDatasetLoaded;

    Settings * travistoOptions;
    Visualisation * _visualisationThread;
    QLabel * labelCurrentAction;
    QLabel * labelFrameNumber;
    QLabel * labelRecording;
    QSplitter _splitter;
    MyQTreeView _geoStructure;
};
