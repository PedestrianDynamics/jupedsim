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
#include "TrajectoryPoint.h"

#include "network/TraVisToServer.h"
#include "geometry/FacilityGeometry.h"
#include "general/Macros.h"
#include "Debug.h"


using namespace std;


ThreadDataTransfer::ThreadDataTransfer(QObject *parent):
					QThread(parent)
{
	m_port=SystemSettings::getListeningPort();
	tcpServer=NULL;
	headerParsed=false;
	errNr=0;
	finished=false;
}



ThreadDataTransfer::~ThreadDataTransfer() {

}

void ThreadDataTransfer::shutdown(){
	finished=true;
}

void ThreadDataTransfer::run(){

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

	}else
		do{
			emit signal_CurrentAction("waiting for data");
			server->receiveMessage(data);

			if(!data.isEmpty())
				slotProcessMessage(data);

		}while(finished!=true);

	server->close();
}

void ThreadDataTransfer::slotHandleConnection(){

	Debug::Messages("handling new connection");

	QTcpSocket *clientConnection = tcpServer->nextPendingConnection();
	connect(clientConnection, SIGNAL(disconnected()),
			clientConnection, SLOT(deleteLater()));

	connect(clientConnection, SIGNAL(readyRead()), this, SLOT(slotReadMessage()));

	connect(clientConnection, SIGNAL(error(QAbstractSocket::SocketError)),
			this, SLOT(displayError(QAbstractSocket::SocketError)));
	//Q_DECLARE_METATYPE( QAbstractSocket::SocketError );
	Debug::Messages("juhuuuu");
}

void ThreadDataTransfer::slotReadMessage(){
	Debug::Messages("new post");
	//slotProcessMessage();
}

void ThreadDataTransfer::slotProcessMessage(QString& data){
	QDomDocument doc("");

	data = "<travisto>\n" +data+ "\n</travisto>\n";

	//cout<<data.toStdString()<<endl;

	QString errorMsg="";
	doc.setContent(data,&errorMsg);

	if(!errorMsg.isEmpty()){
		errNr++;
		Debug::Error(">> %s",(const char *)errorMsg.toStdString().c_str());
		Debug::Error(">> %s",(const char *)data.toStdString().c_str());
		return;
	}

	QDomNode header =doc.elementsByTagName("header").item(0);
	QDomNode geometry =doc.elementsByTagName("geometry").item(0);
	QDomNode shapes =doc.elementsByTagName("shape").item(0);

	QDomNodeList dataList = doc.elementsByTagName("frame");

	if(!header.isNull()) {
		parseHeaderNode(header);
		Debug::Messages("header received and parsed");
	}
	if(!shapes.isNull()) {
		parseShapeNode(shapes);
		Debug::Messages("header received and parsed");
	}

	if(!geometry.isNull()) {
		//emit signal_loadGeometry(data);
		geoData=data;
		//parseGeometryNode(geometry);
	}
	if(!dataList.isEmpty()) {
		parseDataNode(dataList);
	}

	data.clear();
}

void ThreadDataTransfer::slotConnectionClosed(){
	Debug::Error("connection lost");
}


void ThreadDataTransfer::parseHeaderNode(QDomNode header ){


	emit signal_CurrentAction("parsing new header");

	bool ok;
	numberOfAgents = getTagValueFromElement(header, "agents").toInt(&ok);

	if(!ok){
		Debug::Error("The number of agents is invalid");
		Debug::Error("The number must be between 1...65355");
		emit signal_errorMessage("The number of agents is invalid");
		numberOfAgents=1000;
	}

	if(numberOfAgents==extern_trajectories_firstSet.getNumberOfAgents()){
		// only clear
		emit signal_stopVisualisationThread(false);
	}else{
		emit signal_stopVisualisationThread(true);
	}


	// no need
	//unsigned long timeFirstFrame_us = header.toElement().elementsByTagName("timeFirstFrame").item(0)
	//unsigned long timeFirstFrame_s = header.toElement().elementsByTagName("timeFirstFrame").item(0)
	//	.toElement().attribute("sec",0).toUInt();
	QString frameRateStr=getTagValueFromElement(header, "frameRate");
	frameRate =frameRateStr.toFloat(&ok);

	if(!ok){
		Debug::Error("The frame rate is invalid");
		Debug::Error("The number must be between 1...1000");
		QMessageBox msgBox;
		msgBox.setText("The frame rate is invalid");
		msgBox.setInformativeText("The number must be between 1...1000,  I will consider 25");
		msgBox.setStandardButtons(QMessageBox::Ok);
		msgBox.setIcon(QMessageBox::Critical);
		msgBox.exec();
		frameRate=25;

	}
	headerParsed=true;
}

QString ThreadDataTransfer::getTagValueFromElement(QDomNode node,
		const char * tagName) {
	if (node.isNull())
		return "";
	return node.toElement().namedItem(tagName).firstChild().nodeValue();

}

void ThreadDataTransfer::parseDataNode(QDomNodeList frames){
	//static int frameNumbers=0;

	//parsing the data
	// TODO: i amybe a problem
	//emit signal_CurrentAction("parsing data");


    for (int i = 0; i < frames.length(); i++) {
		Frame *newFrame = new Frame();
		QDomElement el = frames.item(i).toElement();
		QDomNodeList agents = el.elementsByTagName("agent");
		//cout << "found:  " << agents.length() <<" agents" <<endl;
        for (int i = 0; i < agents.length(); i++) {

			bool ok=false;
			int id=agents.item(i).toElement().attribute("ID").toInt(&ok);
			if(!ok) continue; // invalid ID
            double xPos=agents.item(i).toElement().attribute("xPos","0").toDouble()*FAKTOR;
            double yPos=agents.item(i).toElement().attribute("yPos","0").toDouble()*FAKTOR;
            double zPos=agents.item(i).toElement().attribute("zPos","0").toDouble()*FAKTOR;

			double agent_color =std::numeric_limits<double>::quiet_NaN();

            double xVel=agents.item(i).toElement().attribute("xVel").toDouble(&ok)*FAKTOR;
			if(!ok)xVel=std::numeric_limits<double>::quiet_NaN();
            double yVel=agents.item(i).toElement().attribute("yVel").toDouble(&ok)*FAKTOR;
			if(!ok)yVel=std::numeric_limits<double>::quiet_NaN();
            double zVel=agents.item(i).toElement().attribute("zVel").toDouble(&ok)*FAKTOR;
			if(!ok)zVel=std::numeric_limits<double>::quiet_NaN();

			//coordinates of the ellipse, default to the head of the agent
            double el_x=agents.item(i).toElement().attribute("xEll").toDouble(&ok)*FAKTOR;
			if(!ok)	el_x=xPos;
            double el_y=agents.item(i).toElement().attribute("yEll").toDouble(&ok)*FAKTOR;
			if(!ok)	el_y=yPos;
            double el_z=agents.item(i).toElement().attribute("zEll").toDouble(&ok)*FAKTOR;
			if(!ok)	el_z=zPos;

            double dia_a=agents.item(i).toElement().attribute("radiusA").toDouble(&ok)*FAKTOR;
			if(!ok)dia_a=std::numeric_limits<double>::quiet_NaN();
            double dia_b=agents.item(i).toElement().attribute("radiusB").toDouble(&ok)*FAKTOR;
			if(!ok)dia_b=std::numeric_limits<double>::quiet_NaN();
			double el_angle=agents.item(i).toElement().attribute("ellipseOrientation").toDouble(&ok);
			if(!ok){el_angle=std::numeric_limits<double>::quiet_NaN(); }
			double el_color=agents.item(i).toElement().attribute("ellipseColor").toDouble(&ok);
			if(!ok)el_color=std::numeric_limits<double>::quiet_NaN();

			double pos[3]={xPos,yPos,zPos};
			double vel[3]={xVel,yPos,zPos};
			double ellipse[7]={el_x,el_y,el_z,dia_a,dia_b,el_angle,el_color};
			double para[2]={agent_color,el_angle};

			TrajectoryPoint * point = new TrajectoryPoint(id-1);
			point->setEllipse(ellipse);
			point->setPos(pos);
			point->setVel(vel);
			point->setAgentInfo(para);
			newFrame->addElement(point);
		}

		//adding the new frame to the right dataset
		extern_trajectories_firstSet.addFrame(newFrame);
		//	frameNumbers++;
	}

	//	cout <<"frames size: "<<extern_trajectories_firstSet.getSize()<<endl;
	//	cout <<"frames numbes: "<<frameNumbers<<endl;
	if(headerParsed==true){
		//		static int count=1;
		//		count++;
		//		if (count<100) return; // start after 100 frames
		emit signal_startVisualisationThread(geoData,numberOfAgents,frameRate);
		headerParsed=false;
		//		count=0;
	}
}


void ThreadDataTransfer::slotDisplayError(QAbstractSocket::SocketError socketError){

	switch (socketError) {

	case QAbstractSocket::RemoteHostClosedError:
		Debug::Error( "The host closes the connection ");
		break;

	case QAbstractSocket::HostNotFoundError:
		Debug::Error("The host was not found. Please check the ");
		Debug::Error("host name and port settings.");
		break;

	case QAbstractSocket::ConnectionRefusedError:
		Debug::Error("The connection was refused by the peer. ");
		Debug::Error("Make sure the fortune server is running");
		Debug::Error("and check that the host name and port ");
		Debug::Error("settings are correct.");
		break;

	default:
		Debug::Error("TraVisTo Client:");
		Debug::Error("The following error occurred: ");
		//cerr<< clientConnection->errorString().toStdString()<<endl;
		break;
	}


}

void ThreadDataTransfer::slotProcessPendingDatagrams() {
	Debug::Messages("connected");
	while (udpSocket->hasPendingDatagrams()) {
		QByteArray datagram;
		datagram.resize(udpSocket->pendingDatagramSize());
		udpSocket->readDatagram(datagram.data(), datagram.size());
		//		Debug::Messages("%s",(const char *) datagram.data().c_str()));
	}
}


void ThreadDataTransfer::parseShapeNode(QDomNode shape){


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

		if(!isnan(height)){
			heights.append(QString::number(id));
			heights.append(QString::number(height));
		}
		if(!isnan(color)){
			colors.append(QString::number(id));
			colors.append(QString::number(color));
		}

	}

	extern_trajectories_firstSet.setInitialHeights(heights);
	extern_trajectories_firstSet.setInitialColors(colors);
}
