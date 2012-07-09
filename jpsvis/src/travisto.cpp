/**
 * @file travisto.cpp
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
 *  Created on: 11.05.2009
 *
 */


#include "travisto.h"
#include "SaxParser.h"
#include "SystemSettings.h"
#include "./forms/Settings.h"


#include "extern_var.h"
#include "TrajectoryPoint.h"
#include "Frame.h"
#include "Debug.h"
#include "ThreadVisualisation.h"
#include "ThreadDataTransfert.h"
#include "SimpleVisualisationWindow.h"

#include "geometry/FacilityGeometry.h"
#include "geometry/jul/Building.h"
#include "geometry/pg3/CBuilding.h"


#include <QtGui/QMessageBox>
#include <QtXml/QDomDocument>
#include <QTime>
#include <QApplication>
#include <QCloseEvent>
#include <QThread>
#include <QIODevice>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QInputDialog>
#include <QString>
#include <QColorDialog>
#include <QDebug>
#include <QtXml>

#include <iostream>
#include <limits>
#include <string>
#include <vector>

using namespace std;

TraVisTo::TraVisTo(QWidget *parent) :
																QMainWindow(parent) {
	ui.setupUi(this);
	this->setWindowTitle("TraVisTo");

	//disable the maximize Button
	setWindowFlags( Qt::Window | Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint );

	//create the 2 threads and connect them
	dataTransferThread = new ThreadDataTransfer(this);
	visualisationThread = new ThreadVisualisation(this);


	travistoOptions = new Settings(this);
	travistoOptions->setWindowTitle("Settings");
	travistoOptions->setWindowFlags(Qt::Window | Qt::WindowCloseButtonHint);


	if(!QObject::connect(dataTransferThread,
			SIGNAL(signal_controlSequence(const char*)), this,
			SLOT(slotControlSequence(const char *)))){
		Debug::Error("dataTransferThread Thread: control sequence could not be connected");
	}

	if(!QObject::connect(dataTransferThread,
			SIGNAL(signal_startVisualisationThread(QString,int,float )), this,
			SLOT(slotStartVisualisationThread(QString,int,float )))){
		Debug::Error(" signal_startVisualisationThread not connected");
	}

	if(!QObject::connect(dataTransferThread,
			SIGNAL(signal_stopVisualisationThread(bool )), this,
			SLOT(slotShutdownVisualisationThread(bool )))){
		Debug::Error(" signal_stopVisualisationThread not connected ");
	}

	if(!QObject::connect(dataTransferThread,
			SIGNAL(signal_errorMessage(QString)), this,
			SLOT(slotErrorOutput(QString)))){
		Debug::Error("signal_errorMessage  not connected ");
	}

	QObject::connect(visualisationThread,
			SIGNAL(signal_controlSequences(const char*)), this,
			SLOT(slotControlSequence(const char *)));

	QObject::connect(&extern_trajectories_firstSet,
			SIGNAL(signal_controlSequences(const char*)), this,
			SLOT(slotControlSequence(const char *)));

	QObject::connect(&extern_trajectories_secondSet,
			SIGNAL(signal_controlSequences(const char*)), this,
			SLOT(slotControlSequence(const char *)));

	QObject::connect(&extern_trajectories_thirdSet,
			SIGNAL(signal_controlSequences(const char*)), this,
			SLOT(slotControlSequence(const char *)));


	QObject::connect(dataTransferThread, SIGNAL(signal_CurrentAction(QString)),
			this, SLOT(slotCurrentAction(QString)));

	QObject::connect(this, SIGNAL(signal_controlSequence(QString)),
			travistoOptions, SLOT(slotControlSequence(QString)));

	isPlaying = false;
	isPaused = false;
	numberOfDatasetLoaded=0;
	frameSliderHold=false;


	labelCurrentAction = new QLabel();
	labelCurrentAction->setFrameStyle(QFrame::Panel | QFrame::Sunken);
	//	currentAction->setLineWidth(5);
	//	currentAction->setMargin(2);
	//	currentAction->setMidLineWidth(2);
	labelCurrentAction->setText("   Idle   ");
	statusBar()->addPermanentWidget(labelCurrentAction);

	labelFrameNumber = new QLabel();
	labelFrameNumber->setFrameStyle(QFrame::Panel | QFrame::Sunken);
	//	frameNumber->setLineWidth(5);
	//	frameNumber->setFixedWidth(80);
	//	frameNumber->setMargin(2);
	//	frameNumber->setMidLineWidth(2);
	labelFrameNumber->setText("fps:");
	statusBar()->addPermanentWidget(labelFrameNumber);

	labelRecording = new QLabel();
	labelRecording->setFrameStyle(QFrame::Panel | QFrame::Sunken);
	labelRecording->setText(" rec: off ");
	statusBar()->addPermanentWidget(labelRecording);

	labelMode = new QLabel();
	labelMode->setFrameStyle(QFrame::Panel | QFrame::Sunken);
	labelMode->setText(" Offline ");
	statusBar()->addPermanentWidget(labelMode);


	ui.BtNextFrame->setVisible(false);
	ui.BtPreviousFrame->setVisible(false);
	//TODO: Maybe this is not longer needed
	ui.framePerSecondSliderLabel->setVisible(false);
	ui.contrastSettingSlider->setVisible(false);

	QStringList arguments =QApplication::arguments();
	int group=1; // there are max 3 groups of pedestrians
	bool mayPlay=false;

	//	arguments.append("-online");
	//	arguments.append("-caption");
		arguments.append("-2D");
	// parse arguments list
	if(arguments.size()>1)
		for(int argCount=1;argCount<arguments.size();argCount++){

			QString argument=arguments[argCount];

			if(argument.compare("help")==0) {
				Debug::Error("Usage: ./TraVisTo [file1] [file2] [file3] [-2D] [-caption] [-online [port]]");
			}
			else if(argument.compare("-2D")==0) {
				ui.action2_D->setChecked(true);
				slotToogle2D();
			}else if(argument.compare("-caption")==0) {
				ui.actionShow_Captions->setChecked(true);
				slotShowPedestrianCaption();

			}else if(argument.compare("-online")==0) {
				slotSetOnlineMode(true);

				// checking for other possible options [ port...]
				if(argCount!=arguments.size()-1){
					bool ok=false;
					int port = arguments[++argCount].toInt(&ok);
					Debug::
					Debug::Messages(" listening port: %d",port);

					if (ok) {
						SystemSettings::setListningPort(port);
					}else{ // maybe that wasnt the port
						argCount--; // to ensure that the "option" will be checked
					}
				}
				mayPlay=true;

			}else if(argument.startsWith("-")) {
				const char* std=argument.toStdString().c_str();
				Debug::Error(" unknown options: %s",std);
				Debug::Error("Usage: ./TraVisTo [file1] [file2] [file3] [-2D] [-caption] [-online [port] ]");

			}else if(addPedestrianGroup(group,argument)){
				//slotHelpAbout();
				group++;
				mayPlay=true;
			}

		}

	if(mayPlay)slotStartPlaying();
}

TraVisTo::~TraVisTo() {

	extern_shutdown_visual_thread=true;
	extern_recording_enable=false;


	if (visualisationThread->isRunning()) {
		//std::Debug::Messages("Thread  visualisation is still running" << std::endl;
		waitForVisioThread();
	}

	if (dataTransferThread->isRunning()) {
		//std::Debug::Messages("Thread  visualisation is still running" << std::endl;
		//it is your own fault if you were still recording something
		waitForDataThread();
	}

	delete dataTransferThread;
	delete visualisationThread;
	delete travistoOptions;
	delete labelCurrentAction;
	delete labelFrameNumber;
	delete labelRecording;
	delete labelMode;

}

void TraVisTo::slotHelpAbout() {

	QMessageBox::about(
			this,
			"About TraVisTo",
			"Version 0.2 build with  QT 4.6 and VTK 5.4\n\n"
			"TraVisTo stands for Trajectories Visualisation Tool. It is a tool for visualizing pedestrians motion\n"
			"developped at the Forschungszentrum Juelich, Germany\n\n"
			"Copyright 2009-2011.\n"
			"Authors: Ulrich Kemloh\n\n"
			"For questions, contact +49-40-246161-4193 \nor mail at \n"
			"u.kemloh@fz-juelich.de\n");

}



/**
 *setup the network port to bind
 *
 */

void TraVisTo::slotNetworkSettings() {
	bool ok;

	int port = QInputDialog::getInteger(this, tr("input a port "), tr(
			"port(default to 8081):"), 8989, 5000, 65355, 1, &ok);

	if (ok) {
		SystemSettings::setListningPort(port);
	}
}


void TraVisTo::slotStartPlaying() {

	//first reset this variable. just for the case
	// the thread was shutdown from the reset option
	extern_shutdown_visual_thread = false;


	if (!isPlaying) {
		statusBar()->showMessage(QString::fromUtf8("waiting for data"));
		//only start the visualisation thread if in offline modus
		// otherwise both threads should be started
		if (extern_offline_mode) {
			if(anyDatasetLoaded()){ //at least one dataset was loaded, restarting the playback

				// starting the playback from the beginning
				extern_trajectories_firstSet.resetFrameCursor();
				extern_trajectories_secondSet.resetFrameCursor();
				extern_trajectories_thirdSet.resetFrameCursor();

			} else if (!slotAddDataSet()) { //else load a dataset
				return;//could not read any data
			}
			visualisationThread->start();

		} else /*if (extern_online_mode)*/ { //live visualisation
			dataTransferThread->start();
			//visualisationThread->start();
		}

		//enable recording
		ui.BtRecord->setEnabled(true);
		labelCurrentAction->setText("   playing   ");

		ui.action3_D->setEnabled(false);
		ui.action2_D->setEnabled(false);

		//disable legend
		ui.actionShow_Legend->setEnabled(false);

		//change Icon to Pause
		QIcon icon1;
		icon1.addPixmap(QPixmap(QString::fromUtf8(
				":/new/iconsS/icons/PauseHot.png")), QIcon::Normal, QIcon::Off);
		ui.BtStart->setToolTip("pause the visualisation");
		ui.BtStart->setIcon(icon1);

		//anyDataLoaded=true;
		isPlaying = true;
		extern_is_pause = false;

	} else if (isPaused) { //resume playing
		isPaused = false;
		extern_is_pause = false;
		//restore the play Icon
		QIcon icon1;
		icon1.addPixmap(QPixmap(QString::fromUtf8(
				":/new/iconsS/icons/PauseHot.png")), QIcon::Normal, QIcon::Off);
		ui.BtStart->setIcon(icon1);
		labelCurrentAction->setText("   playing   ");
		ui.BtStart->setToolTip("pause the visualisation");

	} else { //call pause
		isPaused = true;
		extern_is_pause = true;
		QIcon icon1;
		icon1.addPixmap(QPixmap(QString::fromUtf8(
				":/new/iconsS/icons/Play1Hot.png")), QIcon::Normal, QIcon::Off);
		ui.BtStart->setIcon(icon1);
		ui.BtStart->setToolTip("resume the visualisation");
		labelCurrentAction->setText("   paused   ");
	}

	//no matter what, the stop button should be enabled
	ui.BtStop->setEnabled(true);
}

void TraVisTo::slotStopPlaying() {

	if (extern_recording_enable) {
		int res = QMessageBox::question(this, "action",
				"do you wish to stop the recording?", QMessageBox::Discard
				| QMessageBox::Yes, QMessageBox::Yes);
		if (res == QMessageBox::Yes) {
			extern_recording_enable = false;
			//currentAction->setText("   Idle   ");
			labelRecording->setText("rec: OFF");

		} else {
			return;
		}
	}

	//shutdown the threads
	//fixme: shut the network thread as well
	// maybe this is a feature !
	extern_shutdown_visual_thread=true;
	waitForVisioThread();

	//reset all frames cursors
	resetAllFrameCursor();

	//disable/reset all graphical elements
	isPlaying = false;
	isPaused = false;
	resetGraphicalElements();
	labelCurrentAction->setText(" Idle ");
}

/**
 * open a file containing visualisation data, parse them
 * and initialize the visualization
 *
 */
bool TraVisTo::slotLoadFile() {

	return slotAddDataSet();

}


// This function is only used in online Mode
FacilityGeometry* TraVisTo::parseGeometry(QDomNode geoNode){

	if(geoNode.isNull()) return NULL;

	//check if there is a tag 'file' there in
	QString fileName = geoNode.toElement().elementsByTagName("file").item(0).toElement().attribute("location");
	FacilityGeometry* geometry = visualisationThread->getGeometry();

	if(!fileName.isEmpty())
	{
		if(fileName.endsWith(".jul",Qt::CaseInsensitive))
		{
			//should be a file name
			//return parseGeometryJUL(fileName);
			SaxParser::parseGeometryJUL(fileName,geometry);
		}
		else if (fileName.endsWith(".pg3",Qt::CaseInsensitive))
		{
			//should be a file name
			//return parseGeometryPG3(fileName);
			SaxParser::parseGeometryPG3(fileName,geometry);
		}
		else if (fileName.endsWith(".trav",Qt::CaseInsensitive))
		{
			//must not be a file name
			SaxParser::parseGeometryTRAV(fileName,geometry);
		}
	}
	// I assume it is a trav format node,
	//which is the only one which can directly be inserted into a file
	else
	{
		//must not be a file name
		SaxParser::parseGeometryTRAV(fileName,geometry,geoNode);
	}
	return geometry;
}

// TODO: still used?
bool TraVisTo::parsePedestrianShapes(QDomNode shapeNode, int groupID){

	if(shapeNode.isNull()) return false;
	QStringList heights;
	QStringList colors;

	QDomNodeList agents = shapeNode.toElement().elementsByTagName("agentInfo");

	for (unsigned int i = 0; i < agents.length(); i++) {

		bool ok=false;
		int id=agents.item(i).toElement().attribute("ID").toInt(&ok);
		if(!ok) continue; // invalid ID
		double height=agents.item(i).toElement().attribute("height").toDouble(&ok);
		if(!ok)height=std::numeric_limits<double>::quiet_NaN();

		int color=agents.item(i).toElement().attribute("color").toDouble(&ok);
		if(!ok)color=std::numeric_limits<int>::quiet_NaN();
		Debug::Messages("id= %d height= %lf color =%d",id,height,color);

		if(!isnan(height)){
			heights.append(QString::number(id));
			heights.append(QString::number(height));
		}
		if(!isnan(color)){
			colors.append(QString::number(id));
			colors.append(QString::number(color));
		}

	}
	switch (groupID){

	case 1:
		extern_trajectories_firstSet.setInitialHeights(heights);
		extern_trajectories_firstSet.setInitialColors(colors);
		break;

	case 2:
		extern_trajectories_secondSet.setInitialHeights(heights);
		extern_trajectories_secondSet.setInitialColors(colors);
		break;

	case 3:
		extern_trajectories_thirdSet.setInitialHeights(heights);
		extern_trajectories_thirdSet.setInitialColors(colors);
		break;
	}

	return true;

}


/// add a new dataset
bool TraVisTo::slotAddDataSet(){

	if (numberOfDatasetLoaded>=3){
		QMessageBox::information(this,"notice","You can load at most 3 datasets.\n In"
				" Order to load other data, please first clear previously loaded data.");
		return false;
	}
	// if at least one data set was loaded
	if (numberOfDatasetLoaded>=1){
		int res = QMessageBox::question(this, "action",
				"adding a new dataset will reset the visualisation process. Continue?", QMessageBox::Discard
				| QMessageBox::Yes, QMessageBox::Yes);

		if (res == QMessageBox::Discard) return false;

	}

	// just continue
	numberOfDatasetLoaded++;
	if(addPedestrianGroup(numberOfDatasetLoaded)==false){
		numberOfDatasetLoaded--;
		return false;
	}

	QString stre(numberOfDatasetLoaded);
	stre.setNum(numberOfDatasetLoaded);
	stre.append(" dataset loaded");
	statusBar()->showMessage(stre);

	return true;
}

///clear all datasets previously entered.
void TraVisTo::slotClearAllDataset(){

	clearDataSet(1);
	clearDataSet(2);
	clearDataSet(3);
	numberOfDatasetLoaded=0;
}


bool TraVisTo::addPedestrianGroup(int groupID,QString fileName){
	statusBar()->showMessage(tr("Select a file"));

	if(fileName.isEmpty())
		fileName = QFileDialog::getOpenFileName(this,
				"Select the file containing data to visualize",
				"F:\\workspace\\TraVisTo\\data",
				"Visualisation Files (*.dat *.trav *.xml *.pg3 *.jul);;All Files (*.*)");


	if (fileName.isNull()) {
		return false;
	}

	//the geometry actor
	//FacilityGeometry* geometry=NULL;
	FacilityGeometry* geometry = visualisationThread->getGeometry();

	//reading and parsing
	if(fileName.endsWith(".jul",Qt::CaseInsensitive))
	{
		//geometry=parseGeometryJUL(fileName);
		SaxParser::parseGeometryJUL(fileName,geometry);
	}
	else if (fileName.endsWith(".pg3",Qt::CaseInsensitive))
	{
		//geometry=parseGeometryPG3(fileName);
		SaxParser::parseGeometryPG3(fileName,geometry);

	}

	//	// if jul is detected, just load and show the geometry then exit
	//	if(fileName.endsWith(".jul",Qt::CaseInsensitive)){
	//		slotLoadParseShowGeometry(fileName);
	//		return false;
	//	}

	QFile file(fileName);
	if (!file.open(QIODevice::ReadOnly)) {
		Debug::Error("could not open the File: ",fileName.toStdString().c_str());
		return false;
	}

	SyncData* dataset=NULL;


	switch(groupID){
	case 1:
		Debug::Messages("handling first set");
		dataset=&extern_trajectories_firstSet;
		extern_first_dataset_loaded=true;
		extern_first_dataset_visible=true;
		ui.actionFirst_Group->setEnabled(true);
		ui.actionFirst_Group->setChecked(true);
		slotToggleFirstPedestrianGroup();
		break;

	case 2:
		Debug::Messages("handling second set");
		dataset=&extern_trajectories_secondSet;
		extern_second_dataset_loaded=true;
		extern_second_dataset_visible=true;
		ui.actionSecond_Group->setEnabled(true);
		ui.actionSecond_Group->setChecked(true);
		slotToggleSecondPedestrianGroup();
		break;

	case 3:
		Debug::Messages("handling third set");
		dataset=&extern_trajectories_thirdSet;
		extern_third_dataset_loaded=true;
		extern_third_dataset_visible=true;
		ui.actionThird_Group->setEnabled(true);
		ui.actionThird_Group->setChecked(true);
		slotToggleThirdPedestrianGroup();
		break;

	default:
		Debug::Error("invalid pedestrian group: %d " ,groupID);
		Debug::Error("should be 1, 2 or 3");
		return false;
		break;
	}

	double frameRate=15; //default frame rate
	statusBar()->showMessage(tr("parsing the file"));
	QXmlInputSource source(&file);
	QXmlSimpleReader reader;

	// no other geometry format was detected
	if(geometry==NULL)
		geometry=new FacilityGeometry();
	SaxParser handler(geometry,dataset,&frameRate);
	reader.setContentHandler(&handler);
	reader.parse(source);
	file.close();

	QString frameRateStr=QString::number(frameRate);
	SystemSettings::setFilenamePrefix(QFileInfo ( fileName ).baseName()+"_");

	// set the visualisation window title
	visualisationThread->setWindowTitle(fileName);
	visualisationThread->slotSetFrameRate(frameRate);
	visualisationThread->setGeometry(geometry);
	//visualisationThread->setWindowTitle(caption);
	labelFrameNumber->setText("fps: " + frameRateStr+"/"+frameRateStr);

	//shutdown the visio thread
	extern_shutdown_visual_thread=true;
	waitForVisioThread();

	statusBar()->showMessage(tr("file loaded and parsed"));

	return true;
}


void TraVisTo::slotRecord() {

	if (extern_recording_enable) {
		int res = QMessageBox::warning(this, "action",
				"TraVisTo is already recording a video\n"
				"do you wish to stop the recording?", QMessageBox::Yes
				| QMessageBox::No, QMessageBox::No);
		if (res == QMessageBox::Yes) {
			extern_recording_enable = false;
			ui.BtRecord->setToolTip("Start Recording");
			labelCurrentAction->setText("   Playing   ");
			return;
		}
	}
	extern_launch_recording = true;
	ui.BtRecord->setToolTip("Stop Recording");
	labelRecording->setText("recording");
	//labelCurrentAction->setText("   recording   ");

}


QString TraVisTo::getTagValueFromElement(QDomNode node,
		const char * tagName) {
	if (node.isNull())
		return "";
	return node.toElement().namedItem(tagName).firstChild().nodeValue();

}

void TraVisTo::slotFullScreen(bool status) {

	Debug::Messages("changing full screen status %d",status);
	extern_fullscreen_enable = true;

	//dont forget this.
	extern_force_system_update=true;

}

void TraVisTo::slotSetOfflineMode(bool status) {

	if (status) {
		ui.actionOnline->setChecked(false);
		extern_offline_mode = true;
		labelMode->setText(" Offline ");
	} else {
		ui.actionOnline->setChecked(true);
		extern_offline_mode = false;
		labelMode->setText(" Online ");

	}

}

void TraVisTo::slotSetOnlineMode(bool status) {
	if (status) {
		ui.actionOffline->setChecked(false);
		extern_offline_mode = false;
		labelMode->setText(" Online  ");
	} else {
		ui.actionOffline->setChecked(true);
		extern_offline_mode = true;
		labelMode->setText(" Offline ");

	}
}

void TraVisTo::slotReset() {

	//stop any recording
	if (extern_recording_enable) {
		int res = QMessageBox::question(this, "action",
				"do you wish to stop the recording?", QMessageBox::Discard
				| QMessageBox::Yes, QMessageBox::Yes);
		if (res == QMessageBox::Yes) {
			extern_recording_enable = false;
			labelCurrentAction->setText("   Playing   ");
		}else{
			return;
		}
	}

	if(anyDatasetLoaded())
	{
		int res = QMessageBox::question(this, "action",
				"This will also clear any dataset if loaded.\n"
				"Do you wish to continue?", QMessageBox::Discard
				| QMessageBox::Yes, QMessageBox::Yes);
		if (res == QMessageBox::No) {
			return;
		}
	}


	//shutdown the visual thread
	extern_shutdown_visual_thread = true;
	waitForVisioThread();
	waitForDataThread();

	//reset all buttons
	//anyDataLoaded = false;
	slotClearAllDataset();
	isPlaying = false;
	isPaused  = false;
	labelCurrentAction->setText("   Idle   ");
	resetGraphicalElements();

}

void TraVisTo::slotCurrentAction(QString msg) {
	msg = " " + msg + " ";
	//labelCurrentAction->setText(msg);
	statusBar()->showMessage(msg);
	//	labelMode->setText(msg);
}

void TraVisTo::slotFrameNumber(unsigned long actualFrameCount) {
	QString msg;
	msg.setNum(actualFrameCount);

	ui.frameSliderLabel->setText("Frame: " + msg);

	//compute the  mamixum framenumber
	int maxFrameCount=1;
	if(extern_first_dataset_loaded){
		maxFrameCount=extern_trajectories_firstSet.getFramesNumber();

	}
	if(extern_second_dataset_loaded){
		int frameCount=extern_trajectories_secondSet.getFramesNumber();
		if(frameCount>maxFrameCount) maxFrameCount=frameCount;

	}
	if(extern_third_dataset_loaded){
		int frameCount=extern_trajectories_thirdSet.getFramesNumber();
		if(frameCount>maxFrameCount) maxFrameCount=frameCount;
	}
	if(!frameSliderHold)if(maxFrameCount!=0)//TODO WTF, otherwise an arrymtic exeption arises
		ui.framesIndicatorSlider->setValue((ui.framesIndicatorSlider->maximum()*actualFrameCount)/maxFrameCount);
}

void TraVisTo::slotRunningTime(unsigned long timems) {
	//	QString msg;
	//	msg.setNum(timems);
	//	msg.append(" milsec");
	//labelRunningTime->setText(msg);
	ui.lcdRunningTime->display((int)timems);
}

void TraVisTo::slotRenderingTime(int fps)
{
	QString msg =labelFrameNumber->text().replace(QRegExp("[0-9]+/"),QString::number(fps)+"/");
	labelFrameNumber->setText(msg);
}


void TraVisTo::slotExit() {
	cleanUp();
	qApp->exit();
}

void TraVisTo::closeEvent(QCloseEvent* event) {
	hide();
	cleanUp();
	event->accept();
}

/**
 * performs the necessary cleaning such as:
 *  closing data
 *  stopping recording
 */
void TraVisTo::cleanUp() {
	//stop the recording process
	extern_recording_enable = false;
	extern_shutdown_visual_thread = true;

	if(SystemSettings::getRecordPNGsequence())
		slotRecordPNGsequence();

	waitForVisioThread();
	waitForDataThread();

}



void TraVisTo::slotControlSequence(const char * sex) {

	QString str(sex);

	//todo: check wich stack is empty and disable
	// the correspinding datasets.
	// are all stacks empty? then stop
	// visualisation adn recording if any
	if (str.compare("CONTROL_STACK_EMPTY") == 0) {

		statusBar()->showMessage(tr(" stack empty "));
		extern_recording_enable = false;
		ui.BtRecord->setToolTip("Start Recording");
		labelRecording->setText(" rec: off ");

		//Debug::Messages("stack empty" << endl;
		//		Debug::Messages("stoping the playback and recording if any" << endl;
		//		//extern_recording_enable = false;
		//		extern_shutdown_visual_thread=true;
		//		waitForVisioThread();
		//
		//		//isPlaying = true; //very important
		//		//isPaused = true;
		//		//extern_is_pause = true;
		//
		//
		//		extern_screen_contrast=1;
		//		extern_is_pause = false;
		//		isPlaying=false;
		//		isPaused=false;

		//pause the system
		//		QIcon icon1;
		//		icon1.addPixmap(QPixmap(QString::fromUtf8(
		//				":/new/iconsS/icons/Play1Hot.png")), QIcon::Normal, QIcon::Off);
		//		ui.BtStart->setIcon(icon1);
		//		labelCurrentAction->setText("paused");
		//		extern_is_pause = true;
		//		isPlaying = true; //very important, cuz the visio was running
		//		isPaused = true;
		//		extern_update_step=1;


	} else if (str.compare("CONTROL_RESET") == 0) {
		Debug::Messages("resetting all");
		isPlaying = false;
		isPaused = false;
		labelCurrentAction->setText("   Idle   ");
		extern_update_step=1;
		//extern_screen_contrast=1;

		//reset the frames cursor
		extern_trajectories_firstSet.resetFrameCursor();
		extern_trajectories_secondSet.resetFrameCursor();
		extern_trajectories_thirdSet.resetFrameCursor();

		resetGraphicalElements();

	} else if (str.compare("STACK_REACHS_BEGINNING")==0){
		//return to normal speed
		if(extern_update_step<0){
			Debug::Messages("stack reaches beginning, resuming the playback with vel 1");
			extern_update_step=1;
			ui.speedSettingSlider->setValue(1);
		}

	}
}


void TraVisTo::resetGraphicalElements(){

	//restore play button
	QIcon icon1;
	icon1.addPixmap(QPixmap(
			QString::fromUtf8(":/new/iconsS/icons/Play1Hot.png")),
			QIcon::Normal, QIcon::Off);
	ui.BtStart->setIcon(icon1);

	//disable record button
	ui.BtRecord->setEnabled(false);

	//disable fullscreen
	ui.BtFullscreen->setEnabled(false);

	//disable stop button

	ui.BtStop->setEnabled(false);
	//reset the slider
	ui.framesIndicatorSlider->setValue(ui.framesIndicatorSlider->minimum());

	//reset the speed cursor
	ui.speedSettingSlider->setValue(1);
	//reset the contrast

	// bla bla
	ui.actionShow_Legend->setEnabled(true);

	//restore the play Icon

	//
	ui.actionShow_Trajectories->setEnabled(true);

	ui.action3_D->setEnabled(true);
	ui.action2_D->setEnabled(true);

	labelRecording->setText("rec: OFF");
	statusBar()->showMessage(tr("select a File"));

	//resetting the start/stop recording action
	//check whether the a png recording sequence was playing, stop if the case
	if(SystemSettings::getRecordPNGsequence())
		slotRecordPNGsequence();

}


void TraVisTo::slotToggleFirstPedestrianGroup(){
	if(ui.actionFirst_Group->isChecked()){
		extern_first_dataset_visible=true;
	}else{
		extern_first_dataset_visible=false;
	}
	extern_force_system_update=true;

}

/// enable/disable the second pedestrian group
void TraVisTo::slotToggleSecondPedestrianGroup(){
	if(ui.actionSecond_Group->isChecked()){
		extern_second_dataset_visible=true;
	}else{
		extern_second_dataset_visible=false;
	}
	extern_force_system_update=true;
}

/// enable/disable the third pedestrian group
void TraVisTo::slotToggleThirdPedestrianGroup(){
	if(ui.actionThird_Group->isChecked()){
		extern_third_dataset_visible=true;
	}else{
		extern_third_dataset_visible=false;
	}
	extern_force_system_update=true;
}

bool TraVisTo::anyDatasetLoaded(){
	return
			(extern_first_dataset_loaded||
					extern_second_dataset_loaded||
					extern_third_dataset_loaded);
}

void TraVisTo::slotShowTrajectoryOnly(){
	if(ui.actionShow_Trajectories->isChecked()){
		//visualisationThread->setTrailVisibility(true);
		extern_tracking_enable=true;
		//ui.actionShow_Trajectories->setEnabled(false);
	}else{
		extern_tracking_enable=false;
		//visualisationThread->setTrailVisibility(false);
	}
}


/// TODO: implement me
void TraVisTo::slotShowPedestrianOnly(){
	Debug::Error("Not implemented");
}

void TraVisTo::slotShowGeometry(){

	if (ui.actionShow_Geometry->isChecked()){
		visualisationThread->setGeometryVisibility(true);
		ui.actionShow_Exits->setEnabled(true);
		ui.actionShow_Walls->setEnabled(true);
	}
	else{
		visualisationThread->setGeometryVisibility(false);
		ui.actionShow_Exits->setEnabled(false);
		ui.actionShow_Walls->setEnabled(false);
	}

}

/// shows/hide geometry
void TraVisTo::slotShowHideExits(){
	if (ui.actionShow_Exits->isChecked()){
		visualisationThread->showDoors(true);
	}
	else{
		visualisationThread->showDoors(false);
	}
}

/// shows/hide geometry
void TraVisTo::slotShowHideWalls(){
	if (ui.actionShow_Walls->isChecked()){
		visualisationThread->showWalls(true);
	}
	else{
		visualisationThread->showWalls(false);
	}
}


/// update the playing speed
void TraVisTo::slotUpdateSpeedSlider(int newValue){

	QString msg;
	msg.setNum(newValue);

	extern_update_step=newValue;
	//Debug::Error( " speed updated to: %d", extern_update_step);
	ui.speedSliderLabel->setText("Speed: " + msg + " ");

}

/// update the position slider
void TraVisTo::slotUpdateFrameSlider(int newValue){

	// first get the correct position
	int maxFrameCount=1;
	if(extern_first_dataset_loaded){
		int t=extern_trajectories_firstSet.getFramesNumber();
		if(maxFrameCount<t) maxFrameCount=t;
	}
	if(extern_second_dataset_loaded){
		int t=extern_trajectories_secondSet.getFramesNumber();
		if(maxFrameCount<t) maxFrameCount=t;

	}
	if(extern_third_dataset_loaded){
		int t=extern_trajectories_thirdSet.getFramesNumber();
		if(maxFrameCount<t) maxFrameCount=t;
	}

	int update = ((maxFrameCount*newValue)/ui.framesIndicatorSlider->maximum());

	// then set the correct position
	if(extern_first_dataset_loaded){
		extern_trajectories_firstSet.setFrameCursorTo(update);
		//Debug::Error( " first dataset frames update to [1] : " <<update<<endl;

	}
	if(extern_second_dataset_loaded){
		extern_trajectories_secondSet.setFrameCursorTo(update);
		//Debug::Error( " second dataset frames update to[2] : " <<update<<endl;

	}
	if(extern_third_dataset_loaded){
		extern_trajectories_thirdSet.setFrameCursorTo(update);
		//Debug::Error( " third dataset frames update to [3] : " <<update<<endl;
	}


}

/// update the contrast
/// TODO: now known as framePerScond slider
void TraVisTo::slotUpdateContrastSlider(int newValue){

	//	 extern_screen_contrast=ui.contrastSettingSlider->value();
	//extern_screen_contrast=newValue;
	QString msg;
	msg.setNum(newValue);
	ui.framePerSecondSliderLabel->setText("Frames Per Second: " +msg +" ");
	Debug::Error( " contrast updated to: %d",newValue);

}


///clear the corresponding dataset;
void TraVisTo::clearDataSet(int ID){

	switch(ID){
	case 1:
		//extern_trajectories_firstSet.clear();
		extern_trajectories_firstSet.clearFrames();
		extern_trajectories_firstSet.resetFrameCursor();
		extern_first_dataset_loaded=false;
		extern_first_dataset_visible=false;
		ui.actionFirst_Group->setEnabled(false);
		ui.actionFirst_Group->setChecked(false);
		slotToggleFirstPedestrianGroup();
		numberOfDatasetLoaded--;
		break;

	case 2:
		extern_trajectories_secondSet.clearFrames();
		//extern_trajectories_secondSet.clear();
		extern_trajectories_secondSet.resetFrameCursor();
		extern_second_dataset_visible=false;
		extern_second_dataset_loaded=false;
		ui.actionSecond_Group->setEnabled(false);
		ui.actionSecond_Group->setChecked(false);
		slotToggleSecondPedestrianGroup();
		numberOfDatasetLoaded--;
		break;

	case 3:
		extern_trajectories_thirdSet.clearFrames();
		//extern_trajectories_thirdSet.clear();
		extern_trajectories_thirdSet.resetFrameCursor();
		extern_third_dataset_loaded=false;
		extern_third_dataset_visible=false;
		ui.actionThird_Group->setEnabled(false);
		ui.actionThird_Group->setChecked(false);
		slotToggleThirdPedestrianGroup();
		numberOfDatasetLoaded--;
		break;

	default:
		break;
	}

	if(numberOfDatasetLoaded<0)numberOfDatasetLoaded=0;

	//finally clear the geometry
	slotClearGeometry();
}

void TraVisTo::resetAllFrameCursor(){
	extern_trajectories_thirdSet.resetFrameCursor();
	extern_trajectories_secondSet.resetFrameCursor();
	extern_trajectories_firstSet.resetFrameCursor();
}

/// wait for visualisation thread to shutdown
///@todo why two different threads shutdown procedure.
void TraVisTo::waitForVisioThread(){
	while(visualisationThread->isRunning()){
		visualisationThread->wait(200);
		Debug::Messages("waiting for visualisation engine to terminate ...");
#ifdef __linux__
		visualisationThread->quit();
#else
		visualisationThread->terminate();
#endif
	}
	Debug::Messages("Visualisation Engine shutdown successfully");

	//to avoid some sudden shutting down after resuming
	extern_shutdown_visual_thread=false;
}

/// wait for datatransfer thread to be ready
void TraVisTo::waitForDataThread(){

	while(dataTransferThread->isRunning()){
		dataTransferThread->shutdown();
		Debug::Messages("Waiting for network engine to terminate ...");
		dataTransferThread->wait(500);
	}
	Debug::Messages("Network Engine shutdown successfully.");
}


/// set visualisation mode to 2D
void TraVisTo::slotToogle2D(){
	if(ui.action2_D->isChecked()){
		extern_is_3D=false;
		ui.action3_D->setChecked(false);
		SystemSettings::set2D(true);

	}else{
		extern_is_3D=true;
		ui.action3_D->setChecked(true);
		SystemSettings::set2D(false);
	}
	visualisationThread->setGeometryVisibility2D(SystemSettings::get2D());
	extern_force_system_update=true;
}

/// set visualisation mode to 3D
void TraVisTo::slotToogle3D(){
	if(ui.action3_D->isChecked()){
		extern_is_3D=true;
		ui.action2_D->setChecked(false);
		SystemSettings::set2D(false);

	} else {
		extern_is_3D=false;
		ui.action2_D->setChecked(true);
		SystemSettings::set2D(true);
	}
	visualisationThread->setGeometryVisibility3D(!SystemSettings::get2D());
	extern_force_system_update=true;
}

void TraVisTo::slotFrameSliderPressed(){
	frameSliderHold=true;
}

void TraVisTo::slotFrameSliderReleased(){
	frameSliderHold=false;
}

void TraVisTo::slotToogleShowLegend(){
	if(ui.actionShow_Legend->isChecked()){
		SystemSettings::setShowLegend(true);
	}else{
		SystemSettings::setShowLegend(false);
	}
}
/***
 *
 * @param geodata the geometry data received from the data transfer thread
 */
void TraVisTo::slotStartVisualisationThread(QString data,int numberOfAgents,float frameRate){

	extern_trajectories_firstSet.setNumberOfAgents(numberOfAgents);

	labelFrameNumber->setText("fps: "+QString::number(frameRate)+"/"+QString::number(frameRate));

	extern_first_dataset_loaded=true;
	extern_first_dataset_visible=true;
	ui.actionFirst_Group->setEnabled(true);
	ui.actionFirst_Group->setChecked(true);
	slotToggleFirstPedestrianGroup();

	QDomDocument doc("");
	data = "<travisto>\n" +data+ "\n</travisto>\n";

	QString errorMsg="";
	doc.setContent(data,&errorMsg);

	if(!errorMsg.isEmpty()){
		Debug::Error("%s", (const char *)errorMsg.toStdString().c_str());
		return;
	}

	QDomNode geoNode =doc.elementsByTagName("geometry").item(0);
	FacilityGeometry *geo = parseGeometry(geoNode);

	visualisationThread->slotSetFrameRate(frameRate);
	visualisationThread->setGeometry(geo);
	visualisationThread->start();
}

/// this method is called by the data transfer thread
/// investigate if you should shutdown the thread
void TraVisTo::slotShutdownVisualisationThread( bool clearAndStop){
	//FIXME: it could have some side effects.
	//you should close this threads
	if(clearAndStop){
		extern_shutdown_visual_thread=true;
		waitForVisioThread();
	}
	clearDataSet(1);
}

void TraVisTo::slotFramesByFramesNavigation(){
	if(ui.actionFrames_Navigation->isChecked()){
		ui.BtNextFrame->setVisible(true);
		ui.BtPreviousFrame->setVisible(true);
	}
	else{
		ui.BtNextFrame->setVisible(false);
		ui.BtPreviousFrame->setVisible(false);
	}
}

void TraVisTo::slotNextFrame(){

	if(extern_first_dataset_loaded){
		int newValue=extern_trajectories_firstSet.getFrameCursor()+1;
		extern_trajectories_firstSet.setFrameCursorTo(newValue);
		//Debug::Error( " update to : " <<newValue<<endl;
	}
	if(extern_second_dataset_loaded){
		int newValue=extern_trajectories_secondSet.getFrameCursor()+1;
		extern_trajectories_secondSet.setFrameCursorTo(newValue);
		//Debug::Error( " update to : " <<newValue<<endl;
	}
	if(extern_third_dataset_loaded){
		int newValue=extern_trajectories_thirdSet.getFrameCursor()+1;
		extern_trajectories_thirdSet.setFrameCursorTo(newValue);
		//Debug::Error( " update to : " <<newValue<<endl;

	}
}

void TraVisTo::slotPreviousFrame(){
	if(extern_first_dataset_loaded){
		int newValue=extern_trajectories_firstSet.getFrameCursor()-1;
		extern_trajectories_firstSet.setFrameCursorTo(newValue);
		//Debug::Error( " update to : " <<newValue<<endl;
	}
	if(extern_second_dataset_loaded){
		int newValue=extern_trajectories_secondSet.getFrameCursor()-1;
		extern_trajectories_secondSet.setFrameCursorTo(newValue);
		//Debug::Error( " update to : " <<newValue<<endl;
	}
	if(extern_third_dataset_loaded){
		int newValue=extern_trajectories_thirdSet.getFrameCursor()-1;
		extern_trajectories_thirdSet.setFrameCursorTo(newValue);
		//Debug::Error( " update to : " <<newValue<<endl;

	}
}

void TraVisTo::slotShowPedestrianCaption(){

	SystemSettings::setShowCaptions(ui.actionShow_Captions->isChecked());
	extern_force_system_update=true;
}


void TraVisTo::slotToogleShowAxis(){

	visualisationThread->setAxisVisible(ui.actionShow_Axis->isChecked());
}


//todo: rename this to slotChangeSettting
void TraVisTo::slotChangePedestrianShape(){

	//	Qt::WindowFlags flags = Qt::Window  | Qt::WindowCloseButtonHint;
	//
	//	Settings* travistoOptions = new Settings(this);
	//	travistoOptions->setWindowFlags(flags);

	travistoOptions->show();

	//shape->deleteLater();

}

void TraVisTo::slotCaptionColorAuto(){
	emit signal_controlSequence("CAPTION_AUTO");
}

void TraVisTo::slotCaptionColorCustom(){
	emit signal_controlSequence("CAPTION_CUSTOM");
}

void TraVisTo::slotChangeBackgroundColor(){

	QColorDialog* colorDialog = new QColorDialog(this);
	colorDialog->setToolTip("Choose a new color for the background");
	QColor col=colorDialog->getColor(Qt::white,this,"Select new background color");

	//the user may have cancelled the process
	if(col.isValid()==false) return;

	double  bkcolor[3]={(double)col.red()/255.0 ,(double)col.green()/255.0 ,(double)col.blue()/255.0};
	visualisationThread->setBackgroundColor(bkcolor);

	delete colorDialog;

}
/// change the wall color
void TraVisTo::slotChangeWallsColor(){

	QColorDialog* colorDialog = new QColorDialog(this);
	colorDialog->setToolTip("Choose a new color for walls");
	QColor col=colorDialog->getColor(Qt::white,this,"Select new wall color");

	//the user may have cancelled the process
	if(col.isValid()==false) return;

	double  bkcolor[3]={(double)col.red()/255.0 ,(double)col.green()/255.0 ,(double)col.blue()/255.0};

	visualisationThread->setWallsColor(bkcolor);

	delete colorDialog;

}

/// change the exits color
void TraVisTo::slotChangeExitsColor(){
	QColorDialog* colorDialog = new QColorDialog(this);
	colorDialog->setToolTip("Choose a new color for walls");
	QColor col=colorDialog->getColor(Qt::white,this,"Select new wall color");

	//the user may have cancelled the process
	if(col.isValid()==false) return;

	double  bkcolor[3]={(double)col.red()/255.0 ,(double)col.green()/255.0 ,(double)col.blue()/255.0};

	visualisationThread->setExitsColor(bkcolor);

	delete colorDialog;
}

void TraVisTo::slotSetCameraPerspectiveToTop(){
	int p= 1; //TOP

	visualisationThread->setCameraPerspective(p);
	//disable the virtual agent view
	SystemSettings::setVirtualAgent(-1);
	//cerr <<"Setting camera view to top"<<endl;
}

void TraVisTo::slotSetCameraPerspectiveToFront(){
	int p= 2; //FRONT
	visualisationThread->setCameraPerspective(p);
	//disable the virtual agent view
	SystemSettings::setVirtualAgent(-1);
	//	cerr <<"Setting camera view to FRONT"<<endl;
}

void TraVisTo::slotSetCameraPerspectiveToSide(){
	int p= 3; //SIDE
	visualisationThread->setCameraPerspective(p);
	//disable the virtual agent view
	SystemSettings::setVirtualAgent(-1);
	//cerr <<"Setting camera view to Side"<<endl;
}

void TraVisTo::slotSetCameraPerspectiveToVirtualAgent(){

	bool ok=false;
	int agent = QInputDialog::getInteger(this, tr("choose the agent you want to see the scene through"), tr(
			"agent ID(default to 1):"),1,1,500, 1, &ok);

	if (ok) {
		int p= 4; //virtual agent
		visualisationThread->setCameraPerspective(p);

		//get the virtual agent ID
		SystemSettings::setVirtualAgent(agent);
	}
}

/// @todo does it work? mem check?
void TraVisTo::slotClearGeometry(){
	visualisationThread->setGeometry(NULL);
}

void TraVisTo::slotErrorOutput(QString err) {
	QMessageBox msgBox;
	msgBox.setText("Error");
	msgBox.setInformativeText(err);
	msgBox.setStandardButtons(QMessageBox::Ok);
	msgBox.setIcon(QMessageBox::Critical);
	msgBox.exec();
}

void TraVisTo::slotTakeScreenShot(){
	//extern_take_screenshot=true;
	extern_take_screenshot=!extern_take_screenshot;
}

/// load settings, parsed from the project file
void TraVisTo::loadSettings(){

	Debug::Error("Not implemented");

}

/// start/stop the recording process als png images sequences
void TraVisTo::slotRecordPNGsequence(){
	if(!isPlaying){
		slotErrorOutput("Start a video first");
	}

	// get the status from the system settings and toogle it
	bool status =SystemSettings::getRecordPNGsequence();

	if(status){
		ui.actionRecord_PNG_sequences->setText("Record PNG sequence");
	}else{
		ui.actionRecord_PNG_sequences->setText("Stop PNG Recording");
	}

	// toggle the status
	SystemSettings::setRecordPNGsequence(!status);

}

/// render a PNG image sequence to an AVI video
void TraVisTo::slotRenderPNG2AVI(){
	slotErrorOutput("Not Implemented yet, sorry !");

}

void TraVisTo::dragEnterEvent(QDragEnterEvent *event){
	if (event->mimeData()->hasFormat("text/uri-list"))
		event->acceptProposedAction();
}

void TraVisTo::dropEvent(QDropEvent *event) {

	if (!extern_offline_mode) {
		slotErrorOutput("online mode, ignoring DnD !");
		return;
	}

	QList<QUrl> urls = event->mimeData()->urls();
	if (urls.isEmpty())
		return;

	slotStopPlaying();

	bool mayPlay = false;

	for (int i = 0; i < urls.size(); i++) {
		QString fileName = urls[i].toLocalFile();
		if (fileName.isEmpty())
			continue;
		if (addPedestrianGroup(numberOfDatasetLoaded + i + 1, fileName)) {
			mayPlay = true;
		}
	}

	if (mayPlay) {
		slotStartPlaying();
	}
}

/// show/hide onscreen information
/// information include Time and pedestrians left in the facility
void TraVisTo::slotShowOnScreenInfos(){
	bool value=ui.actionShow_Onscreen_Infos->isChecked();
	visualisationThread->setOnscreenInformationVisibility(value);
	SystemSettings::setOnScreenInfos(value);
}

/// show/hide the geometry captions
void TraVisTo::slotShowHideGeometryCaptions(){

	bool value=ui.actionShow_Geometry_Captions->isChecked();
		visualisationThread->setGeometryLabelsVisibility(value);
		//SystemSettings::setOnScreenInfos(value);
}
