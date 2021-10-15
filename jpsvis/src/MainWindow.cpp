/**
 * @file MainWindow.cpp
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
 *  Created on: 11.05.2009
 *
 */

#include "MainWindow.h"

#include "ApplicationState.h"
#include "BuildInfo.h"
#include "Frame.h"
#include "Log.h"
#include "Parsing.h"
#include "Settings.h"
#include "TrajectoryPoint.h"
#include "Visualisation.h"
#include "geometry/FacilityGeometry.h"

#include <QApplication>
#include <QCloseEvent>
#include <QColorDialog>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileSystemModel>
#include <QIODevice>
#include <QInputDialog>
#include <QListView>
#include <QMessageBox>
#include <QMimeData>
#include <QSplitter>
#include <QStandardItemModel>
#include <QString>
#include <QTableView>
#include <QTemporaryFile>
#include <QThread>
#include <QTime>
#include <filesystem>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

//////////////////////////////////////////////////////////////////////////////
// Creation & Destruction
//////////////////////////////////////////////////////////////////////////////
MainWindow::MainWindow(QWidget * parent) : QMainWindow(parent)
{
    ui.setupUi(this);
    this->setWindowTitle("JPSvis");

    // used for saving the settings in a persistant way
    QCoreApplication::setOrganizationName("Forschungszentrum_Juelich_GmbH");
    QCoreApplication::setOrganizationDomain("jupedsim.org");
    QCoreApplication::setApplicationName("jupedsim");

    _visualisation = std::make_unique<Visualisation>(
        this, ui.render_widget->renderWindow(), &_settings, &_trajectories);

    QObject::connect(
        &_visualisation->getGeometry().GetModel(),
        SIGNAL(itemChanged(QStandardItem *)),
        this,
        SLOT(slotOnGeometryItemChanged(QStandardItem *)));

    // some hand made stuffs
    // the state of actionShowGeometry_Structure connect the state of the
    // structure window
    connect(&_geoStructure, &MyQTreeView::changeState, [=]() {
        ui.actionShowGeometry_Structure->setChecked(false);
    });
    // connections
    connect(
        ui.actionTop_Rotate,
        &QAction::triggered,
        this,
        &MainWindow::slotSetCameraPerspectiveToTopRotate);
    connect(
        ui.actionSide_Rotate,
        &QAction::triggered,
        this,
        &MainWindow::slotSetCameraPerspectiveToSideRotate);

    labelCurrentAction.setFrameStyle(QFrame::Panel | QFrame::Sunken);
    labelCurrentAction.setText("   Idle   ");
    statusBar()->addPermanentWidget(&labelCurrentAction);

    labelFrameNumber.setFrameStyle(QFrame::Panel | QFrame::Sunken);
    labelFrameNumber.setText("fps:");
    statusBar()->addPermanentWidget(&labelFrameNumber);

    labelRecording.setFrameStyle(QFrame::Panel | QFrame::Sunken);
    labelRecording.setText(" rec: off ");
    statusBar()->addPermanentWidget(&labelRecording);

    // restore the settings
    loadAllSettings();
}

MainWindow::~MainWindow()
{
    // save all settings for the next session
    if(ui.actionRemember_Settings->isChecked()) {
        saveAllSettings();
        Log::Info("saving all settings");
    } else {
        // first clear everyting
        QSettings settings;
        settings.clear();
        // then remember that we do not want any settings saved
        settings.setValue("options/rememberSettings", false);
        Log::Info("clearing all settings");
    }
}

//////////////////////////////////////////////////////////////////////////////
// Public slots
//////////////////////////////////////////////////////////////////////////////
void MainWindow::slotOpenFile()
{
    switch(_state) {
        case ApplicationState::Playing:
            [[fallthrough]];
        case ApplicationState::NoData:
            [[fallthrough]];
        case ApplicationState::Paused: {
            const auto path = selectFileToLoad();
            if(path) {
                stopRendering();
                ui.BtStart->toggled(false);
                unloadData();
                const bool could_load_data = tryParseFile(path.value());
                if(could_load_data) {
                    _state = ApplicationState::Paused;
                    enablePlayerControls();
                    startRendering();
                } else {
                    _state = ApplicationState::NoData;
                    disablePlayerControls();
                }
            }
        }
    }
}

void MainWindow::slotTogglePlayback(bool checked)
{
    if(checked) {
        startReplay();
    } else {
        stopReplay();
    }
}

void MainWindow::slotRewindFramesToBegin()
{
    switch(_state) {
        case ApplicationState::NoData:
            break;
        case ApplicationState::Paused:
            [[fallthrough]];
        case ApplicationState::Playing:
            _trajectories.resetFrameCursor();
    }
}

void MainWindow::slotUpdateNumFrames(int num_frames)
{
    ui.framesIndicatorSlider->setMaximum(num_frames - 1);
}

//////////////////////////////////////////////////////////////////////////////
// Private methods
//////////////////////////////////////////////////////////////////////////////
void MainWindow::startReplay()
{
    switch(_state) {
        case ApplicationState::Playing:
            [[fallthrough]];
        case ApplicationState::NoData:
            break;
        case ApplicationState::Paused:
            _state = ApplicationState::Playing;
            _visualisation->pauseRendering(false);
            labelCurrentAction.setText("   playing   ");
            break;
    }
}

void MainWindow::stopReplay()
{
    switch(_state) {
        case ApplicationState::Playing:
            _visualisation->pauseRendering(true);
            labelCurrentAction.setText("   paused   ");
            _state = ApplicationState::Paused;
            break;
        case ApplicationState::NoData:
            [[fallthrough]];
        case ApplicationState::Paused:
            break;
    }
}

void MainWindow::enablePlayerControls()
{
    ui.BtPreviousFrame->setEnabled(true);
    ui.BtStart->setEnabled(true);
    ui.BtNextFrame->setEnabled(true);
    ui.BtRecord->setEnabled(true);
    ui.rewind->setEnabled(true);
    ui.framesIndicatorSlider->setEnabled(true);
    ui.actionTogglePlayback->setEnabled(true);
    ui.actionTogglePlayback->setChecked(false);
    ui.actionRewind->setEnabled(true);
}

void MainWindow::disablePlayerControls()
{
    ui.BtPreviousFrame->setEnabled(false);
    ui.BtStart->setEnabled(false);
    ui.BtNextFrame->setEnabled(false);
    ui.BtRecord->setEnabled(false);
    ui.rewind->setEnabled(false);
    ui.framesIndicatorSlider->setEnabled(false);
    ui.actionTogglePlayback->setEnabled(false);
    ui.actionTogglePlayback->setChecked(false);
    ui.actionRewind->setEnabled(false);
}

void MainWindow::startRendering()
{
    _visualisation->start();
    labelFrameNumber.setText(
        QString("fps: %1").arg(QString::number(_visualisation->trajectoryRecordingFps())));
}

void MainWindow::stopRendering()
{
    _visualisation->stop();
    resetAllFrameCursor();
    unloadData();
    resetGraphicalElements();
    labelCurrentAction.setText(" Idle ");
};

std::optional<std::filesystem::path> MainWindow::selectFileToLoad()
{
    const auto fileName = QFileDialog::getOpenFileName(
        this,
        "Select the file containing the data to visualize",
        QDir::currentPath(),
        "JuPedSim Files (*.xml *.txt);;All Files (*.*)");

    // the action was cancelled
    if(fileName.isNull()) {
        return {};
    } else {
        return {std::filesystem::path{fileName.toStdString()}};
    }
}

//////////////////////////////////////////////////////////////////////////////
// Old Code
//////////////////////////////////////////////////////////////////////////////
void MainWindow::slotHelpAbout()
{
    Log::Info("About JPSvis");
    QString gittext =
        QMessageBox::tr("<p style=\"line-height:1.4\" "
                        "style=\"color:Gray;\"><small><i>Version %1</i></small></p>"
                        "<p style=\"line-height:0.4\" style=\"color:Gray;\"><i>Hash</i> "
                        "%2</p>"
                        "<p  style=\"line-height:0.4\" style=\"color:Gray;\"><i>Date</i> "
                        "%3</p>"
                        "<p  style=\"line-height:0.4\" style=\"color:Gray;\"><i>Branch</i> "
                        "%4</p><hr>")
            .arg(JPSVIS_VERSION.c_str())
            .arg(GIT_COMMIT_HASH.c_str())
            .arg(GIT_COMMIT_DATE.c_str())
            .arg(GIT_BRANCH.c_str());

    QString text =
        QMessageBox::tr("<p style=\"color:Gray;\"><small><i> &copy; 2009-2021  Ulrich Kemloh "
                        "<br><a href=\"http://jupedsim.org\">jupedsim.org</a></i></small></p>"

        );

    QMessageBox msg(QMessageBox::Information, "About JPSvis", gittext + text, QMessageBox::Ok);
    QString logo = ":/new/iconsS/icons/JPSvis.icns";
    msg.setIconPixmap(QPixmap(logo));

    // Change font
    QFont font("Tokyo");
    font.setPointSize(10);
    msg.setFont(font);
    msg.exec();
}

bool MainWindow::tryParseFile(const std::filesystem::path & path)
{
    Log::Info("Trying to parse %s", path.string().c_str());
    const auto file_type = Parsing::detectFileType(path);
    switch(file_type) {
        case Parsing::InputFileType::GEOMETRY_XML:
            return tryParseGeometry(path);
        case Parsing::InputFileType::TRAJECTORIES_TXT:
            return tryParseTrajectory(path);
        case Parsing::InputFileType::UNRECOGNIZED:
            return false;
    }
}

bool MainWindow::tryParseGeometry(const std::filesystem::path & path)
{
    return Parsing::readJpsGeometryXml(path, _visualisation->getGeometry());
}

bool MainWindow::tryParseTrajectory(const std::filesystem::path & path)
{
    const auto parent_path       = path.parent_path();
    auto fileName                = QString::fromStdString(path.string());
    const auto additional_inputs = Parsing::extractAdditionalInputFilePaths(path);

    const bool readTrainTimeTable =
        additional_inputs.train_time_table_path &&
        std::filesystem::is_regular_file(additional_inputs.train_time_table_path.value());
    if(readTrainTimeTable) {
        Log::Info(
            "Found train time table file: \"%s\"",
            additional_inputs.train_time_table_path.value().string().c_str());
    }

    const bool readTrainTypes =
        additional_inputs.train_type_path &&
        std::filesystem::is_regular_file(additional_inputs.train_type_path.value());
    if(readTrainTypes) {
        Log::Info(
            "Found train types file: \"%s\"",
            additional_inputs.train_type_path.value().string().c_str());
    }

    std::map<std::string, std::shared_ptr<TrainType>> trainTypes;
    if(readTrainTypes) {
        // TODO(kkratz): This just continues on error, fixup impl.
        Parsing::LoadTrainType(additional_inputs.train_type_path.value().string(), trainTypes);
    }

    std::map<int, std::shared_ptr<TrainTimeTable>> trainTimeTable;
    if(readTrainTimeTable) {
        // TODO(kkratz): This just continues on error, fixup impl.
        bool ret = Parsing::LoadTrainTimetable(
            additional_inputs.train_time_table_path.value().string(), trainTimeTable);
    }
    if(readTrainTimeTable && readTrainTypes) {
        _visualisation->setTrainData(std::move(trainTypes), std::move(trainTimeTable));
    }


    if(additional_inputs.geometry_path) {
        if(!tryParseGeometry(additional_inputs.geometry_path.value())) {
            return false;
        }
    }

    std::tuple<Point, Point> trackStartEnd;
    double elevation;
    for(auto tab : trainTimeTable) {
        int trackId   = tab.second->pid;
        trackStartEnd = Parsing::GetTrackStartEnd(
            QString::fromStdString(additional_inputs.geometry_path.value().string()), trackId);
        elevation = 0;

        Point trackStart = std::get<0>(trackStartEnd);
        Point trackEnd   = std::get<1>(trackStartEnd);

        tab.second->pstart    = trackStart;
        tab.second->pend      = trackEnd;
        tab.second->elevation = elevation;

        Log::Info("=======\n");
        Log::Info("tab: %d\n", tab.first);
        Log::Info("Track start: [%.2f, %.2f]\n", trackStart._x, trackStart._y);
        Log::Info("Track end: [%.2f, %.2f]\n", trackEnd._x, trackEnd._y);
        Log::Info("Room: %d\n", tab.second->rid);
        Log::Info("Subroom %d\n", tab.second->sid);
        Log::Info("Elevation %d\n", tab.second->elevation);
        Log::Info("=======\n");
    }
    for(auto tab : trainTypes)
        Log::Info("type: %s\n", tab.first.c_str());

    _trajectories.clearFrames();
    if(false == Parsing::ParseTxtFormat(fileName, &_trajectories)) {
        return false;
    }

    statusBar()->showMessage(tr("file loaded and parsed"));

    return true;
}

void MainWindow::slotToggleRecording(bool checked)
{
    if(checked) {
        ui.BtRecord->setToolTip("Stop Recording");
        labelRecording.setText(" rec: on ");
        // TODO(kkratz): Call visualization to start recording. This is not yet implemented
    } else {
        ui.BtRecord->setToolTip("Start Recording");
        labelRecording.setText(" rec: off ");
        // TODO(kkratz): Call visualization to start recording. This is not yet implemented
    }
}

void MainWindow::slotFrameNumber(int frame)
{
    ui.framesIndicatorSlider->setValue(frame);
    int elapsed_time = frame * _visualisation->trajectoryRecordingFps();
    ui.time->setText(QString("%1/%2 (%3ms)")
                         .arg(
                             QString::number(frame),
                             QString::number(ui.framesIndicatorSlider->maximum()),
                             QString::number(elapsed_time)));
}

void MainWindow::slotExit()
{
    Log::Info("Exit jpsvis");
    stopRendering();
    cleanUp();
    qApp->exit();
}

void MainWindow::closeEvent(QCloseEvent * event)
{
    hide();
    cleanUp();
    event->accept();
}

void MainWindow::cleanUp()
{
    if(_settings.recordPNGsequence) {
        slotRecordPNGsequence();
    }
}

void MainWindow::resetGraphicalElements()
{
    ui.BtRecord->setEnabled(false);
    ui.framesIndicatorSlider->setValue(ui.framesIndicatorSlider->minimum());
    ui.actionShow_Trajectories->setEnabled(true);
    ui.action3_D->setEnabled(true);
    ui.action2_D->setEnabled(true);

    labelRecording.setText("rec: off");
    statusBar()->showMessage(tr("select a File"));

    // resetting the start/stop recording action
    // check whether the a png recording sequence was playing, stop if the case
    if(_settings.recordPNGsequence) {
        slotRecordPNGsequence();
    }
}

void MainWindow::slotShowTrajectoryOnly()
{
    _settings.showTrajectories = ui.actionShow_Trajectories->isChecked();
}

void MainWindow::slotShowPedestrianOnly()
{
    if(ui.actionShow_Agents->isChecked()) {
        _settings.showAgents = true;
    } else {
        _settings.showAgents = false;
    }
}

void MainWindow::slotShowGeometry()
{
    if(ui.actionShow_Geometry->isChecked()) {
        _visualisation->setGeometryVisibility(true);
        ui.actionShow_Exits->setEnabled(true);
        ui.actionShow_Walls->setEnabled(true);
        ui.actionShow_Geometry_Captions->setEnabled(true);
        ui.actionShow_Navigation_Lines->setEnabled(true);
        ui.actionShow_Floor->setEnabled(true);
        _settings.showGeometry = true;
    } else {
        _visualisation->setGeometryVisibility(false);
        ui.actionShow_Exits->setEnabled(false);
        ui.actionShow_Walls->setEnabled(false);
        ui.actionShow_Geometry_Captions->setEnabled(false);
        ui.actionShow_Navigation_Lines->setEnabled(false);
        ui.actionShow_Floor->setEnabled(false);
        _settings.showGeometry = false;
    }
}

void MainWindow::slotShowHideExits()
{
    bool status = ui.actionShow_Exits->isChecked();
    _visualisation->showDoors(status);
    _settings.showExits = status;
}

void MainWindow::slotShowHideWalls()
{
    bool status = ui.actionShow_Walls->isChecked();
    _visualisation->showWalls(status);
    _settings.showWalls = status;
}

void MainWindow::slotShowHideFloor()
{
    bool status = ui.actionShow_Floor->isChecked();
    _visualisation->showFloor(status);
    _settings.showFloor = status;
}

void MainWindow::slotUpdateFrameSlider(int newValue)
{
    _trajectories.moveToFrame(newValue);
}

void MainWindow::unloadData()
{
    // extern_trajectories_firstSet.clear();
    _trajectories.clearFrames();
    _trajectories.resetFrameCursor();
    _visualisation->getGeometry().Clear(); // also clear the geometry info
    _visualisation->setTrainData({}, {});
    // close geometryStrucutre window
    if(_geoStructure.isVisible()) {
        _geoStructure.close();
    }
}

void MainWindow::resetAllFrameCursor()
{
    _trajectories.resetFrameCursor();
}

/// set visualisation mode to 2D
void MainWindow::slotToogle2D()
{
    RenderMode mode{};
    if(ui.action2_D->isChecked()) {
        ui.action3_D->setChecked(false);
        mode = RenderMode::MODE_2D;

    } else {
        ui.action3_D->setChecked(true);
        mode = RenderMode::MODE_3D;
    }
    _settings.mode = mode;
    bool status    = mode == RenderMode::MODE_2D && _settings.showGeometry;
    _visualisation->setGeometryVisibility2D(status);
}

/// set visualisation mode to 3D
void MainWindow::slotToogle3D()
{
    RenderMode mode{};
    if(ui.action3_D->isChecked()) {
        ui.action2_D->setChecked(false);
        mode = RenderMode::MODE_3D;

    } else {
        ui.action2_D->setChecked(true);
        mode = RenderMode::MODE_2D;
    }
    _settings.mode = mode;
    bool status    = mode == RenderMode::MODE_3D && _settings.showGeometry;
    _visualisation->setGeometryVisibility3D(status);
}

void MainWindow::slotNextFrame()
{
    ui.BtStart->setChecked(false);
    _trajectories.incrementFrame();
}

void MainWindow::slotPreviousFrame()
{
    ui.BtStart->setChecked(false);
    _trajectories.decrementFrame();
}

void MainWindow::slotShowPedestrianCaption()
{
    _settings.showAgentsCaptions = ui.actionShow_Captions->isChecked();
}

void MainWindow::slotTogglePedestrianDirections(bool is_enabled)
{
    _settings.showAgentDirections = is_enabled;
}

void MainWindow::slotToogleShowAxis()
{
    _visualisation->setAxisVisible(ui.actionShow_Axis->isChecked());
}

void MainWindow::slotChangeBackgroundColor()
{
    QColorDialog * colorDialog = new QColorDialog(this);
    colorDialog->setToolTip("Choose a new color for the background");
    QColor col = colorDialog->getColor(Qt::white, this, "Select new background color");

    // the user may have cancelled the process
    if(col.isValid() == false)
        return;

    _visualisation->setBackgroundColor(col);

    QSettings settings;
    settings.setValue("options/bgColor", col);

    delete colorDialog;
}

/// change the wall color
void MainWindow::slotChangeWallsColor()
{
    QColorDialog * colorDialog = new QColorDialog(this);
    colorDialog->setToolTip("Choose a new color for walls");
    QColor col = colorDialog->getColor(Qt::white, this, "Select new wall color");

    // the user may have cancelled the process
    if(col.isValid() == false)
        return;

    _visualisation->setWallsColor(col);

    QSettings settings;
    settings.setValue("options/wallsColor", col);

    delete colorDialog;
}

/// change the exits color
void MainWindow::slotChangeExitsColor()
{
    QColorDialog * colorDialog = new QColorDialog(this);
    colorDialog->setToolTip("Choose a new color for the exits");
    QColor col = colorDialog->getColor(Qt::white, this, "Select new exit color");

    // the user may have cancelled the process
    if(col.isValid() == false)
        return;

    _visualisation->setExitsColor(col);

    QSettings settings;
    settings.setValue("options/exitsColor", col);
    Log::Info("Change Exits Color to %s", col.name().toStdString().c_str());
    delete colorDialog;
}

/// change the navigation lines colors
void MainWindow::slotChangeNavLinesColor()
{
    QColorDialog * colorDialog = new QColorDialog(this);
    colorDialog->setToolTip("Choose a new color for walls");
    QColor col = colorDialog->getColor(Qt::white, this, "Select new navigation lines color");

    // the user may have cancelled the process
    if(col.isValid() == false)
        return;

    _visualisation->setNavLinesColor(col);

    QSettings settings;
    settings.setValue("options/navLinesColor", col);

    delete colorDialog;
}

void MainWindow::slotChangeFloorColor()
{
    QColorDialog * colorDialog = new QColorDialog(this);
    colorDialog->setToolTip("Choose a new color for the floor");
    QColor col = colorDialog->getColor(Qt::white, this, "Select new floor color");

    // the user may have cancelled the process
    if(col.isValid() == false)
        return;

    _visualisation->setFloorColor(col);

    QSettings settings;
    settings.setValue("options/floorColor", col);

    delete colorDialog;
}

void MainWindow::slotChangeObstacleColor()
{
    QColorDialog * colorDialog = new QColorDialog(this);
    colorDialog->setToolTip("Choose a new color for the obstacles");
    QColor col = colorDialog->getColor(Qt::white, this, "Select new obstalce color");

    // the user may have cancelled the process
    if(col.isValid() == false)
        return;

    _visualisation->setObstacleColor(col);

    QSettings settings;
    settings.setValue("options/obstacle", col);

    delete colorDialog;
}

void MainWindow::slotSetCameraPerspectiveToTop()
{
    int p = 1; // TOP
    _visualisation->setCameraPerspective(p);
}

void MainWindow::slotSetCameraPerspectiveToTopRotate()
{
    int p      = 2; // TOP rotate
    bool ok    = false;
    int degree = QInputDialog::getInt(
        this, "Top rotate", "Rotation degrees [range:-180-->180]:", 0, -180, 180, 10, &ok);
    if(ok) {
        _visualisation->setCameraPerspective(p, degree);
    }
}

void MainWindow::slotSetCameraPerspectiveToSideRotate()
{
    int p      = 3; // SIDE rotate
    bool ok    = false;
    int degree = QInputDialog::getInt(
        this, "Side rotate", "Rotation degrees [range:-80-->80]:", 0, -80, 80, 10, &ok);
    if(ok) {
        _visualisation->setCameraPerspective(p, degree);
    }
}

void MainWindow::slotErrorOutput(QString err)
{
    QMessageBox msgBox;
    msgBox.setText("Error");
    msgBox.setInformativeText(err);
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.exec();
}

void MainWindow::slotTakeScreenShot()
{
    _visualisation->takeScreenshot();
}

/// load settings, parsed from the project file
void MainWindow::loadAllSettings()
{
    Log::Info("restoring previous settings");
    QSettings settings;

    // view
    if(settings.contains("view/2d")) {
        bool checked = settings.value("view/2d").toBool();
        ui.action2_D->setChecked(checked);
        ui.action3_D->setChecked(!checked);
        _settings.mode = RenderMode::MODE_2D;
        Log::Info("2D: %s", checked ? "Yes" : "No");
    }
    if(settings.contains("view/showAgents")) {
        bool checked = settings.value("view/showAgents").toBool();
        ui.actionShow_Agents->setChecked(checked);
        _settings.showAgents = checked;
        Log::Info("show Agents: %s", checked ? "Yes" : "No");
    }
    if(settings.contains("view/showCaptions")) {
        bool checked = settings.value("view/showCaptions").toBool();
        ui.actionShow_Captions->setChecked(checked);
        _settings.showAgentsCaptions = checked;
        Log::Info("show Captions: %s", checked ? "Yes" : "No");
    }
    if(settings.contains("view/showDirections")) {
        bool checked = settings.value("view/showDirections").toBool();
        ui.actionShow_Directions->setChecked(checked);
        _settings.showAgentDirections = checked;
        Log::Info("show Directions: %s", checked ? "Yes" : "No");
    }
    if(settings.contains("view/showTrajectories")) {
        bool checked = settings.value("view/showTrajectories").toBool();
        ui.actionShow_Trajectories->setChecked(checked);
        _settings.showTrajectories = checked;
        Log::Info("show Trajectories: %s", checked ? "Yes" : "No");
    }
    if(settings.contains("view/showGeometry")) {
        bool checked = settings.value("view/showGeometry").toBool();
        ui.actionShow_Geometry->setChecked(checked);
        _settings.showGeometry = checked;
        Log::Info("show Geometry: %s", checked ? "Yes" : "No");
    }
    if(settings.contains("view/showFloor")) {
        bool checked = settings.value("view/showFloor").toBool();
        ui.actionShow_Floor->setChecked(checked);
        _settings.showFloor = checked;
        Log::Info("show Floor: %s", checked ? "Yes" : "No");
    }
    if(settings.contains("view/showExits")) {
        bool checked = settings.value("view/showExits").toBool();
        ui.actionShow_Exits->setChecked(checked);
        _settings.showExits = checked;
        Log::Info("show Exits: %s", checked ? "Yes" : "No");
    }
    if(settings.contains("view/showWalls")) {
        bool checked = settings.value("view/showWalls").toBool();
        ui.actionShow_Walls->setChecked(checked);
        _settings.showWalls = checked;
        Log::Info("show Walls: %s", checked ? "Yes" : "No");
    }
    if(settings.contains("view/showGeoCaptions")) {
        bool checked = settings.value("view/showGeoCaptions").toBool();
        ui.actionShow_Geometry_Captions->setChecked(checked);
        _settings.showGeometryCaptions = checked;
        Log::Info("show geometry Captions: %s", checked ? "Yes" : "No");
    }
    if(settings.contains("view/showOnScreensInfos")) {
        bool checked = settings.value("view/showOnScreensInfos").toBool();
        ui.actionShow_Onscreen_Infos->setChecked(checked);
        _settings.showInfos = checked;
        Log::Info("show OnScreensInfos: %s", checked ? "Yes" : "No");
    }
    // options
    if(settings.contains("options/rememberSettings")) {
        bool checked = settings.value("options/rememberSettings").toBool();
        ui.actionRemember_Settings->setChecked(checked);
        Log::Info("remember settings: %s", checked ? "Yes" : "No");
    }

    if(settings.contains("options/bgColor")) {
        QColor color      = settings.value("options/bgColor").value<QColor>();
        _settings.bgColor = color;
        Log::Info("background color: %s", color.name().toStdString().c_str());
    }

    if(settings.contains("options/exitsColor")) {
        QColor color         = settings.value("options/exitsColor").value<QColor>();
        _settings.exitsColor = color;
        Log::Info("Exit color: %s", color.name().toStdString().c_str());
    }

    if(settings.contains("options/floorColor")) {
        QColor color         = settings.value("options/floorColor").value<QColor>();
        _settings.floorColor = color;
        Log::Info("Floor color: %s", color.name().toStdString().c_str());
    }

    if(settings.contains("options/wallsColor")) {
        QColor color         = settings.value("options/wallsColor").value<QColor>();
        _settings.wallsColor = color;
        Log::Info("Walls color: %s", color.name().toStdString().c_str());
    }
}

void MainWindow::saveAllSettings()
{
    // visualiation
    QSettings settings;

    // view
    settings.setValue("view/2d", _settings.mode == RenderMode::MODE_2D);
    settings.setValue("view/showAgents", _settings.showAgents);
    settings.setValue("view/showCaptions", _settings.showAgentsCaptions);
    settings.setValue("view/showDirections", _settings.showAgentDirections);
    settings.setValue("view/showTrajectories", _settings.showTrajectories);
    settings.setValue("view/showGeometry", _settings.showGeometry);
    settings.setValue("view/showFloor", _settings.showFloor);
    settings.setValue("view/showWalls", _settings.showWalls);
    settings.setValue("view/showExits", _settings.showExits);
    settings.setValue("view/showGeoCaptions", _settings.showGeometryCaptions);
    settings.setValue("view/showOnScreensInfos", _settings.showInfos);

    // options: the color settings are saved in the methods where they are used.
    settings.setValue("options/rememberSettings", ui.actionRemember_Settings->isChecked());
}

/// start/stop the recording process als png images sequences
void MainWindow::slotRecordPNGsequence()
{
    // TODO(kkz) disabled
    if(true) {
        slotErrorOutput("Start a video first");
    }

    // get the status from the system settings and toogle it
    bool status = _settings.recordPNGsequence;

    if(status) {
        ui.actionRecord_PNG_sequences->setText("Record PNG sequence");
    } else {
        ui.actionRecord_PNG_sequences->setText("Stop PNG Recording");
    }

    // toggle the status
    _settings.recordPNGsequence = !status;
}

/// render a PNG image sequence to an AVI video
void MainWindow::slotRenderPNG2AVI()
{
    slotErrorOutput("Not Implemented yet, sorry !");
}

void MainWindow::dragEnterEvent(QDragEnterEvent * event)
{
    if(event->mimeData()->hasFormat("text/uri-list"))
        event->acceptProposedAction();
}

/// show/hide onscreen information
/// information include Time and pedestrians left in the facility
void MainWindow::slotShowOnScreenInfos()
{
    bool value = ui.actionShow_Onscreen_Infos->isChecked();
    _visualisation->setOnscreenInformationVisibility(value);
    _settings.showInfos = value;
    Log::Info("Show On Screen Infos: %s", value ? "On" : "Off");
}

/// show/hide the geometry captions
void MainWindow::slotShowHideGeometryCaptions()
{
    bool value = ui.actionShow_Geometry_Captions->isChecked();
    _visualisation->setGeometryLabelsVisibility(value);
    _settings.showGeometryCaptions = value;
}

void MainWindow::slotShowHideObstacles()
{
    // TODO(kkratz): clarify this is required to keep
}

void MainWindow::slotShowGeometryStructure()
{
    _geoStructure.setHidden(!ui.actionShowGeometry_Structure->isChecked());
    if(_visualisation->getGeometry().RefreshView()) {
        _geoStructure.setWindowTitle("Geometry structure");
        _geoStructure.setModel(&_visualisation->getGeometry().GetModel());
    }
    Log::Info(
        "Show Geometry Structure: %s", ui.actionShowGeometry_Structure->isChecked() ? "On" : "Off");
}

void MainWindow::slotOnGeometryItemChanged(QStandardItem * item)
{
    QStringList l = item->data().toString().split(":");
    if(l.length() > 1) {
        int room   = l[0].toInt();
        int subr   = l[1].toInt();
        bool state = item->checkState();
        _visualisation->getGeometry().UpdateVisibility(room, subr, state);
    } else {
        for(int i = 0; i < item->rowCount(); i++) {
            QStandardItem * child = item->child(i);
            child->setCheckState(item->checkState());
            slotOnGeometryItemChanged(child);
        }
    }
}

void MainWindow::slotMousePositionUpdated(double x, double y, double z)
{
    statusBar()->showMessage(
        QString("x:%1 y:%2").arg(QString::number(x, 'f', 2), QString::number(y, 'f', 2)));
}
