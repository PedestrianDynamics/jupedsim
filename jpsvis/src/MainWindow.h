/**
* @headerfile travisto.h
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
* \brief main program class
*
*
*
*  Created on: 11.05.2009
*
*/

#ifndef TRAVISTO_H
#define TRAVISTO_H


#include "ui_mainwindow.h"
#include <QMainWindow>

#include <vector>

class vtkWindowToImageFilter;
class SyncData;
class ThreadDataTransfer;
class ThreadVisualisation;
class QString;
class QDomNode;
class FacilityGeometry;
class Building;
class Message;
class Settings;


extern int extern_update_step;
extern int extern_screen_contrast;
extern bool extern_is_pause;
extern bool extern_launch_recording;
extern bool extern_recording_enable;
extern bool extern_offline_mode;
extern bool extern_shutdown_visual_thread;
extern bool extern_fullscreen_enable;

extern bool extern_pedestrians_firstSet_visible;
extern bool extern_pedestrians_secondSet_visible;
extern bool extern_pedestrians_thirdSet_visible;

extern SyncData extern_trajectories_firstSet;
extern SyncData extern_trajectories_secondSet;
extern SyncData extern_trajectories_thirdSet;

//states if the datasets are loaded.
extern bool extern_first_dataset_loaded;
extern bool extern_second_dataset_loaded;
extern bool extern_third_dataset_loaded;

//states whether the loaded datasets are visible
extern bool extern_first_dataset_visible;
extern bool extern_second_dataset_visible;
extern bool extern_third_dataset_visible;


class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    virtual ~MainWindow();


public Q_SLOTS:

	/// display the help modus
	void slotHelpAbout();

	///quit the program
	void slotExit();

	/// load a file
	bool slotLoadFile();
	//void slotLoadProject();

	/// output an Error
	void slotErrorOutput(QString err);

	/// output a warning
	//void slotWarningOutput(QString warning);

	/// start the visualisation thread
	/// the signal is emitted when the
	/// data transfer thread the header and at least one data set
	void slotStartVisualisationThread(QString data="",int numberOfAgents=1000,float frameRate=25);

	/// shutdown the visualisation thread
	void slotShutdownVisualisationThread(bool);

	/// add a new dataset to the store.
	/// note that at most three (3) datasets can be loaded for a visualisation round
	bool slotAddDataSet();

	/// clear all previously added/loaded datasets
	void slotClearAllDataset();

	/// set the camera view angle to  TOP/FRONT/SIDE
	// TODO: high priority
	void slotSetCameraPerspectiveToTop();
	void slotSetCameraPerspectiveToFront();
	void slotSetCameraPerspectiveToSide();
	void slotSetCameraPerspectiveToVirtualAgent();

	//controls visualisation
	void slotStartPlaying();
	void slotStopPlaying();
	void slotRecord();
	void slotFullScreen(bool status);
	void slotReset();
	void slotNetworkSettings();
	// void slotToggleVisualisationMode();
	void slotSetOfflineMode(bool status);
	void slotSetOnlineMode(bool status);
	/// take a screenshot of the rendering window
	void slotTakeScreenShot();

	/// update the status message
	void slotCurrentAction(QString msg);
	void slotFrameNumber(unsigned long timems);
	void slotRunningTime(unsigned long timems);
	void slotRenderingTime(int fps);
	void slotControlSequence(const char *);

	/// load a geometry file and display it
	//void slotLoadGeometry( );
	void slotClearGeometry();
	//void slotLoadParseShowGeometry(QString fileName);

	/// load a geometry sent by the data transfer thread
	//void slotLoadGeometryToThread(QString data);


	/// enable/disable the first pedestrian group
	void slotToggleFirstPedestrianGroup();
	/// enable/disable the second pedestrian group
	void slotToggleSecondPedestrianGroup();
	/// enable/disable the third pedestrian group
	void slotToggleThirdPedestrianGroup();

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

	/// show pedestrians only without trail
	void slotShowPedestrianOnly();

	/// update the playing speed
	void slotUpdateSpeedSlider(int newValue);

	/// update the position slider
	void slotUpdateFrameSlider(int newValue);
	void slotFrameSliderPressed();
	void slotFrameSliderReleased();
	//void slotFrameSliderChanged();

	/// handle the frame by frame navigation
	void slotFramesByFramesNavigation();
	void slotNextFrame();
	void slotPreviousFrame();

	/// enable/disable the pedestrian captions
	void slotShowPedestrianCaption();

	/// set the bg color
	//void slotPickBackgroundColor();

	/// update the contrast
	void slotUpdateContrastSlider(int newValue);

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

	/// show/hide onscreen information
	/// information include Time and pedestrians left in the facility
	void slotShowOnScreenInfos();

private:
	Q_SIGNALS:
	void signal_controlSequence(QString);

protected:
	virtual void closeEvent(QCloseEvent* event);
	void dragEnterEvent(QDragEnterEvent *event);
	void dropEvent(QDropEvent *event);

private:

	/// load settings, parsed from the project file
	/// @todo not implemented
	void  loadSettings();

	/// performs the necessary cleaning before shutting down the program.
	void cleanUp();

	/// reset the system to the very initial state
	//void reset();

	/// get the value of the tag <tagName> from a QDomNode
	QString getTagValueFromElement(QDomNode node, const char * tagName);

	/// parse the geometry  Node and return a pointer to geometry object
	/// used in online mode only
	FacilityGeometry* parseGeometry(QDomNode geoNode);
    FacilityGeometry* parseGeometry(QString geometryString);

	/**
	 * parse a shape node and get the initials heights and colors of pedestrians.
	 *
	 * @param shapeNode The node to be parsed
	 * @param groupID 1, 2 or 3
	 * @return true if success
	 */
	bool parsePedestrianShapes(QDomNode shapeNode, int groupID);

	/// reset all graphic element to their initial(default) state
	void resetGraphicalElements();

	/// add a second/third dataset to the visualization data
	///  groupID may be 2 or 3
	/// @return false if something went wrong.

	bool addPedestrianGroup_old(int groupID, QString fileName="");
	bool addPedestrianGroup(int groupID, QString fileName="");

	///return true if at least one dataset was loaded
	bool anyDatasetLoaded();

	/// clear the corresponding dataset if it exists
	void clearDataSet(int ID);

	/// reset all dataset, to the beginning
	void resetAllFrameCursor();

	/// wait for visualisation thread to terminate
	void waitForVisioThread();

	/// wait for data transfer thread to terminate
	void waitForDataThread();



private:
    Ui::mainwindow ui;

	bool isPlaying;
	bool isPaused;
	bool frameSliderHold;
	int numberOfDatasetLoaded;

	Settings* travistoOptions;
	ThreadDataTransfer* dataTransferThread;
	ThreadVisualisation* visualisationThread;
	QLabel *labelCurrentAction;
	QLabel *labelFrameNumber;
	QLabel *labelRecording;
	QLabel *labelMode;
};

#endif // TRAVISTO_H
