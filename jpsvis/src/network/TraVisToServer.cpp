/**
* @headerfile TraVisToServer.cpp
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

#include <QMessageBox>

#include "TraVisToServer.h"

#include <iostream>


using namespace std;

TraVisToServer::TraVisToServer(quint16 port)
{
    this->port=port;
    isConnected=false;
    listening=false;
    //QObject(0);
    qRegisterMetaType<QAbstractSocket::SocketError>("QAbstractSocket::SocketError");
    createConnection();

}

TraVisToServer::~TraVisToServer()
{
    if(tcpServer->isListening()) {
        tcpServer->close();
    }
    delete tcpServer;

}

void TraVisToServer::close()
{
    tcpServer->close();
}

void TraVisToServer::slotHandleConnection()
{

    //	cerr<<"handling new connection"<<endl;
    //
    ////	while(tcpServer->waitForNewConnection(-1)){
    ////		cerr<<"uuuu"<<endl;
    ////	}
    //	QTcpSocket *clientConnection = tcpServer->nextPendingConnection();
    //	    connect(clientConnection, SIGNAL(disconnected()),
    //	            clientConnection, SLOT(deleteLater()));
    //
    //	    if(clientConnection->waitForReadyRead(-1)){
    //	    	cerr<<"could somehow not read from the device"<<endl;
    //	    }
    //
    //	//    while(1){
    //receive data until disconnected

    //	cerr<<"juhuuuu"<<endl;

    //receive geometry

    //	    	// receive trajectories
    //	    	QDataStream in(clientConnection);
    //	    	in.setVersion(QDataStream::Qt_4_0);
    //
    //	    	if (blockSize == 0) {
    //	    		if (clientConnection->bytesAvailable() < (int)sizeof(quint16))
    //	    			return;
    //	    		in >> blockSize;
    //	    	}
    //
    //	    	if (clientConnection->bytesAvailable() < blockSize)
    //	    		return;
    //
    //	    	QString nextFortune;
    //	    	in >> nextFortune;
    //
    //	    	cerr<<nextFortune.toStdString()<<endl;

    //    }

}


void TraVisToServer::createConnection()
{

    tcpServer = new QTcpServer();
    tcpServer->setMaxPendingConnections(2);

    if (!tcpServer->listen(QHostAddress::Any,port)) {
        cerr<<"I cant listen to ALL interfaces"<<endl;
        cerr<<"I will try to listen only the LocalHost (127.0.0.1)"<<endl;

        if (!tcpServer->listen(QHostAddress::LocalHost,port)) {
            cerr<<"I cant listen to LocalHost either, sorry"<<endl;

            //QMessageBox::critical(0, tr("TraVisTo Server"),
            //		tr("Unable to start the server: %1.").arg(tcpServer->errorString()));
            //		close();
            return;
        }
    }
    if(!connect(tcpServer, SIGNAL(newConnection()), this, SLOT(slotHandleConnection()))) {
        cerr<<"could not create connection newconnection"<<endl;
    }

    if(tcpServer->isListening ()) {
        listening=true;
        cerr<<"\nJPSvis is listening on port "<< tcpServer->serverPort()<<endl;
    }
    //slotHandleConnection();
}


void TraVisToServer::receiveMessage(/*char* result*/ QString & result)
{

    result.clear();

    if(!isConnected) {
        // pool each 1000 ms. bad idea i Know !
        if(!tcpServer->waitForNewConnection(1000)) {
//			cerr<<"something went wrong, with the server"<<endl;
//			cerr <<" 200 ms passed !"<<endl;
            return;
        }

        clientConnection = tcpServer->nextPendingConnection();
        connect(clientConnection, SIGNAL(disconnected()),
                clientConnection, SLOT(deleteLater()));

        //		if(!clientConnection->waitForReadyRead()){
        //			cerr<<"could somehow not read from the device"<<endl;
        //			return;
        //		}
        if(!connect(clientConnection, SIGNAL(readyRead()), this, SLOT(slotReadMessage()))) {
            cerr<<"can't connect ready read slot"<<endl;
        }

        qRegisterMetaType<QAbstractSocket::SocketError>("QAbstractSocket::SocketError");
        if(!connect(clientConnection, SIGNAL(error(QAbstractSocket::SocketError)),
                    this, SLOT(slotDisplayError(QAbstractSocket::SocketError)))) {
            cerr<<"cant connect error slot"<<endl;
        }

        isConnected=true;
    }

    if(!clientConnection->waitForReadyRead()) {
        clientConnection->disconnectFromHost();
        clientConnection->abort();
        isConnected=false; // the connection is lost
        cerr<<"Connection was closed"<<endl;
        return;
    }

    while (clientConnection->bytesAvailable() < (int)sizeof(quint16)) {
        if (!clientConnection->waitForReadyRead(-1)) {
            // emit error(socket.error(), socket.errorString());
            cout << "o000ooh la lalala"<<endl;
            return;
        }
    }
    //	quint16 blockSize;
    //	QDataStream in(clientConnection);
    //	in.setVersion(QDataStream::Qt_4_0);
    //	in >> blockSize;

    //first the size of the data
    char tell[10];
    clientConnection->readLine(tell,10);
    //	 cout<<"line: "<<tell<<endl;

    blockSize = atoi(tell);

    while (clientConnection->bytesAvailable() < blockSize) {
        if (!clientConnection->waitForReadyRead(-1)) {
            //emit error(clientConnection->error(), clientConnection->errorString());
            cout << "ouh la lalala"<<endl;
            return;
        }
    }


    //	QString fortune;
    //	in >> result;
    //clientConnection->flush();

    //	 blockSize =clientConnection->read(sizeof(quint16)).toUShort();
    //	quint16 toRead=0;
    //	QString mesg;

    //	while (toRead < blockSize) {
    //		toRead+=clientConnection->bytesAvailable();
    //		QByteArray fr =clientConnection->read(toRead);
    //		clientConnection->flush();
    //		mesg.append(fr);
    //		cout<<toRead <<":"<<blockSize<<endl;
    //	}
    //	cout <<"data size:"<<clientConnection->bytesAvailable() <<":" << blockSize<<endl;
    QByteArray fr =clientConnection->read(blockSize);
    //	char *data = (char*)malloc(blockSize*sizeof(char));;
    //	cout<<data<<endl;
    //	cout<< clientConnection->read(data,blockSize)<<" read"<<endl;
    //	cout<<data<<endl;
    result = QString(fr);
    //	in >> result;
    //	 cout <<"size: "<<result.length()<<endl;
    //cout <<"size: "<<result.toStdString()<<endl;
    //	 cout <<"size: "<<fr.size()<<endl;
    //	QString msg(fr);

    //	result=msg;
    //	cout << "data: "<< count++<< msg.toStdString()<<endl;
    //	cout << "data: "<< count++<<":" << blockSize<<endl;
    //	cout << "data: "<< count++<<":" << data<<endl;
    //	cout << "data: "<< count++<< size<<endl;


    //	//get all data
    //	QByteArray data = clientConnection->readAll();
    //
    //	//result= new char[data.size()];
    //	result=data.data();

    //	//if(!data.isEmpty())
    //		cout <<" size>>"<< blockSize<<endl;
    //	cout <<" >>"<<result.toStdString()<<endl;
    //	system("pause");
    //		//cout <<" >>"<<result.t<<endl;

    //cout <<result.toStdString()<<endl;
}


void TraVisToServer::sendMessage(char* data)
{


}

void TraVisToServer::slotDisplayError(QAbstractSocket::SocketError socketError)
{
    switch (socketError) {

    case QAbstractSocket::RemoteHostClosedError:
        cerr<< "The host closed the connection "<<endl;
        break;

    case QAbstractSocket::HostNotFoundError:
        cerr<< "The host was not found. Please check the "<<endl;
        cerr<< "host name and port settings."<<endl;
        break;

    case QAbstractSocket::ConnectionRefusedError:
        cerr<<"The connection was refused by the peer. "<<endl;
        cerr<< "Make sure the fortune server is running, "<<endl;
        cerr<<"and check that the host name and port "<<endl;
        cerr<<"settings are correct."<<endl;
        break;

    default:
        cerr<<"TraVisTo Client:"<<endl;
        cerr<<"The following error occurred: "<<endl;
        //cerr<< clientConnection->errorString().toStdString()<<endl;
        break;
    }

}
void TraVisToServer::slotConnectionClosed()
{
    //clientConnection->deleteLater();
    isConnected=false;
    //cerr<< "connection lost"<<endl;
}

void TraVisToServer::slotReadMessage()
{
    //clientConnection->deleteLater();
    //isConnected=false;
    //	cerr<< "you have a post"<<endl;

}
