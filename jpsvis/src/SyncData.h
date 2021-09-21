/**
* @headerfile SyncData.cpp
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

#ifndef SYNCDATA_H_
#define SYNCDATA_H_

#include <QMutex>

#include <QObject>
#include <QStringList>

#include <memory>
#include <vector>
#include <map>
#include <vtkSmartPointer.h>
#include <vtkSphereSource.h>

class QObject;
class QStringList;
class Frame;

extern int extern_update_step;

class SyncData: public QObject {

    Q_OBJECT

public:
    SyncData();
    virtual ~SyncData();

    void add(std::string newData);
    std::string get();

    void resetFrameCursor();

    /// get the size
    unsigned int getSize();

    /// add a frame to the synchronized data
    void addFrame(Frame* frame);

    /// clears all frames
    void clearFrames();

    /// returns the total number of frames
    int getFramesNumber();

    /// return the position of the actual frame
    int getFrameCursor();

    /// set to cursor position
    void setFrameCursorTo(int position);

    /// return the frame at position i
    Frame* getFrame(int i);
     vtkSmartPointer<vtkSphereSource> getTTT();
     void setTTT(vtkSmartPointer<vtkSphereSource> s);


    /// return a pointer to the next frame
    Frame* getNextFrame();

    /// return a poiner to the previous frame
    Frame* getPreviousFrame();

    /// return the number of pedestrians involved in this dataset
    int getNumberOfAgents();

    std::map <int, Frame*>& GetFrames() {return _frames;}

    /// set the number of pedestrians
    void setNumberOfAgents(int _numberOfAgents);

    /// set the offset.
    /// this is useful when several datasets needed
    /// to be synchronized
    void setFrameCursorOffset(int offset);

    /// \brief initialize the pedestrians height.
    /// the initialiation is a list, where the even terms are the IDs
    /// and the odd terms are the heights
    void setInitialHeights(const QStringList& _pedHeight);

    ///  \brief get initial heights
    QStringList getInitialHeights()
    {
        return _pedHeight;
    }

    /**
     * \brief Set the pedestrian initial colors.
     * the initialiation is a list, where the even terms are the IDs
     * and the odd terms are the Colors
     * @param pedColor
     */
    void setInitialColors(const QStringList& pedColor)
    {
        _pedColor.clear();
        _pedColor=pedColor;
    };

    /**
     * \brief return the  initial colors
     */

    QStringList getInitialColors()
    {
        return _pedColor;
    }

Q_SIGNALS:
    /// send a control sequence to the main GUI.
    /// a control sequence could be STACK_EMPTY.
    void signal_controlSequences(const char* sex);

private:

    double _frameRate;
    char _roomCaption[256];

    // the actual position (real) of the frame
    int _frameCursor;

    // the offset. this is 0 when all datasets are synchronised
    int _frameCursorOffset;

    // list containing the initial heights of pedestrians
    QStringList _pedHeight;

    /// list containing the initial colors of pedestrians
    QStringList _pedColor;

    /// the number of agents
    int _numberOfAgents;

    ///used in online mode for syncronizing
    QMutex _mutex;
    //std::vector<Frame*> _frames;
    std::map <int, Frame*> _frames;
    //std::map<int, std::unique_ptr<Frame> > _frames;
     vtkSmartPointer<vtkSphereSource> _ttt;

};

#endif /* SYNCDATA_H_ */
