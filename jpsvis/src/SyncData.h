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
#include <vector>
#include <QObject>
#include <QStringList>

class QObject;
class QStringList;
class Frame;

extern int extern_update_step;
extern bool extern_offline_mode;

class SyncData: public QObject {

    Q_OBJECT

public:
    SyncData();
    virtual ~SyncData();

    void add(std::string newData);
    std::string get();
    //void clear();
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

    /// return a pointer to the next frame
    Frame* getNextFrame();

    /// return a poiner to the previous frame
    Frame* getPreviousFrame();

    /// return the number of pedestrians involved in this dataset
    int getNumberOfAgents();

    /// set the number of pedestrians
    void setNumberOfAgents(int numberOfAgents);

    /// set the absolute time (delay) after which, this dataset will start to play.
    /// This is useful when loading several datasets, that needs to be synchronised.
    /// @para second the number of seconds elapsed since midnight 1970
    /// @para microsecond the number of microsecond(in addition to the seconds)
    void setDelayAbsolute(unsigned long second, unsigned long microsecond/*=0*/);

    /// returns the arguments given in setDelay.
    void getDelayAbsolute(unsigned long *);

    /// get the relative delays between the dataset in millisecond.
    /// the delay of the first dataset/group will typically have the delay 0 ms.
    signed long getDelayRelative();

    /// set the relative delays

    void setDelayRelative(signed long milliseconds);

    /// compute the relative relative
    /// @deprecated [should use setDelayRelative]
    void computeDelayRelative(unsigned long* delays);

    /// set the offset.
    /// this is useful when several datasets needed
    /// to be synchronized
    void setFrameCursorOffset(int offset);

    /// \brief initialize the pedestrians height.
    /// the initialiation is a list, where the even terms are the IDs
    /// and the odd terms are the heights
    void setInitialHeights(const QStringList& pedHeight);

    ///  \brief get initial heights
    QStringList getInitialHeights()
    {
        return pedHeight;
    }

    /**
     * \brief Set the pedestrian initial colors.
     * the initialiation is a list, where the even terms are the IDs
     * and the odd terms are the Colors
     * @param pedColor
     */
    void setInitialColors(const QStringList& pedColor)
    {
        this->pedColor.clear();
        this->pedColor=pedColor;
    };

    /**
     * \brief return the  initial colors
     */

    QStringList getInitialColors()
    {
        return pedColor;
    }

Q_SIGNALS:
    /// send a control sequence to the main GUI.
    /// a control sequence could be STACK_EMPTY.
    void signal_controlSequences(const char* sex);

private:

    //general information about this dataset
    float frameRate;
    char roomCaption[256];

    // give the actual position of the frame beeing read, in the frame dataset

    // the actual position (real) of the frame
    int frameCursor;

    // the offset. this is 0 when all dataets are synchronised
    int frameCursorOffset;

    // relative delay in milliseconds
    signed long delay_ms_rel;
    // ablotute time elapsed since 1970 in sec
    unsigned long delay_s_abs;
    // absolute additional time in microsecond
    unsigned long delay_us_abs;

    // list containing the initial heights of pedestrians
    QStringList pedHeight;

    /// list containing the initial colors of pedestrians
    QStringList pedColor;

    /// the number of agents
    int numberOfAgents;
    QMutex mutex;
    std::vector<Frame*> frames;
};

#endif /* SYNCDATA_H_ */
