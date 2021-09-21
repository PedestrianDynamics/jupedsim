/**
* @headerfile ThreadDataTransfert.cpp
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
* This class provides a thread utility that is charged of
* asynchronously receive trajectories (streaming)
* and  put them in the adequate data structure to be read by the Visualisation Thread.
*
* \brief Thread that reads data from the socket and parse it
*
*
*  Created on: 24.05.2009
*
*/



#ifndef THREADDATATRANSFERT_H_
#define THREADDATATRANSFERT_H_

#include <QThread>
#include <QObject>
#include <QtNetwork/QtNetwork>

class QThread;
class QObject;
class SyncData;
class QDomNode;
class QString;
class QDomNodeList;
class FacilityGeometry;

extern SyncData extern_trajectories_firstSet;
extern bool extern_shutdown_visual_thread;

class ThreadDataTransfer: public QThread {

    Q_OBJECT

public:
    ThreadDataTransfer(QObject *parent = 0);
    virtual ~ThreadDataTransfer();
    void run();
    void setListeningPort(int port);

    /// \brief force the thread to do some cleaning work and shutdown
    void shutdown();

    /// parse a shape node
    void parseShapeNode(QDomNode shapes);

    /// parse a geometry node
    void parseHeaderNode(QDomNode header);

    /// parse data node
    void parseDataNode(QDomNodeList data);

    //old and new geometry format
    void InitHeader(int major, int minor, int patch);


private:
    QString getTagValueFromElement(QDomNode node,
                                   const char * tagName);

Q_SIGNALS:
    void signal_start();
    void signal_stop();
    void signal_controlSequence(const char* sex);

    void signal_CurrentAction(QString msg);

    /// connect to the setFrameRateSlot
    /// of the visualisation Thread
    //void signal_frameRate(float fps);

    /// start the visual thread,
    /// once header has been received
    void signal_startVisualisationThread(QString data, int numberOfAgents,float frameRate);

    /// shutdown the visio thread
    void signal_stopVisualisationThread(bool clearAndStop);

    void signal_errorMessage(QString geometry);

public Q_SLOTS:
    //udp
    void slotProcessPendingDatagrams();
    //tcp
    void slotHandleConnection();
    void slotReadMessage();
    void slotProcessMessage(QString& data);
    void slotConnectionClosed();
    void slotDisplayError(QAbstractSocket::SocketError socketError);


private:
    int m_port;
    QUdpSocket* udpSocket;
    QTcpServer* tcpServer;
    bool headerParsed;
    long errNr;
    QString geoData;
    int numberOfAgents;
    float frameRate;
    bool finished;

    // temp variables to be removed in the next versions
    //header dependant variables
    QString _jps_xPos;
    QString _jps_yPos;
    QString _jps_zPos;
    QString _jps_radiusA;
    QString _jps_radiusB;
    QString _jps_ellipseOrientation;
    QString _jps_ellipseColor;
};

#endif /* THREADDATATRANSFERT_H_ */

