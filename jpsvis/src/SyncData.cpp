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
 * been parsed in the appropriate structure. They are either read from a file or obtained via a TCP
 * socket
 *
 * \brief maintains the parsed data in an appropriate structure
 *
 *
 *  Created on: 02.06.2009
 *
 */

#include "SyncData.h"

#include "Frame.h"
#include "Log.h"
#include "Message.h"
#include "TrajectoryPoint.h"

#include <QMutex>
#include <QObject>
#include <QStringList>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

SyncData::SyncData()
{
    _frameCursor       = 0;
    _numberOfAgents    = 0;
    _frameCursorOffset = 0;
}

SyncData::~SyncData()
{
    for(auto itr = _frames.begin(); itr != _frames.end(); itr++) {
        // FIXME
        // delete itr->second;
    }
    _frames.clear();
}


void SyncData::add(std::string newData) {}

std::string SyncData::get()
{
    std::string res;
    return res;
}

void SyncData::setFrameCursorOffset(int offset)
{
    _frameCursorOffset = offset;
}

void SyncData::addFrame(Frame * frame)
{
    _mutex.lock();
    _frames[frame->GetID()] = frame;
    //_frames.push_back(frame);
    _mutex.unlock();
}

Frame * SyncData::getFrame(int i)
{
    _mutex.lock();
    i += _frameCursorOffset;

    if((i < 0) || (i >= (int) _frames.size())) {
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


Frame * SyncData::getNextFrame()
{
    // this may be the case if the file only contains geometry, thus no trajectories available
    if(_frames.empty())
        return nullptr;

    // Navigation  in the negative direction is also possible
    // review
    _mutex.lock();

    _frameCursor += extern_update_step;

    // FIXME: do I really need two variables to handle this?
    int cursor = _frameCursor + _frameCursorOffset;

    if(cursor < 0) {
        // frameCursor=0;
        emit signal_controlSequences("STACK_REACHS_BEGINNING");
        _mutex.unlock();
        return NULL;

    } else if(_frames.count(cursor) == 0) {
        // return the last frame, if I am at the end
        // otherwise retrun the first frame
        Frame * res = nullptr;
        if(_frameCursor > _frames.rbegin()->first) {
            res          = _frames.rbegin()->second;
            _frameCursor = res->GetID();
        } else {
        }

        _mutex.unlock();
        emit signal_controlSequences("CONTROL_STACK_EMPTY");
        return res;
    }

    Frame * res = _frames[cursor];
    _mutex.unlock();
    return res;
}

void SyncData::clearFrames()
{
    _mutex.lock();

    _frameCursor       = 0;
    _numberOfAgents    = 0;
    _frameCursorOffset = 0;
    _pedHeight.clear();
    _pedColor.clear();

    _frames.clear();
    _mutex.unlock();
}

int SyncData::getFramesNumber()
{
    return _frames.size();
}

void SyncData::resetFrameCursor()
{
    _mutex.lock();
    _frameCursor = 0;
    _mutex.unlock();
}


int SyncData::getFrameCursor()
{
    return _frameCursor;
}

void SyncData::setFrameCursorTo(int position)
{
    _mutex.lock();

    _frameCursor = position;

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

void SyncData::setInitialHeights(const QStringList & pedHeight)
{
    _pedHeight.clear();
    _pedHeight = pedHeight;
}

unsigned int SyncData::getSize()
{
    if(_frames.empty())
        return 0;
    else
        return _frames.size();
}
