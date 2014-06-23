/**
 * @file    SaxParser.cpp
 * @author  Ulrich Kemloh <kemlohulrich@gmail.com>
 * @version 0.1
 * Created on: 6 Sep 2010
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
 *
 *
 *
 */

#include "SaxParser.h"
#include "TrajectoryPoint.h"
#include "Frame.h"
#include "SyncData.h"
#include "Debug.h"

#include "geometry/JPoint.h"
#include "geometry/FacilityGeometry.h"
#include "geometry/Building.h"

#include <QMessageBox>
#include <QString>
#include <limits>
#include <iostream>
#include <cmath>


#include <vtkVersion.h>
#include <vtkSmartPointer.h>
#include <vtkPolygon.h>
#include <vtkCellArray.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkAssembly.h>
#include <vtkProperty.h>
#include <vtkTriangleFilter.h>


using namespace std;

/**
 * constructor
 *
 * @param geo
 * @param data
 * @param par 0=fps, 1=agents
 * @param roomCaption
 * @return
 */
SaxParser::SaxParser(FacilityGeometry* geo, SyncData* data, double* fps){
	geometry=geo;
	dataset=data;
	para=fps;
	parsingWalls=false;
	color=0.0;

	dataset->clearFrames();
}

SaxParser::~SaxParser() {

}

bool SaxParser::startElement(const QString & /* namespaceURI */,
		const QString & /* localName */, const QString &qName,
		const QXmlAttributes &at)
{
    if (qName == "header") {
        for(int i=0;i<at.length();i++){
            if(at.localName(i)=="version")
            {
                double version=at.value(i).toDouble();
                InitHeader(version);
            }
        }
    }else if (qName == "file") {
        for(int i=0;i<at.length();i++){
            if(at.localName(i)=="location")
            {
                QString fileName=at.value(i);
                if(!fileName.isEmpty())
                {
                    if(fileName.endsWith(".xml",Qt::CaseInsensitive))
                    {
                        SaxParser::parseGeometryJPS(fileName,geometry);
                    }
                    else if (fileName.endsWith(".trav",Qt::CaseInsensitive))
                    {
                        SaxParser::parseGeometryTRAV(fileName,geometry);
                    }
                }
            }
        }
    }
    else if (qName == "floor") {
        double xMin=0,
                xMax=0,
                yMin=0,
                yMax=0;

        for(int i=0;i<at.length();i++){
            if(at.localName(i)=="xMin")
            {
                xMin=at.value(i).toDouble()*FAKTOR;
            }
            else if(at.localName(i)=="xMax")
            {
                xMax=at.value(i).toDouble()*FAKTOR;
            }
            else if(at.localName(i)=="yMin")
            {
                yMin=at.value(i).toDouble()*FAKTOR;
            }
            else if(at.localName(i)=="yMax")
            {
                yMax=at.value(i).toDouble()*FAKTOR;
            }

        }
        geometry->addFloor(xMin,yMin,xMax,yMax);
    }
    else if (qName == "cuboid") {
        double length=0, height=0,
                width=0, color=0;
        double center[3]={0,0,0};

        for(int i=0;i<at.length();i++){
            if(at.localName(i)=="centerX")
            {
                center[0]=at.value(i).toDouble()*FAKTOR;
            }
            else if(at.localName(i)=="centerY")
            {
                center[1]=at.value(i).toDouble()*FAKTOR;
            }
            else if(at.localName(i)=="centerZ")
            {
                center[2]=at.value(i).toDouble()*FAKTOR;
            }
            else if(at.localName(i)=="length")
            {
                length=at.value(i).toDouble()*FAKTOR;
            }
            else if(at.localName(i)=="height")
            {
                height=at.value(i).toDouble()*FAKTOR;
            }
            else if(at.localName(i)=="width")
            {
                width=at.value(i).toDouble()*FAKTOR;
            }
            else if(at.localName(i)=="color")
            {
                color=at.value(i).toDouble()*FAKTOR;
            }
        }
        geometry->addObjectBox(center,height,width,length,color);

    }
    else if (qName == "sphere") {
        double radius=0, color=0;
        double center[3]={0,0,0};

        for(int i=0;i<at.length();i++){
            if(at.localName(i)=="centerX")
            {
                center[0]=at.value(i).toDouble()*FAKTOR;
            }
            else if(at.localName(i)=="centerY")
            {
                center[1]=at.value(i).toDouble()*FAKTOR;
            }
            else if(at.localName(i)=="centerZ")
            {
                center[2]=at.value(i).toDouble()*FAKTOR;
            }
            else if(at.localName(i)=="radius")
            {
                radius=at.value(i).toDouble()*FAKTOR;
            }
            else if(at.localName(i)=="color")
            {
                color=at.value(i).toDouble();
            }
        }
        geometry->addObjectSphere(center,radius,color);
    }
    else if (qName == "label") {
        double  color=0;
        double center[3]={0,0,0};
        QString text;

        for(int i=0;i<at.length();i++){
            if(at.localName(i)=="centerX")
            {
                center[0]=at.value(i).toDouble()*FAKTOR;
            }
            else if(at.localName(i)=="centerY")
            {
                center[1]=at.value(i).toDouble()*FAKTOR;
            }
            else if(at.localName(i)=="centerZ")
            {
                center[2]=at.value(i).toDouble()*FAKTOR;
            }
            else if(at.localName(i)=="text")
            {
                text=at.value(i);
            }
            else if(at.localName(i)=="color")
            {
                color=at.value(i).toDouble();
            }
        }
        geometry->addObjectLabel(center,center,text.toStdString(),color);
    }
    else if (qName == "cylinder") {
        double height=0, radius=0, color=0;
        double center[3]={0,0,0};
        double rotation[3]={0,0,0};

        for(int i=0;i<at.length();i++){
            if(at.localName(i)=="centerX")
            {
                center[0]=at.value(i).toDouble()*FAKTOR;
            }
            else if(at.localName(i)=="centerY")
            {
                center[1]=at.value(i).toDouble()*FAKTOR;
            }
            else if(at.localName(i)=="centerZ")
            {
                center[2]=at.value(i).toDouble()*FAKTOR;
            }
            else if(at.localName(i)=="height")
            {
                height=at.value(i).toDouble()*FAKTOR;
            }
            else if(at.localName(i)=="radius")
            {
                radius=at.value(i).toDouble()*FAKTOR;
            }
            else if(at.localName(i)=="color")
            {
                color=at.value(i).toDouble();
            }
            else if(at.localName(i)=="angleX")
            {
                rotation[0]=at.value(i).toDouble();
            }
            else if(at.localName(i)=="angleY")
            {
                rotation[1]=at.value(i).toDouble();
            }
            else if(at.localName(i)=="angleZ")
            {
                rotation[2]=at.value(i).toDouble();
            }
        }
        geometry->addObjectCylinder(center,radius,height,rotation,color);
    }
    else if (qName == "agents") {
    } else if (qName == "roomCaption") {
    } else if (qName == "frameRate") {
    } else if (qName == "geometry") {
    }
    else if (qName == "wall")
    {
        parsingWalls=true;
        thickness=15;
        height=250;
        color=0;
        caption="";

        for(int i=0;i<at.length();i++){
            if(at.localName(i)=="thickness")
            {
                thickness=at.value(i).toDouble()*FAKTOR;
            }
            else if(at.localName(i)=="height")
            {
                height=at.value(i).toDouble()*FAKTOR;
            }
            else if(at.localName(i)=="color")
            {
                color=at.value(i).toDouble();
            }
            else if(at.localName(i)=="caption")
            {
                caption=at.value(i);
            }
        }

    }
    else if (qName == "door")
    {
        parsingWalls=false;
        thickness=15;
        height=250;
        color=255;
        caption="";

        for(int i=0;i<at.length();i++){
            if(at.localName(i)=="thickness")
            {
                thickness=at.value(i).toDouble()*FAKTOR;
            }
            else if(at.localName(i)=="height")
            {
                height=at.value(i).toDouble()*FAKTOR;
            }
            else if(at.localName(i)=="color")
            {
                color=at.value(i).toDouble();
            }
            else if(at.localName(i)=="caption")
            {
                caption=at.value(i);
            }
        }

    }
    else if (qName == "timeFirstFrame")
    {
        unsigned long timeFirstFrame_us=0;
        unsigned long timeFirstFrame_s=0;

        for(int i=0;i<at.length();i++)
        {
            if(at.localName(i)=="microsec")
            {
                timeFirstFrame_us=at.value(i).toULong();
            }
            else if(at.localName(i)=="sec")
            {
                timeFirstFrame_s=at.value(i).toULong();
            }
        }
        dataset->setDelayAbsolute(timeFirstFrame_s,timeFirstFrame_us);
    }
    else if (qName == "point")
    {
        double xPos=0;
        double yPos=0;
        double zPos=0;

        for(int i=0;i<at.length();i++)
        {
            if(at.localName(i)=="xPos")
            {
                xPos=at.value(i).toDouble()*FAKTOR;
            }
            else if(at.localName(i)=="yPos")
            {
                yPos=at.value(i).toDouble()*FAKTOR;
            }
            else if(at.localName(i)=="zPos")
            {
                zPos=at.value(i).toDouble()*FAKTOR;
            }
        }
        double CHT[3]={color,height,thickness};

        JPoint* pt= new JPoint(xPos,yPos,zPos);
        pt->setColorHeightThicknes(CHT);
        currentPointsList.push_back(pt);

    }  else if (qName == "frame") {

    }  else if (qName == "agent") {

        int id=0;
        double xPos=0;
        double yPos=0;
        double zPos=0;
        double agent_color =std::numeric_limits<double>::quiet_NaN();
        double xVel=std::numeric_limits<double>::quiet_NaN();
        double yVel=std::numeric_limits<double>::quiet_NaN();
        double zVel=std::numeric_limits<double>::quiet_NaN();
        double dia_a=std::numeric_limits<double>::quiet_NaN();
        double dia_b=std::numeric_limits<double>::quiet_NaN();
        double el_angle=std::numeric_limits<double>::quiet_NaN();
        double el_color=std::numeric_limits<double>::quiet_NaN();
        double el_x=std::numeric_limits<double>::quiet_NaN();
        double el_y=std::numeric_limits<double>::quiet_NaN();
        double el_z=std::numeric_limits<double>::quiet_NaN();

        for(int i=0;i<at.length();i++)
        {
            if(at.localName(i)=="ID")
            {
                id=at.value(i).toInt();
                //TODO: maybe you should change ur format to take the ID 0 as first valid ID.
                if (id==0){
                    //slotErrorOutput("Person with ID=0 detected. ID should start with 1 !");
                    return false;
                }
            }
            else if(at.localName(i)==_jps_xPos)
            {
                xPos=at.value(i).toDouble()*FAKTOR;
            }
            else if(at.localName(i)==_jps_yPos)
            {
                yPos=at.value(i).toDouble()*FAKTOR;
            }
            else if(at.localName(i)==_jps_zPos)
            {
                zPos=at.value(i).toDouble()*FAKTOR;
            }

            else if(at.localName(i)==_jps_radiusA)
            {
                dia_a=at.value(i).toDouble()*FAKTOR;
            }
            else if(at.localName(i)==_jps_radiusB)
            {
                dia_b=at.value(i).toDouble()*FAKTOR;
            }
            else if(at.localName(i)==_jps_ellipseOrientation)
            {
                el_angle=at.value(i).toDouble();
            }
            else if(at.localName(i)==_jps_ellipseColor)
            {
                el_color=at.value(i).toDouble();
            }
            else if(at.localName(i)=="agentColor")
            {
                agent_color=at.value(i).toDouble();
            }
            else if(at.localName(i)==_jps_xVel)
            {
                xVel=at.value(i).toDouble()*FAKTOR;
            }
            else if(at.localName(i)==_jps_yVel)
            {
                yVel=at.value(i).toDouble()*FAKTOR;
            }
            else if(at.localName(i)==_jps_zVel)
            {
                zVel=at.value(i).toDouble()*FAKTOR;
            }

        }

        //coordinates of the ellipse, default to the head of the agent
        if(isnan(el_x)) el_x=xPos;
        if(isnan(el_y)) el_y=yPos;
        if(isnan(el_z)) el_z=zPos;

        double pos[3]={xPos,yPos,zPos};
        double vel[3]={xVel,yPos,zPos};
        double ellipse[7]={el_x,el_y,el_z,dia_a,dia_b,el_angle,el_color};
        double para[2]={agent_color,el_angle};

        TrajectoryPoint * point = new TrajectoryPoint(id-1);
        point->setEllipse(ellipse);
        point->setPos(pos);
        point->setVel(vel);
        point->setAgentInfo(para);
        currentFrame.push_back(point);
    }
    else if (qName == "agentInfo")
    {
        double height=std::numeric_limits<double>::quiet_NaN();
        int color=std::numeric_limits<int>::quiet_NaN();
        int id=std::numeric_limits<int>::quiet_NaN();

        for(int i=0;i<at.length();i++)
        {
            if(at.localName(i)=="ID")
            {
                id=at.value(i).toInt();
            }
            if(at.localName(i)=="height")
            {
                height=at.value(i).toDouble()*FAKTOR;
            }
            if(at.localName(i)=="color")
            {
                color=at.value(i).toDouble();
            }
        }
        if(isnan(id)) return true;

        if(!isnan(height)){
            initialPedestriansHeights.append(QString::number(id));
            initialPedestriansHeights.append(QString::number(height));
        }
        if(!isnan(color)){
            initialPedestriansColors.append(QString::number(id));
            initialPedestriansColors.append(QString::number(color));
        }
    }
    return true;
}

bool SaxParser::characters(const QString &str) {
	currentText.append(str);
	return true;
}

bool SaxParser::endElement(const QString & /* namespaceURI */,
		const QString & /* localName */, const QString &qName)
{
	if (qName == "header") {

	} else if (qName == "agents") {
		dataset->setNumberOfAgents(currentText.toInt());
	} else if (qName == "frameRate") {
		para[0]=currentText.toFloat();
	} else if (qName == "wall") {
		for(unsigned int i=0;i<currentPointsList.size()-1;i++){
			geometry->addWall(currentPointsList[i],currentPointsList[i+1],caption.toStdString());
		}
		clearPoints();
	} else if (qName == "door") {
		for(unsigned int i=0;i<currentPointsList.size()-1;i++){
			geometry->addDoor(currentPointsList[i],currentPointsList[i+1],caption.toStdString());
		}
		clearPoints();
	} else if (qName == "step") {//FIXME
		for(unsigned int i=0;i<currentPointsList.size()-1;i++){
			geometry->addDoor(currentPointsList[i],currentPointsList[i+1],caption.toStdString());
		}
		clearPoints();
	} else if (qName == "frame") {
		Frame* frame = new Frame();
		while(!currentFrame.empty()){
			frame->addElement(currentFrame.back());
			currentFrame.pop_back();
			//cout<<"not adding"<<endl;
		}

		dataset->addFrame(frame);
		//to be on the safe side
		currentFrame.clear();

	} else if (qName == "agent") {
	} else if (qName == "geometry") {
	} else if (qName == "point") {
	} else if (qName == "shape") {
		dataset->setInitialHeights(initialPedestriansHeights);
		dataset->setInitialColors(initialPedestriansColors);
	}
	currentText.clear();
	return true;
}

bool SaxParser::fatalError(const QXmlParseException &exception) {
	QMessageBox::warning(0, QObject::tr("SAX Handler"), QObject::tr(
			"Parse error at line %1, column "
			"%2:\n%3.") .arg(exception.lineNumber()) .arg(
					exception.columnNumber()) .arg(exception.message()));
	return false;
}

bool SaxParser::attributeDecl(const QString& eName, const QString& aName,
		const QString& type, const QString& valueDefault, const QString& value) {
	//cout<<aName.toStdString()<<endl;

	QString dummy=eName+aName+type+valueDefault+value;
	return (dummy==dummy);
	//return true;
}

void SaxParser::clearPoints(){

//	currentPointsList.clear();

	while (!currentPointsList.empty()){
		delete currentPointsList.back();
		currentPointsList.pop_back();
	}
	currentPointsList.clear();
	return;
}




/// provided for convenience and will be removed in the next version
void SaxParser::parseGeometryJPS(QString fileName, FacilityGeometry *geometry){

	if(!fileName.endsWith(".xml",Qt::CaseInsensitive)) return ;

	Building* building = new Building();
	string geometrypath = fileName.toStdString();

	// read the geometry
	building->LoadBuildingFromFile(geometrypath);
	building->InitGeometry(); // create the polygons

	int currentID=0;
	// Setup the points
	vtkSmartPointer<vtkPoints> points =
			vtkSmartPointer<vtkPoints>::New();

	// Add the polygon to a list of polygons
	vtkSmartPointer<vtkCellArray> polygons =
			vtkSmartPointer<vtkCellArray>::New();

	for (int i = 0; i < building->GetNumberOfRooms(); i++) {
		Room* r = building->GetRoom(i);
        //string caption = r->GetCaption();

		for (int k = 0; k < r->GetNumberOfSubRooms(); k++) {
			SubRoom* sub = r->GetSubRoom(k);
            vector<Point> poly = sub->GetPolygon();

            if(sub->IsClockwise()==true){
                std::reverse(poly.begin(),poly.end());
            }

			// Create the polygon
			vtkSmartPointer<vtkPolygon> polygon =
					vtkSmartPointer<vtkPolygon>::New();
			polygon->GetPointIds()->SetNumberOfIds(poly.size());

			for (unsigned int s=0;s<poly.size();s++){
				points->InsertNextPoint(poly[s]._x*FAKTOR,poly[s]._y*FAKTOR,sub->GetElevation(poly[s])*FAKTOR);
				polygon->GetPointIds()->SetId(s, currentID++);
			}

			polygons->InsertNextCell(polygon);
		}
	}

	// Create a PolyData
	vtkSmartPointer<vtkPolyData> polygonPolyData =
			vtkSmartPointer<vtkPolyData>::New();
	polygonPolyData->SetPoints(points);
	polygonPolyData->SetPolys(polygons);

    //triagulate everything
    vtkSmartPointer<vtkTriangleFilter> filter=vtkSmartPointer<vtkTriangleFilter>::New();

    // Create a mapper and actor
    vtkSmartPointer<vtkPolyDataMapper> mapper =
            vtkSmartPointer<vtkPolyDataMapper>::New();



#if VTK_MAJOR_VERSION <= 5
     filter->SetInput(polygonPolyData);
    mapper->SetInput(filter->GetOutput());
#else
    filter->SetInputData(polygonPolyData);
    mapper->SetInputConnection(filter->GetOutputPort());
#endif

	vtkSmartPointer<vtkActor> actor =
			vtkSmartPointer<vtkActor>::New();
	actor->SetMapper(mapper);
	actor->GetProperty()->SetColor(0,0,1);
	actor->GetProperty()->SetOpacity(0.5);
	//actor->GetProperty()->SetLineWidth(5);

	geometry->getActor()->AddPart(actor);

	// free memory
	delete building;
}


/// provided for convenience and will be removed in the next version
void SaxParser::parseGeometryTRAV(QString content, FacilityGeometry *geometry,QDomNode geo){

	cout<<"external geometry found"<<endl;
	//creating am empty document
	// to be filled
	QDomDocument doc("");
	QDomNode geoNode;

	//first try to open the file
	if(content.endsWith(".trav",Qt::CaseInsensitive) )
	{
		QFile file(content);
		if (!file.open(QIODevice::ReadOnly)) {
			//slotErrorOutput("could not open the File" );
			cout<<"could not open the File"<<endl;
			return ;
		}
		QString *errorCode = new QString();
		if (!doc.setContent(&file, errorCode)) {
			file.close();
			//slotErrorOutput(*errorCode);
			cout<<errorCode->toStdString()<<endl;
			return ;
		}
		file.close();
		geoNode =doc.documentElement().namedItem("geometry");

		if (geoNode.isNull()){
			cout<<"No geometry information found. <geometry> <geometry/> tag is missing."<<endl;
		}
	}
	else
	{
		if(content.isEmpty()){
			geoNode=geo;
			cout <<"parsing the old fashion way"<<endl;
		}else{
			content = "<travisto>\n" +content+ "\n</travisto>\n";
			QString errorMsg="";
			doc.setContent(content,&errorMsg);

			if(!errorMsg.isEmpty()){
				Debug::Error("%s", (const char *)errorMsg.toStdString().c_str());
				return;
			}
			geoNode =doc.elementsByTagName("geometry").item(0);
		}
	}

	// for the case there is more than just one geometry Node
	while (!geoNode.isNull()) {
		QDomElement e = geoNode.toElement();
		QDomNodeList walls = e.elementsByTagName("wall");
		QDomNodeList doors = e.elementsByTagName("door");

		//objects which can be positioned everywhere in the facility
		QDomNodeList spheres = e.elementsByTagName("sphere");
		QDomNodeList cuboids = e.elementsByTagName("cuboid");
		QDomNodeList floors = e.elementsByTagName("floor");
		QDomNodeList cylinders = e.elementsByTagName("cylinder");
		QDomNodeList labels = e.elementsByTagName("label");


		//parsing the walls
		for (unsigned int i = 0; i < walls.length(); i++) {
			QDomElement el = walls.item(i).toElement();

			//wall thickness, default to 30 cm
			double thickness = el.attribute("thickness","15").toDouble();
			//wall height default to 250 cm
			double height = el.attribute("height","250").toDouble();
			//wall color default to blue
			double color = el.attribute("color","0").toDouble();

			//get the points defining each wall
			//not that a wall is not necessarily defined by two points, could be more...
			QDomNodeList points = el.elementsByTagName("point");
			for (unsigned int i = 0; i < points.length() - 1; i++) {

				double x1=points.item(i).toElement().attribute("xPos", "0").toDouble();
				double y1=points.item(i).toElement().attribute("yPos", "0").toDouble();
				double z1=points.item(i).toElement().attribute("zPos", "0").toDouble();

				double x2=points.item(i+1).toElement().attribute("xPos", "0").toDouble();
				double y2=points.item(i+1).toElement().attribute("yPos", "0").toDouble();
				double z2=points.item(i+1).toElement().attribute("zPos", "0").toDouble();
				geometry->addWall(x1, y1,z1 ,x2, y2,z2,thickness,height,color);
			}
		}

		//parsing the doors
		if(doors.length()>0)
			for (unsigned int i = 0; i < doors.length(); i++) {
				QDomElement el = doors.item(i).toElement();

				//door thickness, default to 15 cm
				double thickness = el.attribute("thickness","15").toDouble();
				//door height default to 250 cm
				double height = el.attribute("height","250").toDouble();
				//door color default to blue
				double color = el.attribute("color","255").toDouble();

				//get the points defining each wall
				//not that a wall is not necesarily defined by two points, could be more...
				QDomNodeList points = el.elementsByTagName("point");
				//Debug::Messages("found:  " << points.length() <<" for this wall" <<endl;
				for (unsigned int i = 0; i < points.length() - 1; i++) {

					double x1=points.item(i).toElement().attribute("xPos", "0").toDouble();
					double y1=points.item(i).toElement().attribute("yPos", "0").toDouble();
					double z1=points.item(i).toElement().attribute("zPos", "0").toDouble();

					double x2=points.item(i+1).toElement().attribute("xPos", "0").toDouble();
					double y2=points.item(i+1).toElement().attribute("yPos", "0").toDouble();
					double z2=points.item(i+1).toElement().attribute("zPos", "0").toDouble();
					geometry->addDoor(x1, y1, z1, x2, y2,z2,thickness,height,color);
				}
			}

		// parsing the objets
		for (unsigned int i = 0; i < spheres.length(); i++) {

			double center[3];
			center[0] = spheres.item(i).toElement().attribute("centerX", "0").toDouble();
			center[1]= spheres.item(i).toElement().attribute("centerY", "0").toDouble();
			center[2]= spheres.item(i).toElement().attribute("centerZ", "0").toDouble();
			double color= spheres.item(i).toElement().attribute("color", "0").toDouble();
			double radius= spheres.item(i).toElement().attribute("radius", "0").toDouble();
			//double width = spheres.item(i).toElement().attribute("width", "0").toDouble();
			//double height= spheres.item(i).toElement().attribute("height", "0").toDouble();

			geometry->addObjectSphere(center,radius,color);
		}
		// cubic shapes
		for (unsigned int i = 0; i < cuboids.length(); i++) {

			double center[3];
			center[0] = cuboids.item(i).toElement().attribute("centerX", "0").toDouble();
			center[1]= cuboids.item(i).toElement().attribute("centerY", "0").toDouble();
			center[2]= cuboids.item(i).toElement().attribute("centerZ", "0").toDouble();
			double color= cuboids.item(i).toElement().attribute("color", "0").toDouble();
			double length= cuboids.item(i).toElement().attribute("length", "0").toDouble();
			double width = cuboids.item(i).toElement().attribute("width", "0").toDouble();
			double height= cuboids.item(i).toElement().attribute("height", "0").toDouble();
			geometry->addObjectBox(center,height,width,length,color);
			//		Debug::Error("cuboids: "<<length<<" || " <<width << " || "<<height<<" || "<<color<<endl;
		}
		// floors
        for (  int i = 0; i < floors.length(); i++) {

			double left =floors.item(i).toElement().attribute("xMin","0").toDouble();
			double right =floors.item(i).toElement().attribute("xMax","0").toDouble();
			double up =floors.item(i).toElement().attribute("yMax","0").toDouble();
			double down =floors.item(i).toElement().attribute("yMin","0").toDouble();
			double z =floors.item(i).toElement().attribute("z","0").toDouble();
			geometry->addFloor(left,down,right,up,z);
		}
		// cylinders
        for (  int i = 0; i < cylinders.length(); i++) {

			double center[3], rotation[3];
			center[0] = cylinders.item(i).toElement().attribute("centerX", "0").toDouble();
			center[1]= cylinders.item(i).toElement().attribute("centerY", "0").toDouble();
			center[2]= cylinders.item(i).toElement().attribute("centerZ", "0").toDouble();
			double color= cylinders.item(i).toElement().attribute("color", "0").toDouble();
			double radius= cylinders.item(i).toElement().attribute("radius", "0").toDouble();
			double height= cylinders.item(i).toElement().attribute("height", "0").toDouble();
			rotation[0] = cylinders.item(i).toElement().attribute("angleX", "90").toDouble();
			rotation[1] = cylinders.item(i).toElement().attribute("angleY", "0").toDouble();
			rotation[2] = cylinders.item(i).toElement().attribute("angleZ", "0").toDouble();
			geometry->addObjectCylinder(center,radius,height,rotation,color);
		}

		//Tlabels
        for (  int i = 0; i < labels.length(); i++) {

			double center[3];
			center[0] = labels.item(i).toElement().attribute("centerX", "0").toDouble();
			center[1]= labels.item(i).toElement().attribute("centerY", "0").toDouble();
			center[2]= labels.item(i).toElement().attribute("centerZ", "0").toDouble();
			double color= labels.item(i).toElement().attribute("color", "0").toDouble();
			string caption= labels.item(i).toElement().attribute("text", "").toStdString();
			geometry->addObjectLabel(center,center,caption,color);
		}
		// you should normally have only one geometry node, but one never knows...
		geoNode = geoNode.nextSiblingElement("geometry");
	}
}

void SaxParser::parseGeometryXMLV04(QString filename, FacilityGeometry *geo){
	QDomDocument doc("");

	QFile file(filename);
    //int size =file.size()/(1024*1024);

    //if(size>100){
    //	cout<<"The file is too large: "<<filename.toStdString()<<endl;
    //	return;
    //}

	if (!file.open(QIODevice::ReadOnly)) {
		qDebug()<<"could not open the file: "<<filename<<endl;
		return ;
	}
	QString *errorCode = new QString();
	if (!doc.setContent(&file, errorCode)) {
		file.close();
		qDebug()<<errorCode<<endl;
		return ;
	}
	QDomElement root= doc.documentElement();

	//only parsing the geometry node
	if(root.tagName()!="geometry") return;


	double version =root.attribute("version","-1").toDouble();

	string unit=root.attribute("unit","cm").toStdString();
	double xToCmfactor=100;
	if (unit=="cm") xToCmfactor=1;
	if (unit=="m") xToCmfactor=100;

	if(version<0.4){
		QMessageBox::warning(0, QObject::tr("Parsing Error"),
				QObject::tr("Only geometry version >= 0.4 supported"));
	}

	//parsing the subrooms
	QDomNodeList xSubRoomsNodeList=doc.elementsByTagName("subroom");
	//parsing the walls
	for (unsigned int i = 0; i < xSubRoomsNodeList.length(); i++) {
		QDomElement xPoly = xSubRoomsNodeList.item(i).firstChildElement("polygon");
		double position[3]={0,0,0};
		double pos_count=1;
		double color=0;

		while(!xPoly.isNull()) {
			//wall thickness, default to 30 cm
			double thickness = xPoly.attribute("thickness","15").toDouble();
			//wall height default to 250 cm
			double height = xPoly.attribute("height","250").toDouble();
			//wall color default to blue
			 color = xPoly.attribute("color","0").toDouble();

			QDomNodeList xVertices=xPoly.elementsByTagName("vertex");
			pos_count+=xVertices.count()-1;

			for( int i=0;i<xVertices.count()-1;i++){
				//all unit are converted in cm
				double x1=xVertices.item(i).toElement().attribute("px", "0").toDouble()*xToCmfactor;
				double y1=xVertices.item(i).toElement().attribute("py", "0").toDouble()*xToCmfactor;
				double z1=xVertices.item(i).toElement().attribute("pz", "0").toDouble()*xToCmfactor;
				double x2=xVertices.item(i+1).toElement().attribute("px", "0").toDouble()*xToCmfactor;
				double y2=xVertices.item(i+1).toElement().attribute("py", "0").toDouble()*xToCmfactor;
				double z2=xVertices.item(i+1).toElement().attribute("pz", "0").toDouble()*xToCmfactor;

				position[0]+= x1;
				position[1]+= y1;
				position[2]+= z1;

				geo->addWall(x1, y1, z1, x2, y2,z2,thickness,height,color);
			}
			xPoly = xPoly.nextSiblingElement("polygon");
		}

		//add the caption
		string roomCaption = xSubRoomsNodeList.item(i).parentNode().toElement().attribute("caption").toStdString();
		string subroomCaption=xSubRoomsNodeList.item(i).toElement().attribute("id").toStdString();
		string caption=roomCaption+" ( " + subroomCaption + " ) ";
		position[0]/=pos_count;
		position[1]/=pos_count;
		position[2]/=pos_count;
		geo->addObjectLabel(position,position,caption,color);
		//cout<<"position: [" <<position[0]<<", "<<position[1]<<", "<<position[2]<<" ]"<<endl;;

	}

	QDomNodeList xObstaclesList=doc.elementsByTagName("obstacle");
    for (  int i = 0; i < xObstaclesList.length(); i++) {
		QDomElement xPoly = xObstaclesList.item(i).firstChildElement("polygon");
		while(!xPoly.isNull()) {
			//wall thickness, default to 30 cm
			double thickness = xPoly.attribute("thickness","15").toDouble();
			//wall height default to 250 cm
			double height = xPoly.attribute("height","250").toDouble();
			//wall color default to blue
			double color = xPoly.attribute("color","0").toDouble();

			QDomNodeList xVertices=xPoly.elementsByTagName("vertex");
			for( int i=0;i<xVertices.count()-1;i++){
				double x1=xVertices.item(i).toElement().attribute("px", "0").toDouble()*xToCmfactor;
				double y1=xVertices.item(i).toElement().attribute("py", "0").toDouble()*xToCmfactor;
				double z1=xVertices.item(i).toElement().attribute("pz", "0").toDouble()*xToCmfactor;

				double x2=xVertices.item(i+1).toElement().attribute("px", "0").toDouble()*xToCmfactor;
				double y2=xVertices.item(i+1).toElement().attribute("py", "0").toDouble()*xToCmfactor;
				double z2=xVertices.item(i+1).toElement().attribute("pz", "0").toDouble()*xToCmfactor;
				geo->addWall(x1, y1, z1, x2, y2,z2,thickness,height,color);
			}
			xPoly = xPoly.nextSiblingElement("polygon");
		}
	}

	QDomNodeList xCrossingsList=doc.elementsByTagName("crossing");
    for (int i = 0; i < xCrossingsList.length(); i++) {
		QDomElement xCrossing = xCrossingsList.item(i).toElement();
		QDomNodeList xVertices=xCrossing.elementsByTagName("vertex");

		///door thickness, default to 15 cm
		double thickness = xCrossing.attribute("thickness","15").toDouble();
		//door height default to 250 cm
		double height = xCrossing.attribute("height","250").toDouble();
		//door color default to blue
		double color = xCrossing.attribute("color","255").toDouble();
		QString id= xCrossing.attribute("id","-1");

		double x1=xVertices.item(0).toElement().attribute("px", "0").toDouble()*xToCmfactor;
		double y1=xVertices.item(0).toElement().attribute("py", "0").toDouble()*xToCmfactor;
		double z1=xVertices.item(0).toElement().attribute("pz", "0").toDouble()*xToCmfactor;

		double x2=xVertices.item(1).toElement().attribute("px", "0").toDouble()*xToCmfactor;
		double y2=xVertices.item(1).toElement().attribute("py", "0").toDouble()*xToCmfactor;
		double z2=xVertices.item(1).toElement().attribute("pz", "0").toDouble()*xToCmfactor;
		geo->addDoor(x1, y1, z1, x2, y2,z2,thickness,height,color);

		double center[3]={(x1+x2)/2.0, (y1+y2)/2.0, (z2+z1)/2.0};
		geo->addObjectLabel(center,center,id.toStdString(),21);
	}

	QDomNodeList xTransitionsList=doc.elementsByTagName("transition");
    for (  int i = 0; i < xTransitionsList.length(); i++) {
		QDomElement xTransition = xTransitionsList.item(i).toElement();
		QDomNodeList xVertices=xTransition.elementsByTagName("vertex");

		///door thickness, default to 15 cm
		double thickness = xTransition.attribute("thickness","15").toDouble();
		//door height default to 250 cm
		double height = xTransition.attribute("height","250").toDouble();
		//door color default to blue
		double color = xTransition.attribute("color","255").toDouble();

		double x1=xVertices.item(0).toElement().attribute("px", "0").toDouble()*xToCmfactor;
		double y1=xVertices.item(0).toElement().attribute("py", "0").toDouble()*xToCmfactor;
		double z1=xVertices.item(0).toElement().attribute("pz", "0").toDouble()*xToCmfactor;

		double x2=xVertices.item(1).toElement().attribute("px", "0").toDouble()*xToCmfactor;
		double y2=xVertices.item(1).toElement().attribute("py", "0").toDouble()*xToCmfactor;
		double z2=xVertices.item(1).toElement().attribute("pz", "0").toDouble()*xToCmfactor;
		geo->addDoor(x1, y1, z1, x2, y2,z2,thickness,height,color);

		string id= xTransition.attribute("id","-1").toStdString();
		double center[3]={(x1+x2)/2.0, (y1+y2)/2.0, (z2+z1)/2.0};
		geo->addObjectLabel(center,center,id,21);
	}
}

void SaxParser::InitHeader(double version)
{
    // set the parsing String map
    if(version==0.5){
        _jps_xPos=QString("xPos");
        _jps_yPos=QString("yPos");
        _jps_zPos=QString("zPos");
        _jps_xVel=QString("xVel");
        _jps_yVel=QString("yVel");
        _jps_zVel=QString("zVel");
        _jps_radiusA=QString("radiusA");
        _jps_radiusB=QString("radiusB");
        _jps_ellipseOrientation=QString("ellipseOrientation");
        _jps_ellipseColor=QString("ellipseColor");
    }
    else if (version==0.6){
        _jps_xPos=QString("x");
        _jps_yPos=QString("y");
        _jps_zPos=QString("z");
        _jps_xVel=QString("xV");
        _jps_yVel=QString("yV");
        _jps_zVel=QString("zV");
        _jps_radiusA=QString("rA");
        _jps_radiusB=QString("rB");
        _jps_ellipseOrientation=QString("eO");
        _jps_ellipseColor=QString("eC");
    }
    else
    {
        cout<<"unsupported header version: "<<version<<endl;
        cout<<"Please use 0.5 or 0.6 "<<endl;
        exit(0);
    }
}
