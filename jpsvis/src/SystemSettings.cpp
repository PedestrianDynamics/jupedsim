/**
 * @file SystemSettings.cpp
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
 * \brief contains travisto global settings
 *
 *
 *
 *  Created on: 14.09.2009
 *
 */


#include <QDir>
#include <QColor>

#include "SystemSettings.h"
#include "Debug.h"
#include "./forms/Settings.h"

bool SystemSettings::showLegend=false;
bool SystemSettings::pedColorProfileReadFromFile=true;
bool SystemSettings::showAgentsCaptions=false;
bool SystemSettings::is2D=false;
bool SystemSettings::showAgents=true;
bool SystemSettings::showGeometry=true;
unsigned short SystemSettings::port=8989;
double SystemSettings::bgColor[]={1.0,1.0,1.0};
int SystemSettings::pedestriansColor[3][3]={{255 , 17, 224},{122, 255, 122},{130, 130, 130}};
int SystemSettings::pedesShape=Settings::PINGUINS;
int SystemSettings::ellipseResolution=10;
int SystemSettings::trailCount=50;
int SystemSettings::trailingType=1;
int SystemSettings::trailingGeometry=1;
int SystemSettings::virtualAgentID=-1;

int SystemSettings::captionSize=30;
QColor SystemSettings::captionColor=QColor();
int SystemSettings::captionOrientaton=1;
bool SystemSettings::captionAutomaticRotation=1;
bool SystemSettings::onScreenInfos=true;

bool SystemSettings::recordPNGsequence=false;
//QString SystemSettings::outputDir =QDir::currentPath()+"/";
QString SystemSettings::outputDir  = QDir::homePath() +"/Desktop/TraVisTo_Files/";
QString SystemSettings::workingDir  = QDir::currentPath();
QString SystemSettings::filesPrefix="";



SystemSettings::SystemSettings() {}

SystemSettings::~SystemSettings() {}

void  SystemSettings::setShowLegend(bool legend){
	showLegend=legend;

}

bool  SystemSettings::getShowLegend(){
	return showLegend;
}


void SystemSettings::setListningPort(unsigned short porta){
	port=porta;
}

unsigned short   SystemSettings::getListeningPort(){
	return port;
}


void  SystemSettings::setShowAgentsCaptions(bool caption){
    showAgentsCaptions=caption;
}

bool  SystemSettings::getShowAgentsCaptions(){
    return showAgentsCaptions;
}

void SystemSettings::set2D(bool lis2D){
	is2D=lis2D;
}
bool SystemSettings::get2D(){
	return is2D;
}

// set/get the  2D visio
void SystemSettings::setShowAgents(bool status)
{
    showAgents=status;
}

bool SystemSettings::getShowAgents()
{
    return showAgents;
}

void SystemSettings::setShowGeometry(bool status)
{
    showGeometry=status;
}

bool SystemSettings::getShowGeometry()
{
    return showGeometry;
}

void   SystemSettings::setWorkingDirectory(QString dir)
{
    workingDir=dir;
}

void   SystemSettings::getWorkingDirectory(QString& dir)
{
    dir=workingDir;
}

void  SystemSettings::getBackgroundColor(double* col){
	col[0]=bgColor[0];
	col[1]=bgColor[1];
	col[2]=bgColor[2];
}

void  SystemSettings::setBackgroundColor(double* col){
	bgColor[0]=col[0];
	bgColor[1]=col[1];
	bgColor[2]=col[2];
}



/// set/get pedestrian private sphere ellipse resolution
int SystemSettings::getEllipseResolution(){
	return ellipseResolution;
}

void SystemSettings::setEllipseResolution(int resolution){
	ellipseResolution=resolution;
}



/// set/get the pedestrian shape
/// 0 for default, 1 for Ellipse, 2 for pinguins
void SystemSettings::setPedestrianShape(int shape){
	pedesShape=shape;
}
int SystemSettings::getPedestrianShape(){
	return pedesShape;
}
void  SystemSettings::setTrailsInfo(int count, int type, int geo){
	trailCount=count;
	trailingType=type;
	trailingGeometry=geo;
}

void  SystemSettings::getTrailsInfo(int* count, int *type, int* geo){
	*count=trailCount;
	*type=trailingType;
	*geo=trailingGeometry;
}
void  SystemSettings::setPedestrianColor(int groupID, int color[3]){
	if((groupID<0) || (groupID>3)) {
		Debug::Error(" wrong pedestrian group ID %d",groupID);
		return;
	}
	pedestriansColor[groupID][0]=color[0];
	pedestriansColor[groupID][1]=color[1];
	pedestriansColor[groupID][2]=color[2];

}

void  SystemSettings::getPedestrianColor(int groupID, int color[3]){
	if((groupID<0) || (groupID>3)) {
		Debug::Error(" wrong pedestrian group ID %d",groupID);
		return;
	}

	color[0]=pedestriansColor[groupID][0];
	color[1]=pedestriansColor[groupID][1];
	color[2]=pedestriansColor[groupID][2];

}

void  SystemSettings::setOutputDirectory(QString dir){
	outputDir=dir;
}
void  SystemSettings::getOutputDirectory(QString& dir){
	dir=QString(outputDir);
}


//void SystemSettings::setPedestrianCaptionSize( int size){
//	captionSize =size;
//}

int SystemSettings::getPedestrianCaptionSize(){
	return captionSize;
}

void SystemSettings::setRecordPNGsequence(bool status){
	recordPNGsequence=status;
}
bool SystemSettings::getRecordPNGsequence(){
	return recordPNGsequence;
}

void SystemSettings::setFilenamePrefix(QString prefix){
	filesPrefix=prefix;
}

QString SystemSettings::getFilenamePrefix() {
	return filesPrefix;
}

void SystemSettings::setPedestrianColorProfileFromFile(bool readFromFile) {
	pedColorProfileReadFromFile = readFromFile;
}

bool SystemSettings::getPedestrianColorProfileFromFile() {
	return pedColorProfileReadFromFile;
}
void SystemSettings::setVirtualAgent(int agent){
	virtualAgentID=agent;
}

int  SystemSettings::getVirtualAgent(){
	return virtualAgentID;
}

void SystemSettings::setCaptionsParameters(int size, const QColor& col, int orientation,
		bool automaticRotation) {
	captionSize=size;
	captionColor=col;
	captionOrientaton=orientation;
	captionAutomaticRotation=automaticRotation;

}

void SystemSettings::getCaptionsParameters(int &size, QColor& col, int &orientation,
		bool &automaticRotation) {
	size=captionSize;
	col=captionColor;
	orientation=captionOrientaton;
	automaticRotation=captionAutomaticRotation;
}

void SystemSettings::setOnScreenInfos(bool status){
	onScreenInfos=status;
}

bool SystemSettings::getOnScreenInfos(){
	return onScreenInfos;
}

