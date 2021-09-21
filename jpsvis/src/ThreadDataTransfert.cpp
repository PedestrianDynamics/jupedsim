/**
 * @file ThreadDataTransfert.cpp
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
 *
 *  Created on: 24.05.2009
 *
 */


#include <cstdlib>
#include <iostream>
#include <limits>


#include <QtXml/QtXml>
#include <QtNetwork/QtNetwork>
#include <QThread>
#include <QObject>
#include <QMessageBox>
#include <QString>


#include "SystemSettings.h"
#include "ThreadDataTransfert.h"
#include "SyncData.h"
#include "Frame.h"
#include "FrameElement.h"

#include "network/TraVisToServer.h"
#include "geometry/FacilityGeometry.h"
#include "general/Macros.h"
#include "Log.h"


//using namespace std;


ThreadDataTransfer::ThreadDataTransfer(QObject *parent):
    QThread(parent)
{
    m_port=SystemSettings::getListeningPort();
    tcpServer=NULL;
    headerParsed=false;
    errNr=0;
    finished=false;
    //Debug::setDebugLevel(Debug::ALL);
}

ThreadDataTransfer::~ThreadDataTransfer()
{

}

void ThreadDataTransfer::shutdown()
{
    finished=true;
}

void ThreadDataTransfer::run()
{

    finished=false;

    //
    //	std::cerr <<">> starting server" <<std::endl;
    //
    //	//	udpSocket = new QUdpSocket();
    //	//	udpSocket->bind(QHostAddress::LocalHost,SystemSettings::getListeningPort());
    //	//
    //	//	if(!connect(udpSocket, SIGNAL(readyRead()),
    //	//			this, SLOT(slotProcessPendingDatagrams()))){
    //	//		cerr<<"could not create the udp connection"<<endl;
    //	//	}
    //
    //	tcpServer = new QTcpServer(/*this->parent()*/);
    //	if(!connect(tcpServer, SIGNAL(newConnection()), this, SLOT(slotHandleConnection()))){
    //		cerr<<"could not create connection newconnection"<<endl;
    //	}
    //	connect(tcpServer, SIGNAL(error(QAbstractSocket::SocketError)),
    //				this, SLOT(displayError(QAbstractSocket::SocketError)));
    //
    //	if (!tcpServer->listen(QHostAddress::LocalHost,SystemSettings::getListeningPort())) {
    //		cerr<<"\nI can't listen, sorry"<<endl;
    //		QMessageBox::critical(0, tr("TraVisTo Server"),
    //				tr("Unable to start the server: %1.")
    //				.arg(tcpServer->errorString()));
    //		quit();
    //	}
    //
    //	if(tcpServer->isListening ())
    //		cout<<"\nTraVisTo is listening on port "<< tcpServer->serverPort()<<endl;

    QString data;
    TraVisToServer* server = new TraVisToServer(SystemSettings::getListeningPort());
    if(!server->isListening()) {
        emit signal_errorMessage("could not connect, make sure that the port is not used by another program.\n "
                                 "Restarting the application may solve the issue!");
        emit signal_controlSequence("CONTROL_RESET");

    } else
        do {
            emit signal_CurrentAction("waiting for data");
            server->receiveMessage(data);

            if(!data.isEmpty())
                slotProcessMessage(data);

        } while(finished!=true);

    server->close();
}

void ThreadDataTransfer::slotHandleConnection()
{

    Log::Info("handling new connection");

    QTcpSocket *clientConnection = tcpServer->nextPendingConnection();
    connect(clientConnection, SIGNAL(disconnected()),
            clientConnection, SLOT(deleteLater()));

    connect(clientConnection, SIGNAL(readyRead()), this, SLOT(slotReadMessage()));

    connect(clientConnection, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(displayError(QAbstractSocket::SocketError)));
    //Q_DECLARE_METATYPE( QAbstractSocket::SocketError );
    Log::Info("juhuuuu");
}

void ThreadDataTransfer::slotReadMessage()
{
    Log::Info("new post");
    //slotProcessMessage();
}

void ThreadDataTransfer::slotProcessMessage(QString& data)
{
    QDomDocument doc("");

    //data = "<travisto>\n" +data+ "</travisto>";
    //cout<<data.toStdString()<<endl;

    QString errorMsg="";
    doc.setContent(data,&errorMsg);

    if(!errorMsg.isEmpty()) {
        errNr++;
        //Debug::Error(">> %s",(const char *)errorMsg.toStdString().c_str());
        //Debug::Error(">> %s",(const char *)data.toStdString().c_str());
        return;
    }

    QDomNode header =doc.elementsByTagName("header").item(0);
    QDomNode geometry =doc.elementsByTagName("geometry").item(0);
    QDomNode shapes =doc.elementsByTagName("shape").item(0);

    QDomNodeList dataList = doc.elementsByTagName("frame");

    if(!header.isNull()) {
        parseHeaderNode(header);
        Log::Info("header received and parsed");
    }
    if(!shapes.isNull()) {
        parseShapeNode(shapes);
        Log::Info("header received and parsed");
    }

    if(!geometry.isNull()) {
        //emit signal_loadGeometry(data);
        geoData=data;
        Log::Info("geometry received and parsed");
        //parseGeometryNode(geometry);
    }
    if(!dataList.isEmpty()) {
        parseDataNode(dataList);
    }

    data.clear();
}

void ThreadDataTransfer::slotConnectionClosed()
{
    Log::Error("connection lost");
}


void ThreadDataTransfer::parseHeaderNode(QDomNode header )
{


    emit signal_CurrentAction("parsing new header");

    bool ok;
    numberOfAgents = getTagValueFromElement(header, "agents").toInt(&ok);

    if(!ok) {
        Log::Error("The number of agents is invalid");
        Log::Error("The number must be between 1...65355");
        emit signal_errorMessage("The number of agents is invalid");
        numberOfAgents=1000;
    }

    if(numberOfAgents==extern_trajectories_firstSet.getNumberOfAgents()) {
        // only clear
        emit signal_stopVisualisationThread(false);
    } else {
        emit signal_stopVisualisationThread(true);
    }

    QString frameRateStr=getTagValueFromElement(header, "frameRate");
    frameRate =frameRateStr.toFloat(&ok);

    if(!ok) {
        Log::Error("The frame rate is invalid");
        Log::Error("The number must be between 1...1000");
        QMessageBox msgBox;
        msgBox.setText("The frame rate is invalid");
        msgBox.setInformativeText("The number must be between 1...1000,  I will consider 25");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
        frameRate=25;
    }

    //get the header version
    if(header.toElement().hasAttributes()) {
        QString version=header.toElement().attribute("version");
        QStringList query = version.split(".");
        int major=0;
        int minor=0;
        int patch=0;
        switch (query.size() ) {
        case 1:
            major=query.at(0).toInt();
            break;
        case 2:
            major=query.at(0).toInt();
            minor=query.at(1).toInt();
            break;
        case 3:
            major=query.at(0).toInt();
            minor=query.at(1).toInt();
            patch=query.at(2).toInt();
            break;
        }
        InitHeader(major,minor,patch);
        //cout<<"version found:"<<at.value(i).toStdString()<<endl;exit(0);
    }

    headerParsed=true;
}

QString ThreadDataTransfer::getTagValueFromElement(QDomNode node,
        const char * tagName)
{
    if (node.isNull())
        return "";
    return node.toElement().namedItem(tagName).firstChild().nodeValue();

}

void ThreadDataTransfer::parseDataNode(QDomNodeList frames)
{

    for (int i = 0; i < frames.length(); i++)
    {
        QDomElement el = frames.item(i).toElement();
        QDomNodeList agents = el.elementsByTagName("agent");
        int frame_id=el.attribute("ID").toInt();
        Frame *newFrame = new Frame(frame_id); // TODO: get the right frame id

        for (int i = 0; i < agents.length(); i++) {
            bool ok=false;
            int id=agents.item(i).toElement().attribute("ID").toInt(&ok);
            if(!ok) continue; // invalid ID
            double xPos=agents.item(i).toElement().attribute(_jps_xPos,"0").toDouble()*FAKTOR;
            double yPos=agents.item(i).toElement().attribute(_jps_yPos,"0").toDouble()*FAKTOR;
            double zPos=agents.item(i).toElement().attribute(_jps_zPos,"0").toDouble()*FAKTOR;

            double dia_a=agents.item(i).toElement().attribute(_jps_radiusA).toDouble(&ok)*FAKTOR;
            if(!ok)dia_a=std::numeric_limits<double>::quiet_NaN();
            double dia_b=agents.item(i).toElement().attribute(_jps_radiusB).toDouble(&ok)*FAKTOR;
            if(!ok)dia_b=std::numeric_limits<double>::quiet_NaN();
            double el_angle=agents.item(i).toElement().attribute(_jps_ellipseOrientation).toDouble(&ok);
            if(!ok) {
                el_angle=std::numeric_limits<double>::quiet_NaN();
            }
            double el_color=agents.item(i).toElement().attribute(_jps_ellipseColor).toDouble(&ok);
            if(!ok)el_color=std::numeric_limits<double>::quiet_NaN();

            double pos[3]= {xPos,yPos,zPos};
            //double vel[3]={xVel,yPos,zPos};
            //double ellipse[7]={el_x,el_y,el_z,dia_a,dia_b,el_angle,el_color};
            //double para[2]={agent_color,el_angle};
            double angle[3]= {0,0,el_angle};
            double radius[3]= {dia_a,dia_b,30.0};

            FrameElement *element = new FrameElement(id-1);
            element->SetPos(pos);
            element->SetOrientation(angle);
            element->SetRadius(radius);
            element->SetColor(el_color);
            newFrame->addElement(element);
        }

        //adding the new frame to the right dataset
        newFrame->ComputePolyData();
        extern_trajectories_firstSet.addFrame(newFrame);
        //	frameNumbers++;
    }

    if(headerParsed==true) {
        //		static int count=1;
        //		count++;
        //		if (count<100) return; // start after 100 frames
        emit signal_startVisualisationThread(geoData,numberOfAgents,frameRate);
        headerParsed=false;
    }
}


void ThreadDataTransfer::slotDisplayError(QAbstractSocket::SocketError socketError)
{

    switch (socketError) {

    case QAbstractSocket::RemoteHostClosedError:
        Log::Error( "The host closes the connection ");
        break;

    case QAbstractSocket::HostNotFoundError:
        Log::Error("The host was not found. Please check the ");
        Log::Error("host name and port settings.");
        break;

    case QAbstractSocket::ConnectionRefusedError:
        Log::Error("The connection was refused by the peer. ");
        Log::Error("Make sure the fortune server is running");
        Log::Error("and check that the host name and port ");
        Log::Error("settings are correct.");
        break;

    default:
        Log::Error("TraVisTo Client:");
        Log::Error("The following error occurred: ");
        //cerr<< clientConnection->errorString().toStdString()<<endl;
        break;
    }


}

void ThreadDataTransfer::slotProcessPendingDatagrams()
{
    Log::Info("connected");
    while (udpSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(udpSocket->pendingDatagramSize());
        udpSocket->readDatagram(datagram.data(), datagram.size());
        //		Debug::Info("%s",(const char *) datagram.data().c_str()));
    }
}


void ThreadDataTransfer::parseShapeNode(QDomNode shape)
{


    QDomNodeList agents = shape.toElement().elementsByTagName("agentInfo");
    QStringList heights;
    QStringList colors;
    for (int i = 0; i < agents.length(); i++) {

        bool ok=false;
        int id=agents.item(i).toElement().attribute("ID").toInt(&ok);
        if(!ok) continue; // invalid ID
        double height=agents.item(i).toElement().attribute("height").toDouble(&ok);
        if(!ok)height=std::numeric_limits<double>::quiet_NaN();

        int color=agents.item(i).toElement().attribute("color").toDouble(&ok);
        if(!ok)color=std::numeric_limits<int>::quiet_NaN();
        //cout <<"id= " <<id <<" height= "<<height<<" color= "<<color<<endl;

        //if(!std::isnan(height)) {
	if(height!=height) {
            heights.append(QString::number(id));
            heights.append(QString::number(height));
        }
        if(color!=color) {
            colors.append(QString::number(id));
            colors.append(QString::number(color));
        }

    }

    extern_trajectories_firstSet.setInitialHeights(heights);
    extern_trajectories_firstSet.setInitialColors(colors);
}

void ThreadDataTransfer::InitHeader(int major, int minor, int patch)
{
    // set the parsing String map
    if(minor==5 && patch==0) {
        _jps_xPos=QString("xPos");
        _jps_yPos=QString("yPos");
        _jps_zPos=QString("zPos");
        _jps_radiusA=QString("radiusA");
        _jps_radiusB=QString("radiusB");
        _jps_ellipseOrientation=QString("ellipseOrientation");
        _jps_ellipseColor=QString("ellipseColor");
    } else if ( (minor==6) || (minor==5 && patch==1) ) {
        _jps_xPos=QString("x");
        _jps_yPos=QString("y");
        _jps_zPos=QString("z");
        _jps_radiusA=QString("rA");
        _jps_radiusB=QString("rB");
        _jps_ellipseOrientation=QString("eO");
        _jps_ellipseColor=QString("eC");
    } else {
      std::cout<<"unsupported header version: "<<major<<"."<<minor<<"."<<patch<<std::endl;
      std::cout<<"Please use 0.5 0.5.1 or 0.6 "<<std::endl;
    }
}
