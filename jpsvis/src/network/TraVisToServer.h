/**
* @headerfile TraVisToServer.h
* @author   Ulrich Kemloh <kemlohulrich@gmail.com>
* @version 0.1
* Copyright (C) <2009-2010>
*
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
* @brief provide a server that listens to incoming trajectories stream
*
*  Created on: 22.09.2009
*
*/


#ifndef TRAVISTOSERVER_H_
#define TRAVISTOSERVER_H_

#include<QtNetwork/QtNetwork>
#include<QObject>


#define PORT 8989

class TraVisToServer:public QObject {

    Q_OBJECT

public:
    TraVisToServer(quint16 port=PORT);
    virtual ~TraVisToServer();

    //void receiveMessage(char* data);
    void receiveMessage(QString& msg);
    void sendMessage(char* data);
    bool isListening()
    {
        return listening;
    }
    void close();

private:
    void createConnection();
    void closeConnection();

private Q_SLOTS:
    void slotHandleConnection();
    void slotReadMessage();
    void slotConnectionClosed();
    void slotDisplayError(QAbstractSocket::SocketError socketError);
    //void slotConnected();
    //void slotDisconnected();


private:
    bool isConnected;
    bool listening;
    //QTcpSocket *tcpSocket;
    //QTcpSocket *clientConnection;
    QTcpSocket *clientConnection;
    QTcpServer *tcpServer;
    QString currentFortune;
    int blockSize;
    quint16 port;

};

#endif /* TRAVISTOSERVER_H_ */
