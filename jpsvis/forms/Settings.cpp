/**
* @file Settings.cpp
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

#include <iostream>
#include <QFileDialog>
#include <QMessageBox>
#include <QColorDialog>

#include "Settings.h"
#include "./src/geometry/LinePlotter.h"

using namespace std;

QColor Settings::tmpCaptionColor=QColor();

Settings::Settings(QWidget *parent)
: QWidget(parent)
{
	ui.setupUi(this);

	// initialize the output directory
	QString dir;
	SystemSettings::getOutputDirectory(dir);
	ui.LEditOutputDir->setText(dir);

	//restore the caption color parameter
	char bgcolor[50];
	sprintf(bgcolor,"background-color: rgb(%d, %d, %d);",tmpCaptionColor.red(),tmpCaptionColor.green(),tmpCaptionColor.blue());
	ui.frameCaptionColorPreview->setStyleSheet(bgcolor);

}

Settings::~Settings()
{

}

void Settings::slotChangePedestrianShape()
{
	int resolution=ui.CbEllipseResolution->currentText().toInt();
	SystemSettings::setEllipseResolution(resolution);
	extern_force_system_update=true;
}



void Settings::slotUpdateTrailSettings(){


	bool ok=false;
	int count=ui.CbTrailPointsCount->currentText().toInt(&ok);
	if(ok==false){
		//FIXME:
//		ui.CbTrailPointsCount->removeItem(ui.CbEllipseRadiusA->currentIndex());
		return;
	}
	int type=ui.CbTrailType->currentIndex();
	int form=ui.CbTrailGeometry->currentIndex();

	if(form==1)
		ui.CbTrailPolygoneWidth->setEnabled(true);
	else
		ui.CbTrailPolygoneWidth->setEnabled(false);
	SystemSettings::setTrailsInfo(count,type,form);
}

/// @todo use the QColorDialog instead
/// @todo make the whole thing live by connecting signal

void Settings::slotPickPedestrianColor(){

	QColorDialog* colorDialog = new QColorDialog(this);
	QColor col=colorDialog->getColor("choose the new pedestrian color");

	// the user may have cancelled the process
	if(col.isValid()==false) return;
	slotChangePedestrianColor(col);

	delete colorDialog;

}
void Settings::slotChangePedestrianColor(const QColor & color){
	int r=0.0,g=0.0,b=0.0;
	color.getRgb(&r,&g,&b);
	int  bkcolor[3]={r ,g,b};

	SystemSettings::setPedestrianColor(ui.CbPedestrianGroup->currentIndex(),bkcolor);
	extern_force_system_update=true;
}

void Settings::slotChangeVerticesWidth(){
	bool ok=false;

	double width=ui.CbTrailPolygoneWidth->currentText().toDouble(&ok);
	if(ok==false){
		//fixme:
		//ui.CbEllipseRadiusA->removeItem(ui.CbTrailPolygoneWidth->currentIndex());
		return;
	}
	LinePlotter::setLineWidth((int)width);
}




/// choose a new directory
void Settings::slotChangeOutputDir(){

	QString dir = QFileDialog::getExistingDirectory(this, tr("Select a Directory for saving your files (screenshots, video,...)"),
			"",
			QFileDialog::ShowDirsOnly
			| QFileDialog::DontResolveSymlinks);

	// in the case the selection was aborted
	if(!QFileInfo(dir).isWritable()) {
		slotErrorOutput("I cant write to that directory");
		dir="";
	}
	if(dir.isEmpty()){
		SystemSettings::getOutputDirectory(dir);
		ui.LEditOutputDir->setText(dir);
	}else{
		ui.LEditOutputDir->setText(dir);
		SystemSettings::setOutputDirectory(dir);
	}
}

void Settings::slotErrorOutput(QString err) {
	QMessageBox msgBox;
	msgBox.setText("Error");
	msgBox.setInformativeText(err);
	msgBox.setStandardButtons(QMessageBox::Ok);
	msgBox.setIcon(QMessageBox::Critical);
	msgBox.exec();
}

void Settings::slotChangePedestrianColorProfile(){

	if(!ui.chBpedestrianDefaultColor->isChecked()){
		ui.CbPedestrianGroup->setEnabled(true);
		ui.pushButtonChangePedestrianColor->setEnabled(true);
		SystemSettings::setPedestrianColorProfileFromFile(false);
	}else{
		ui.CbPedestrianGroup->setEnabled(false);
		ui.pushButtonChangePedestrianColor->setEnabled(false);
		SystemSettings::setPedestrianColorProfileFromFile(true);
	}
}

void Settings::slotChangeCaptionSize(){
	bool ok=false;

	int size =ui.CbCaptionSize->currentText().toInt(&ok);
	if( (ok==false)|| (size >=200)){
		ui.CbCaptionSize->removeItem(ui.CbCaptionSize->currentIndex());
		return;
	}

	int orientation= ui.ComboCaptionOrientation->currentIndex();
	bool automaticRotation=ui.checkBoxCaptionAutoRotate->isChecked();

	SystemSettings::setCaptionsParameters(size, tmpCaptionColor,orientation, automaticRotation);

	extern_force_system_update=true;
}


void Settings::slotPickCaptionColor(){

	QColorDialog* colorDialog = new QColorDialog(this);

	tmpCaptionColor=colorDialog->getColor();

	// the user may have cancelled the process
	if(tmpCaptionColor.isValid()==false) return;

	int size =ui.CbCaptionSize->currentText().toInt();
	int orientation= ui.ComboCaptionOrientation->currentText().toInt();
	bool automaticRotation=ui.checkBoxCaptionAutoRotate->isChecked();

	SystemSettings::setCaptionsParameters(size, tmpCaptionColor,orientation, automaticRotation);

	//update the color marker defined by a frame
	char bgcolor[50];
	sprintf(bgcolor,"background-color: rgb(%d, %d, %d);",tmpCaptionColor.red(),tmpCaptionColor.green(),tmpCaptionColor.blue());
	ui.frameCaptionColorPreview->setStyleSheet(bgcolor);

	extern_force_system_update=true;

	delete colorDialog;
}


void Settings::slotChangeCaptionOrientation(){
	int size =ui.CbCaptionSize->currentText().toInt();
	int orientation= ui.ComboCaptionOrientation->currentIndex();
	bool automaticRotation=ui.checkBoxCaptionAutoRotate->isChecked();

	SystemSettings::setCaptionsParameters(size, tmpCaptionColor,orientation, automaticRotation);
		extern_force_system_update=true;
}

void Settings::slotChangeCaptionAutoRotation(){
	int size =ui.CbCaptionSize->currentText().toInt();
	int orientation= ui.ComboCaptionOrientation->currentText().toInt();
	bool automaticRotation=ui.checkBoxCaptionAutoRotate->isChecked();

	SystemSettings::setCaptionsParameters(size, tmpCaptionColor,orientation, automaticRotation);
		extern_force_system_update=true;
}

void Settings::slotChangeCaptionColorMode(){

	if(ui.comboCaptionColorMode->currentText().compare("Auto")==0){
		int size =ui.CbCaptionSize->currentText().toInt();
		int orientation= ui.ComboCaptionOrientation->currentText().toInt();
		bool automaticRotation=ui.checkBoxCaptionAutoRotate->isChecked();

		SystemSettings::setCaptionsParameters(size, QColor() ,orientation, automaticRotation);
		extern_force_system_update=true;

	}else if(ui.comboCaptionColorMode->currentText().compare("Custom")==0){
		slotPickCaptionColor();
	}
}

//ugly, I had no choice :)
void Settings::closeEvent(QCloseEvent* event){
	hide();
	///@FIXME
//	event->ignore();
}

void Settings::slotControlSequence(QString msg){

	if(msg.compare("CAPTION_AUTO")==0){
		ui.comboCaptionColorMode->setCurrentIndex(0); //should be auto
	}else
		if (msg.compare("CAPTION_CUSTOM")==0){
			ui.comboCaptionColorMode->setCurrentIndex(1); //should be custom
	}
	slotChangeCaptionColorMode();
}
