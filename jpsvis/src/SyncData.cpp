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
#include "Log.h"

#include <QMutex>
#include <QObject>
#include <QStringList>

#include <iostream>
#include <string>
#include <vector>
#include <fstream>

using namespace std;

SyncData::SyncData()
{
    _frameCursor=0;
    _numberOfAgents=0;
    _frameCursorOffset=0;
}

SyncData::~SyncData()
{

    //while (!_frames.empty()) {
    //    delete _frames.back();
    //    _frames.pop_back();
    //}
    //

    for(auto itr = _frames.begin(); itr != _frames.end(); itr++)
    {
        //FIXME
        //delete itr->second;
    }
    _frames.clear();
}



void SyncData::add(std::string newData)
{
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

std::string SyncData::get()
{
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

void SyncData::setFrameCursorOffset(int offset)
{
    _frameCursorOffset=offset;
}

void SyncData::addFrame(Frame* frame)
{
    _mutex.lock();
    _frames[frame->GetID()]=frame;
    //_frames.push_back(frame);
    _mutex.unlock();
}

Frame* SyncData::getFrame( int i)
{
    _mutex.lock();
    i+=_frameCursorOffset;

    if((i<0) || (i>= (int)_frames.size())) {
        _mutex.unlock();
        return NULL;
    } else {
        _mutex.unlock();
        return _frames.at(i);
    }

}

vtkSmartPointer<vtkSphereSource> SyncData::getTTT()
{
     return _ttt;
}

void SyncData::setTTT(vtkSmartPointer<vtkSphereSource> s)
{
     _ttt = s;
}


Frame* SyncData::getNextFrame()
{

    // this may be the case if the file only contains geometry, thus no trajectories available
    if(_frames.empty()) return nullptr;

    // Navigation  in the negative direction is also possible
    //review
    _mutex.lock();

    _frameCursor+=extern_update_step;

    //FIXME: do I really need two variables to handle this?
    int cursor =_frameCursor+_frameCursorOffset;

    if (cursor<0)
    {
        //frameCursor=0;
        emit signal_controlSequences("STACK_REACHS_BEGINNING");
        _mutex.unlock();
        return NULL;

    }
    else if ( _frames.count(cursor)==0)
    {

        //return the last frame, if I am at the end
        // otherwise retrun the first frame
        Frame* res=nullptr;
        if(_frameCursor>_frames.rbegin()->first)
        {
            //_frameCursor-=extern_update_step;
            res= _frames.rbegin()->second;
            _frameCursor=res->GetID();
        }
        else
        {
            //res=_frames.begin()->second;
            //_frameCursor=res->GetID();
        }

        _mutex.unlock();
        emit signal_controlSequences("CONTROL_STACK_EMPTY");
        return res;
    }

    Frame* res =_frames[cursor];
    _mutex.unlock();
    return res;
}

/***
* This method is for convenience only.
* The normal way to get the previous frame is:
*               1. either set the variable extern_update_step to a negative value;
*               2. using the function getFrame(int frameNumber). one may first get
* the current framecursor position using getFrameCursor()
*/
//Frame* SyncData::getPreviousFrame()
//{
//    _mutex.lock();
//    _frameCursor--;
//    //FIXME: do I really need two variables to handle this?
//    int cursor =_frameCursor+_frameCursorOffset;

//    if(cursor<0) {
//        //emit signal_controlSequences("STACK_REACHS_BEGINNING");
//        //frameCursor=0;
//        _mutex.unlock();
//        return NULL;
//    } else if((unsigned)cursor>=_frames.size() ) {
//        //emit signal_controlSequences("CONTROL_STACK_EMPTY");
//        _mutex.unlock();
//        //frameCursor=frames.size()-1;
//        return NULL;
//    }

//    Frame* res =_frames.at(cursor);

//    _mutex.unlock();

//    return res;
//}

void SyncData::clearFrames()
{
    _mutex.lock();

    _frameCursor=0;
    _numberOfAgents=0;
    _frameCursorOffset=0;
    _pedHeight.clear();
    _pedColor.clear();

    //    while (!_frames.empty()) {
    //        delete _frames.back();
    //        _frames.pop_back();
    //    }
    _frames.clear();
    _mutex.unlock();
}

int SyncData::getFramesNumber()
{
    //mutex.lock(); //FIXME
    return _frames.size();
    //mutex.unlock();
}

void SyncData::resetFrameCursor()
{
    _mutex.lock();
    _frameCursor=0;
    _mutex.unlock();
}


int SyncData::getFrameCursor()
{
    return _frameCursor;
}

void SyncData::setFrameCursorTo(int position)
{

    _mutex.lock();

    //TODO: check the unsigned
    //if((unsigned)position>=frames.size())	frameCursor =frames.size()-1;
    //else if (position<0) frameCursor =0;
    //else
    _frameCursor=position;

    _mutex.unlock();
}

int SyncData::getNumberOfAgents()
{
    return _numberOfAgents;
}

void SyncData::setNumberOfAgents(int numberOfAgents)
{
    _mutex.lock();
    _numberOfAgents = numberOfAgents;
    _mutex.unlock();
}

void SyncData::setInitialHeights(const QStringList& pedHeight)
{
    _pedHeight.clear();
    _pedHeight=pedHeight;
}

unsigned int SyncData::getSize()
{
    if(_frames.empty()) return 0;
    else return _frames.size();
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
