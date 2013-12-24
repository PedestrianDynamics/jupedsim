/**
* @file SyncData.cpp
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
* This class contains the data (trajectories / floor field) after they have
* been parsed in the appropriate structure. They are either read from a file or obtained via a TCP socket
*
* \brief maintains the parsed data in an appropriate structure
*
*
*  Created on: 02.06.2009
*
*/

#include "SyncData.h"
#include "Frame.h"
#include "TrajectoryPoint.h"
#include "Message.h"
#include "Debug.h"

#include <QMutex>
#include <QObject>
#include <QStringList>

#include <iostream>
#include <string>
#include <vector>
#include <fstream>

using namespace std;

SyncData::SyncData() {
	frameCursor=0;
	numberOfAgents=0;
	delay_ms_rel=0; //absolute
	delay_s_abs=0; //relative
	delay_us_abs=0; //absolute
	frameCursorOffset=0;

}

SyncData::~SyncData() {
	//	trajectories.clear();
	while (!frames.empty()){
		delete frames.back();
		frames.pop_back();
	}
	frames.clear();
}



void SyncData::add(std::string newData){
	//	mutex.lock();
	//	trajectories.push_back(newData);
	//	mutex.unlock();
}


// and the clearframe alternative for offline visualisation
//void SyncData::clear(){
//	//	mutex.lock();
//	//	frameCursor=0;
//	//	trajectories.clear();
//	//	numberOfAgents=0;
//	//	mutex.unlock();
//}

std::string SyncData::get(){
	std::string res;
	//	mutex.lock();
	//	if(trajectories.empty()) {
	//		res="";
	//		emit signal_controlSequences("CONTROL_STACK_EMPTY");
	//	}
	//	else{
	//		res= trajectories.front();
	//		trajectories.erase(trajectories.begin());
	//	}
	//#ifdef _DEBUG
	//	std::cout<<"size: " <<trajectories.size()<<std::endl;
	//#endif
	//	mutex.unlock();
	return res;
}

void SyncData::setFrameCursorOffset(int offset){
	frameCursorOffset=offset;
}

void SyncData::addFrame(Frame* frame){
	mutex.lock();
	frames.push_back(frame);
	mutex.unlock();
}

Frame* SyncData::getFrame(unsigned int i){
	mutex.lock();
	i+=frameCursorOffset;

	if((i<0) || (i>=frames.size())){
		mutex.unlock();
		return NULL;
	}
	else{
		mutex.unlock();
		return frames.at(i);
	}

}

Frame* SyncData::getNextFrame(){

	// this may be the case if the file only contains geometry, thus no trajectories available
	if(frames.empty()) return NULL;

	// Navigation  in the negative direction is also possible
	//review
	mutex.lock();

	frameCursor+=extern_update_step;

	//FIXME: do I really need two variables to handle this?
	int cursor =frameCursor+frameCursorOffset;

	if (cursor<0) {
		//frameCursor=0;
		emit signal_controlSequences("STACK_REACHS_BEGINNING");
		mutex.unlock();
		return NULL;

	}else if ((unsigned)cursor>=frames.size()){

		//if(extern_offline_mode)
		emit signal_controlSequences("CONTROL_STACK_EMPTY");
		//frameCursor=frames.size()-1;
		mutex.unlock();
		// FIXME: check me, return the last frame, if in o
		//return frames.at(frames.size()-1);
		frameCursor-=extern_update_step;
		return frames.back();
		//return NULL;
	}

	Frame* res =frames.at(cursor);
	mutex.unlock();
	return res;
}

/***
* This method is for convenience only.
* The normal way to get the previous frame is:
* 		1. either set the variable extern_update_step to a negative value;
* 		2. using the function getFrame(int frameNumber). one may first get
* the current framecursor position using getFrameCursor()
*/
Frame* SyncData::getPreviousFrame(){

	mutex.lock();
	frameCursor--;
	//FIXME: do I really need two variables to handle this?
	int cursor =frameCursor+frameCursorOffset;

	if(cursor<0) {
		//emit signal_controlSequences("STACK_REACHS_BEGINNING");
		//frameCursor=0;
		mutex.unlock();
		return NULL;
	}else
		if((unsigned)cursor>=frames.size() ) {
			//emit signal_controlSequences("CONTROL_STACK_EMPTY");
			mutex.unlock();
			//frameCursor=frames.size()-1;
			return NULL;
		}

	Frame* res =frames.at(cursor);

	mutex.unlock();

	return res;
}

void SyncData::clearFrames(){
	mutex.lock();

	frameCursor=0;
	numberOfAgents=0;
	frameCursorOffset=0;
	pedHeight.clear();
	pedColor.clear();

	while (!frames.empty()){
		delete frames.back();
		frames.pop_back();
	}
	frames.clear();

	mutex.unlock();
}

int SyncData::getFramesNumber(){
	//mutex.lock(); //FIXME
	return frames.size();
	//mutex.unlock();
}

void SyncData::resetFrameCursor(){
	mutex.lock();
	frameCursor=0;
	mutex.unlock();
}


int SyncData::getFrameCursor(){

	return frameCursor;
}

void SyncData::setFrameCursorTo(int position){

	mutex.lock();

	//TODO: check the unsigned
	//if((unsigned)position>=frames.size())	frameCursor =frames.size()-1;
	//else if (position<0) frameCursor =0;
	//else
	frameCursor=position;

	mutex.unlock();
}

int SyncData::getNumberOfAgents()
{
	return 0;
	return numberOfAgents;
}

void SyncData::setNumberOfAgents(int numberOfAgents)
{
	mutex.lock();
	this->numberOfAgents = numberOfAgents;
	mutex.unlock();
}

void SyncData::setDelayAbsolute(unsigned long second, unsigned long microsecond=0){
	delay_s_abs=second;
	delay_us_abs=microsecond;

}

///@warning only handle seconds, microseconds are ignored
void SyncData::computeDelayRelative(unsigned long* delays){
	unsigned long sec = delays[0];
	unsigned long usec = delays[1];
	long double delay_a = sec*1000 + (double)usec/1000.0;
	long double delay_b = delay_s_abs*1000 + (double)delay_us_abs/1000.0;

	delay_ms_rel=delay_a-delay_b;
	//std::cerr <<"the delay is: "  << delay_ms_rel<<std::endl;
	//delay_ms_rel=(sec-delay_s_abs)*1000;

	if (delay_ms_rel<0){
		Debug::Warning("warning: negative delay found");
	}

}

/// @warning the old value obtained from computeDelayRelative is just overwritten
//
void SyncData::setDelayRelative(signed long milliseconds){
	delay_ms_rel=milliseconds;

}


void SyncData::getDelayAbsolute(unsigned long* delays){
	delays[0]=delay_s_abs;
	delays[1]=delay_us_abs;
}

signed long SyncData::getDelayRelative(){
	return delay_ms_rel;
}

void SyncData::setInitialHeights(const QStringList& pedHeight){
	this->pedHeight.clear();
	this->pedHeight=pedHeight;
}

unsigned int SyncData::getSize(){
	if(frames.empty()) return 0;
	else return frames.size();
}


//bool SyncData::writeToFile(char* fileName,int version){
//	ofstream myfile (fileName);
//		if (myfile.is_open())
//		{
//			//header
//			myfile << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"<<endl;
//			myfile << "<trajectoriesDataset>"<<endl;
//			myfile << "<header formatVersion= \""<<version<<"\">"<<endl;
//			myfile << "       <roomCaption>"<<roomCaption<<"</roomCaption>"<<endl;
//			myfile << "        <agents>"<<numberOfAgents<<"</agents>"<<endl;
//			myfile << "       <frameRate>"<<frameRate<<"</frameRate> <!--per second-->"<<endl;
//			myfile << "      <timeFirstFrame sec=\"\" microsec=\"\"/> "<<endl;
//			myfile << "</header>"<<endl<<endl;
//
//			// shape
//			myfile << "<shape>"<<endl;
//			for(int i=0;i<pedHeight.size()-1;i+=2){
//						bool ok=false;
//						int id = pedHeight[i].toInt(&ok);
//						if(!ok) {cerr<<"skipping size arguments" <<endl;continue;}
//						double size= pedHeight[i+1].toDouble(&ok);
//						if(!ok) {cerr<<"skipping size arguments" <<endl;continue;}
//						myfile<<"<agentInfo ID=\""<<i<<" height=\""<<size<<"\" color =\"TBD\" "<<endl;
//			}
//
//			myfile << "</shape>"<<endl;
//			// geometry
//
//			// trajectories
//
//			for (int i=0;i<getSize();i++){
//						Frame* frame = getFrame(i);
//						if(frame==NULL){
//					//		cerr<<"Trajectory not available in getTrail(), first data set"<<endl;
//						}else {
//							myfile <<"frame ID=\""<<i<<"\"> "<< endl;
//							TrajectoryPoint* point=NULL;
//							while(NULL!=(point=frame->getNextElement())){
//								if(version==1){
//
//								}else if (version==2){
////									myfile <<"<agent ID=\""<<point->getIndex()<<"\"";
////									myfile <<" xPos=\""<<1311.00<<" yPos=\""<<828.00<<" zPos=\""<<0.00<<"\"";
////									myfile <<" agentOrientation =\""<<10<<"\"";
////									myfile <<" xVel=\""<<1311.00<<" yVel=\""<<828.00<<" zVel=\""<<0.00<<"\"";
////									myfile <<" diameterA=\""<<1311.00<<" diameterB=\""<<828.00<<"\"";
////									myfile <<" ellipseOrientation=\""<<1311.00<<" ellipseColor=\""<<828.00<<"\"";
//
//								}
//							}
//							frame->resetCursor();
//							myfile <<"/frame>"<< endl;
//						}
//					}
//
//
//			myfile << "</trajectoriesDataset>"<<endl;
//			myfile.close();
//			return true;
//		}
//		else return false;
//
//}
