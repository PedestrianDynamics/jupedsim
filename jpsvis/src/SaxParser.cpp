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
#include "FrameElement.h"
#include "Frame.h"
#include "SyncData.h"
#include "Debug.h"

#include "geometry/JPoint.h"
#include "geometry/FacilityGeometry.h"
#include "geometry/Building.h"
#include "geometry/Wall.h"
#include "geometry/SubRoom.h"

#include "SystemSettings.h"

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

#define VTK_CREATE(type, name) \
    vtkSmartPointer<type> name = vtkSmartPointer<type>::New()


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
SaxParser::SaxParser(FacilityGeometry* geo, SyncData* data, double* fps)
{
    geometry=geo;
    dataset=data;
    para=fps;
    parsingWalls=false;
    parsingCrossings=false;
    color=0.0;
    dataset->clearFrames();
    //default header
    InitHeader(0,0,0);
}

SaxParser::~SaxParser()
{

}

bool SaxParser::startElement(const QString & /* namespaceURI */,
                             const QString & /* localName */, const QString &qName,
                             const QXmlAttributes &at)
{
    if (qName == "header") {
        for(int i=0; i<at.length(); i++) {
            if(at.localName(i)=="version") {
                QStringList query = at.value(i).split(".");
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
        }
    } else if (qName == "file") {
        for(int i=0; i<at.length(); i++) {
            if(at.localName(i)=="location") {
                QString fileName=at.value(i);
                if(!fileName.isEmpty()) {
                    if(fileName.endsWith(".xml",Qt::CaseInsensitive)) {
                        //SaxParser::parseGeometryJPS(fileName,geometry);
                    } else if (fileName.endsWith(".trav",Qt::CaseInsensitive)) {
                        SaxParser::parseGeometryTRAV(fileName,geometry);
                    }
                }
            }
        }
    } else if (qName == "floor") {
        double xMin=0,
               xMax=0,
               yMin=0,
               yMax=0;

        for(int i=0; i<at.length(); i++) {
            if(at.localName(i)=="xMin") {
                xMin=at.value(i).toDouble()*FAKTOR;
            } else if(at.localName(i)=="xMax") {
                xMax=at.value(i).toDouble()*FAKTOR;
            } else if(at.localName(i)=="yMin") {
                yMin=at.value(i).toDouble()*FAKTOR;
            } else if(at.localName(i)=="yMax") {
                yMax=at.value(i).toDouble()*FAKTOR;
            }

        }
        geometry->addFloor(xMin,yMin,xMax,yMax);
    } else if (qName == "cuboid") {
        double length=0, height=0,
               width=0, color=0;
        double center[3]= {0,0,0};

        for(int i=0; i<at.length(); i++) {
            if(at.localName(i)=="centerX") {
                center[0]=at.value(i).toDouble()*FAKTOR;
            } else if(at.localName(i)=="centerY") {
                center[1]=at.value(i).toDouble()*FAKTOR;
            } else if(at.localName(i)=="centerZ") {
                center[2]=at.value(i).toDouble()*FAKTOR;
            } else if(at.localName(i)=="length") {
                length=at.value(i).toDouble()*FAKTOR;
            } else if(at.localName(i)=="height") {
                height=at.value(i).toDouble()*FAKTOR;
            } else if(at.localName(i)=="width") {
                width=at.value(i).toDouble()*FAKTOR;
            } else if(at.localName(i)=="color") {
                color=at.value(i).toDouble()*FAKTOR;
            }
        }
        geometry->addObjectBox(center,height,width,length,color);

    } else if (qName == "sphere") {
        double radius=0, color=0;
        double center[3]= {0,0,0};

        for(int i=0; i<at.length(); i++) {
            if(at.localName(i)=="centerX") {
                center[0]=at.value(i).toDouble()*FAKTOR;
            } else if(at.localName(i)=="centerY") {
                center[1]=at.value(i).toDouble()*FAKTOR;
            } else if(at.localName(i)=="centerZ") {
                center[2]=at.value(i).toDouble()*FAKTOR;
            } else if(at.localName(i)=="radius") {
                radius=at.value(i).toDouble()*FAKTOR;
            } else if(at.localName(i)=="color") {
                color=at.value(i).toDouble();
            }
        }
        geometry->addObjectSphere(center,radius,color);
    } else if (qName == "label") {
        double  color=0;
        double center[3]= {0,0,0};
        QString text;

        for(int i=0; i<at.length(); i++) {
            if(at.localName(i)=="centerX") {
                center[0]=at.value(i).toDouble()*FAKTOR;
            } else if(at.localName(i)=="centerY") {
                center[1]=at.value(i).toDouble()*FAKTOR;
            } else if(at.localName(i)=="centerZ") {
                center[2]=at.value(i).toDouble()*FAKTOR;
            } else if(at.localName(i)=="text") {
                text=at.value(i);
            } else if(at.localName(i)=="color") {
                color=at.value(i).toDouble();
            }
        }
        geometry->addObjectLabel(center,center,text.toStdString(),color);
    } else if (qName == "cylinder") {
        double height=0, radius=0, color=0;
        double center[3]= {0,0,0};
        double rotation[3]= {0,0,0};

        for(int i=0; i<at.length(); i++) {
            if(at.localName(i)=="centerX") {
                center[0]=at.value(i).toDouble()*FAKTOR;
            } else if(at.localName(i)=="centerY") {
                center[1]=at.value(i).toDouble()*FAKTOR;
            } else if(at.localName(i)=="centerZ") {
                center[2]=at.value(i).toDouble()*FAKTOR;
            } else if(at.localName(i)=="height") {
                height=at.value(i).toDouble()*FAKTOR;
            } else if(at.localName(i)=="radius") {
                radius=at.value(i).toDouble()*FAKTOR;
            } else if(at.localName(i)=="color") {
                color=at.value(i).toDouble();
            } else if(at.localName(i)=="angleX") {
                rotation[0]=at.value(i).toDouble();
            } else if(at.localName(i)=="angleY") {
                rotation[1]=at.value(i).toDouble();
            } else if(at.localName(i)=="angleZ") {
                rotation[2]=at.value(i).toDouble();
            }
        }
        geometry->addObjectCylinder(center,radius,height,rotation,color);
    } else if (qName == "agents") {
    } else if (qName == "roomCaption") {
    } else if (qName == "frameRate") {
    } else if (qName == "geometry") {
    } else if (qName == "wall") {
        parsingWalls=true;
        thickness=15;
        height=250;
        color=0;
        caption="";

        for(int i=0; i<at.length(); i++) {
            if(at.localName(i)=="thickness") {
                thickness=at.value(i).toDouble()*FAKTOR;
            } else if(at.localName(i)=="height") {
                height=at.value(i).toDouble()*FAKTOR;
            } else if(at.localName(i)=="color") {
                color=at.value(i).toDouble();
            } else if(at.localName(i)=="caption") {
                caption=at.value(i);
            }
        }

    } else if (qName == "door") {
        parsingWalls=false;
        thickness=15;
        height=250;
        color=255;
        caption="";

        for(int i=0; i<at.length(); i++) {
            if(at.localName(i)=="thickness") {
                thickness=at.value(i).toDouble()*FAKTOR;
            } else if(at.localName(i)=="height") {
                height=at.value(i).toDouble()*FAKTOR;
            } else if(at.localName(i)=="color") {
                color=at.value(i).toDouble();
            } else if(at.localName(i)=="caption") {
                caption=at.value(i);
            }
        }

    }
    //FIXME
    else if (qName == "crossing") {
        parsingWalls=false;
        parsingCrossings=true;
        thickness=15;
        height=250;
        color=255;
        caption="";

        for(int i=0; i<at.length(); i++) {
            if(at.localName(i)=="thickness") {
                thickness=at.value(i).toDouble()*FAKTOR;
            } else if(at.localName(i)=="height") {
                height=at.value(i).toDouble()*FAKTOR;
            } else if(at.localName(i)=="color") {
                color=at.value(i).toDouble();
            } else if(at.localName(i)=="caption") {
                caption=at.value(i);
            }
        }

    }else if (qName == "hline") {
        parsingWalls=false;
        parsingCrossings=true;
        thickness=15;
        height=250;
        color=255;
        caption="";

        for(int i=0; i<at.length(); i++) {
            if(at.localName(i)=="thickness") {
                thickness=at.value(i).toDouble()*FAKTOR;
            } else if(at.localName(i)=="height") {
                height=at.value(i).toDouble()*FAKTOR;
            } else if(at.localName(i)=="color") {
                color=at.value(i).toDouble();
            } else if(at.localName(i)=="caption") {
                caption=at.value(i);
            }
        }

    }
    else if (qName == "timeFirstFrame") {
        unsigned long timeFirstFrame_us=0;
        unsigned long timeFirstFrame_s=0;

        for(int i=0; i<at.length(); i++) {
            if(at.localName(i)=="microsec") {
                timeFirstFrame_us=at.value(i).toULong();
            } else if(at.localName(i)=="sec") {
                timeFirstFrame_s=at.value(i).toULong();
            }
        }
        dataset->setDelayAbsolute(timeFirstFrame_s,timeFirstFrame_us);
    } else if (qName == "point") {
        double xPos=0;
        double yPos=0;
        double zPos=0;

        for(int i=0; i<at.length(); i++) {
            if(at.localName(i)=="xPos") {
                xPos=at.value(i).toDouble()*FAKTOR;
            } else if(at.localName(i)=="yPos") {
                yPos=at.value(i).toDouble()*FAKTOR;
            } else if(at.localName(i)=="zPos") {
                zPos=at.value(i).toDouble()*FAKTOR;
            }
        }
        double CHT[3]= {color,height,thickness};

        JPoint* pt= new JPoint(xPos,yPos,zPos);
        pt->setColorHeightThicknes(CHT);
        currentPointsList.push_back(pt);

    }  else if (qName == "frame") {

    }  else if (qName == "agent") {

        int id=0;
        double xPos=0;
        double yPos=0;
        double zPos=0;
        //double agent_color =std::numeric_limits<double>::quiet_NaN();
        //double xVel=std::numeric_limits<double>::quiet_NaN();
        //double yVel=std::numeric_limits<double>::quiet_NaN();
        //double zVel=std::numeric_limits<double>::quiet_NaN();
        double dia_a=std::numeric_limits<double>::quiet_NaN();
        double dia_b=std::numeric_limits<double>::quiet_NaN();
        double el_angle=std::numeric_limits<double>::quiet_NaN();
        double el_color=std::numeric_limits<double>::quiet_NaN();
        double el_x=std::numeric_limits<double>::quiet_NaN();
        double el_y=std::numeric_limits<double>::quiet_NaN();
        double el_z=std::numeric_limits<double>::quiet_NaN();

        for(int i=0; i<at.length(); i++) {
            if(at.localName(i)=="ID") {
                id=at.value(i).toInt();
                //TODO: maybe you should change ur format to take the ID 0 as first valid ID.
                if (id==0) {
                    //slotErrorOutput("Person with ID=0 detected. ID should start with 1 !");
                    return false;
                }
            } else if(at.localName(i)==_jps_xPos) {
                xPos=at.value(i).toDouble()*FAKTOR;
                //xPos=at.value(i).toDouble();
            } else if(at.localName(i)==_jps_yPos) {
                //yPos=at.value(i).toDouble();
                yPos=at.value(i).toDouble()*FAKTOR;
            } else if(at.localName(i)==_jps_zPos) {
                zPos=at.value(i).toDouble()*FAKTOR;
            }

            else if(at.localName(i)==_jps_radiusA) {
                dia_a=at.value(i).toDouble()*FAKTOR;
                //dia_a=at.value(i).toDouble();
            } else if(at.localName(i)==_jps_radiusB) {
                dia_b=at.value(i).toDouble()*FAKTOR;
                //dia_b=at.value(i).toDouble();
            } else if(at.localName(i)==_jps_ellipseOrientation) {
                el_angle=at.value(i).toDouble();
            } else if(at.localName(i)==_jps_ellipseColor) {
                el_color=at.value(i).toDouble();
            } else if(at.localName(i)=="agentColor") {
                //agent_color=at.value(i).toDouble();
            } else if(at.localName(i)==_jps_xVel) {
                //xVel=at.value(i).toDouble()*FAKTOR;
            } else if(at.localName(i)==_jps_yVel) {
                //yVel=at.value(i).toDouble()*FAKTOR;
            } else if(at.localName(i)==_jps_zVel) {
                //zVel=at.value(i).toDouble()*FAKTOR;
            }

        }

        //coordinates of the ellipse, default to the head of the agent
        if(std::isnan(el_x)) el_x=xPos;
        if(std::isnan(el_y)) el_y=yPos;
        if(std::isnan(el_z)) el_z=zPos;

        //double pos[3]={xPos,yPos,zPos};
        //double vel[3]={xVel,yPos,zPos};
        //double ellipse[7]={el_x,el_y,el_z,dia_a,dia_b,el_angle,el_color};
        //double para[2]={agent_color,el_angle};

        double pos[3]= {xPos,yPos,zPos};
        double angle[3]= {0,0,el_angle};
        double radius[3]= {dia_a,dia_b,30.0};

        FrameElement *element = new FrameElement(id-1);
        element->SetPos(pos);
        element->SetOrientation(angle);
        element->SetRadius(radius);
        element->SetColor(el_color);
        currentFrame.push_back(element);

    } else if (qName == "agentInfo") {
        double height=std::numeric_limits<double>::quiet_NaN();
        int color=std::numeric_limits<int>::quiet_NaN();
        int id=std::numeric_limits<int>::quiet_NaN();

        for(int i=0; i<at.length(); i++) {
            if(at.localName(i)=="ID") {
                id=at.value(i).toInt();
            }
            if(at.localName(i)=="height") {
                height=at.value(i).toDouble()*FAKTOR;
            }
            if(at.localName(i)=="color") {
                color=at.value(i).toDouble();
            }
        }
        if(std::isnan(id)) return true;

        if(!std::isnan(height)) {
            initialPedestriansHeights.append(QString::number(id));
            initialPedestriansHeights.append(QString::number(height));
        }
        if(!std::isnan(color)) {
            initialPedestriansColors.append(QString::number(id));
            initialPedestriansColors.append(QString::number(color));
        }
    }
    return true;
}

bool SaxParser::characters(const QString &str)
{
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
        if(currentPointsList.size()>1)
            for(unsigned int i=0; i<currentPointsList.size()-1; i++) {
                geometry->addWall(currentPointsList[i],currentPointsList[i+1],caption.toStdString());
            }
        clearPoints();
    } else if (qName == "door") {
        for(unsigned int i=0; i<currentPointsList.size()-1; i++) {
            geometry->addDoor(currentPointsList[i],currentPointsList[i+1],caption.toStdString());
        }
        clearPoints();
    } else if (qName == "crossing") {
        if(currentPointsList.size()>1) //hack
            for(unsigned int i=0; i<currentPointsList.size()-1; i++) {
                geometry->addNavLine(currentPointsList[i],currentPointsList[i+1],caption.toStdString());
            }
        clearPoints();
    } else if (qName == "hline") {
        if(currentPointsList.size()>1) //hack
            for(unsigned int i=0; i<currentPointsList.size()-1; i++) {
                geometry->addNavLine(currentPointsList[i],currentPointsList[i+1],caption.toStdString());
            }
        clearPoints();
    } else if (qName == "step") {//FIXME
        for(unsigned int i=0; i<currentPointsList.size()-1; i++) {
            geometry->addDoor(currentPointsList[i],currentPointsList[i+1],caption.toStdString());
        }
        clearPoints();
    } else if (qName == "frame") {
        Frame* frame = new Frame();
        while(!currentFrame.empty()) {
            frame->addElement(currentFrame.back());
            currentFrame.pop_back();
            //cout<<"not adding"<<endl;
        }

        //compute the polydata, might increase the runtime
        frame->ComputePolyData();

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

bool SaxParser::fatalError(const QXmlParseException &exception)
{
    QMessageBox::warning(0, QObject::tr("SAX Handler"), QObject::tr(
                             "Parse error at line %1, column "
                             "%2:\n%3.") .arg(exception.lineNumber()) .arg(
                             exception.columnNumber()) .arg(exception.message()));
    return false;
}

bool SaxParser::attributeDecl(const QString& eName, const QString& aName,
                              const QString& type, const QString& valueDefault, const QString& value)
{
    //cout<<aName.toStdString()<<endl;

    QString dummy=eName+aName+type+valueDefault+value;
    return (dummy==dummy);
    //return true;
}

void SaxParser::clearPoints()
{
    while (!currentPointsList.empty()) {
        delete currentPointsList.back();
        currentPointsList.pop_back();
    }
    currentPointsList.clear();
    return;
}




/// provided for convenience and will be removed in the next version
bool SaxParser::parseGeometryJPS(QString fileName, FacilityGeometry *geometry)
{
    double captionsColor=0;//red
    if(!fileName.endsWith(".xml",Qt::CaseInsensitive)) return false;
    QString wd;
    SystemSettings::getWorkingDirectory(wd);
    fileName=wd+"/"+fileName;

    Building* building = new Building();
    string geometrypath = fileName.toStdString();

    // read the geometry
    if(!building->LoadGeometry(geometrypath))
        return false;
    if(!building->InitGeometry())
        return false; // create the polygons

    int currentFloorPolyID=0;
    int currentObstPolyID=0;

    // Setup the points
    VTK_CREATE(vtkPoints,floor_points);
    VTK_CREATE(vtkPoints,obstacles_points);
    // Add the polygon to a list of polygons
    VTK_CREATE(vtkCellArray,floor_polygons);
    VTK_CREATE(vtkCellArray,obstacles_polygons);

    for(auto&& itr_room: building->GetAllRooms())
    {
        for(auto&& itr_subroom: itr_room.second->GetAllSubRooms())
        {

            //string caption = r->GetCaption();
            SubRoom* sub = itr_subroom.second.get();

            vector<Point> poly = sub->GetPolygon();
            if(sub->IsClockwise()==true) {
                std::reverse(poly.begin(),poly.end());
            }

            // Create the polygon
            VTK_CREATE(vtkPolygon,polygon);
            polygon->GetPointIds()->SetNumberOfIds(poly.size());

            for (unsigned int s=0; s<poly.size(); s++) {
                floor_points->InsertNextPoint(poly[s]._x*FAKTOR,poly[s]._y*FAKTOR,sub->GetElevation(poly[s])*FAKTOR);
                polygon->GetPointIds()->SetId(s, currentFloorPolyID++);
            }
            floor_polygons->InsertNextCell(polygon);

            //plot the walls only for not stairs
            const vector<Wall>& walls= sub->GetAllWalls();
            for(unsigned int w=0; w<walls.size(); w++) {
                Point p1 = walls[w].GetPoint1();
                Point p2 = walls[w].GetPoint2();
                double z1= sub->GetElevation(p1);
                double z2= sub->GetElevation(p2);

                if(sub->GetType()=="stair") {
                    geometry->addStair(p1._x*FAKTOR, p1._y*FAKTOR, z1*FAKTOR, p2._x*FAKTOR, p2._y*FAKTOR,z2*FAKTOR);
                } else {
                    geometry->addWall(p1._x*FAKTOR, p1._y*FAKTOR, z1*FAKTOR, p2._x*FAKTOR, p2._y*FAKTOR,z2*FAKTOR);
                }
            }

            //insert the subroom caption
            string caption=itr_room.second->GetCaption()+" ( " + QString::number(sub->GetSubRoomID()).toStdString() + " ) ";
            const Point& p=sub->GetCentroid();
            double z= sub->GetElevation(p);
            double pos[3]= {p._x*FAKTOR,p._y*FAKTOR,z*FAKTOR};
            geometry->addObjectLabel(pos,pos,caption,captionsColor);

            //plot the obstacles
            for(auto obst:sub->GetAllObstacles())
            {
                for(auto wall: obst->GetAllWalls())
                {
                    Point p1 = wall.GetPoint1();
                    Point p2 = wall.GetPoint2();
                    double z1= sub->GetElevation(p1);
                    double z2= sub->GetElevation(p2);
                    geometry->addWall(p1._x*FAKTOR, p1._y*FAKTOR, z1*FAKTOR, p2._x*FAKTOR, p2._y*FAKTOR,z2*FAKTOR);
                }
                //add the obstacle caption
                const Point& p=obst->GetCentroid();
                double z= sub->GetElevation(p);
                double pos[3]= {p._x*FAKTOR,p._y*FAKTOR,z*FAKTOR};
                geometry->addObjectLabel(pos,pos,obst->GetCaption(),captionsColor);

                //add a special texture to the obstacles
                auto poly = obst->GetPolygon();
                //if(obst->IsClockwise()==true) {
                //  std::reverse(poly.begin(),poly.end());
                //}

                // Create the polygon
                VTK_CREATE(vtkPolygon,polygon);
                polygon->GetPointIds()->SetNumberOfIds(poly.size());

                for (unsigned int s=0; s<poly.size(); s++) {
                    obstacles_points->InsertNextPoint(poly[s]._x*FAKTOR,poly[s]._y*FAKTOR,sub->GetElevation(poly[s])*FAKTOR);
                    polygon->GetPointIds()->SetId(s, currentObstPolyID++);
                }
                obstacles_polygons->InsertNextCell(polygon);
            }
        }
    }

    // Create a PolyData to represent the floor
    VTK_CREATE(vtkPolyData, floorPolygonPolyData);
    floorPolygonPolyData->SetPoints(floor_points);
    floorPolygonPolyData->SetPolys(floor_polygons);
    geometry->addFloor(floorPolygonPolyData);

    // Create a PolyData to represen the obstacles
    //TODO:
    VTK_CREATE(vtkPolyData, obstPolygonPolyData);
    obstPolygonPolyData->SetPoints(obstacles_points);
    obstPolygonPolyData->SetPolys(obstacles_polygons);
    geometry->addObstacles(obstPolygonPolyData);


    // add the crossings
    const map<int, Crossing*>& crossings=building->GetAllCrossings();
    for (std::map<int, Crossing*>::const_iterator it=crossings.begin(); it!=crossings.end(); ++it) {
        Crossing* cr=it->second;
        Point p1 = cr->GetPoint1();
        Point p2 = cr->GetPoint2();
        double z1= cr->GetSubRoom1()->GetElevation(p1);
        double z2= cr->GetSubRoom1()->GetElevation(p2);
        geometry->addNavLine(p1._x*FAKTOR, p1._y*FAKTOR, z1*FAKTOR, p2._x*FAKTOR, p2._y*FAKTOR,z2*FAKTOR);

        const Point& p =cr->GetCentre();
        double pos[3]= {p._x*FAKTOR,p._y*FAKTOR,z1*FAKTOR};
        geometry->addObjectLabel(pos,pos,"nav_"+QString::number(cr->GetID()).toStdString(),captionsColor);
    }

    // add the exits
    const map<int, Transition*>& transitions=building->GetAllTransitions();
    for (std::map<int, Transition*>::const_iterator it=transitions.begin(); it!=transitions.end(); ++it) {
        Transition* tr=it->second;
        Point p1 = tr->GetPoint1();
        Point p2 = tr->GetPoint2();
        double z1= tr->GetSubRoom1()->GetElevation(p1);
        double z2= tr->GetSubRoom1()->GetElevation(p2);
        geometry->addDoor(p1._x*FAKTOR, p1._y*FAKTOR, z1*FAKTOR, p2._x*FAKTOR, p2._y*FAKTOR,z2*FAKTOR);

        const Point& p =tr->GetCentre();
        double pos[3]= {p._x*FAKTOR,p._y*FAKTOR,z1*FAKTOR};
        geometry->addObjectLabel(pos,pos,"door_"+QString::number(tr->GetID()).toStdString(),captionsColor);
    }

    //TODO:dirty hack for parsing the Hlines
    // free memory
    delete building;

    return true;
}


/// provided for convenience and will be removed in the next version

void SaxParser::parseGeometryTRAV(QString content, FacilityGeometry *geometry,QDomNode geo)
{

    cout<<"external geometry found"<<endl;
    //creating am empty document
    // to be filled
    QDomDocument doc("");
    QDomNode geoNode;

    //first try to open the file
    if(content.endsWith(".trav",Qt::CaseInsensitive) ) {
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

        if (geoNode.isNull()) {
            cout<<"No geometry information found. <geometry> <geometry/> tag is missing."<<endl;
        }
    } else {
        if(content.isEmpty()) {
            geoNode=geo;
            cout <<"parsing the old fashion way"<<endl;
        } else {
            content = "<travisto>\n" +content+ "\n</travisto>\n";
            QString errorMsg="";
            doc.setContent(content,&errorMsg);

            if(!errorMsg.isEmpty()) {
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
        for (  int i = 0; i < walls.length(); i++) {
            QDomElement el = walls.item(i).toElement();

            //wall thickness, default to 30 cm
            double thickness = el.attribute("thickness","15").toDouble()*FAKTOR;
            //wall height default to 250 cm
            double height = el.attribute("height","250").toDouble()*FAKTOR;
            //wall color default to blue
            double color = el.attribute("color","0").toDouble();

            //get the points defining each wall
            //not that a wall is not necessarily defined by two points, could be more...
            QDomNodeList points = el.elementsByTagName("point");
            for (  int i = 0; i < points.length() - 1; i++) {

                double x1=points.item(i).toElement().attribute("xPos", "0").toDouble()*FAKTOR;
                double y1=points.item(i).toElement().attribute("yPos", "0").toDouble()*FAKTOR;
                double z1=points.item(i).toElement().attribute("zPos", "0").toDouble()*FAKTOR;

                double x2=points.item(i+1).toElement().attribute("xPos", "0").toDouble()*FAKTOR;
                double y2=points.item(i+1).toElement().attribute("yPos", "0").toDouble()*FAKTOR;
                double z2=points.item(i+1).toElement().attribute("zPos", "0").toDouble()*FAKTOR;
                geometry->addWall(x1, y1,z1 ,x2, y2,z2,thickness,height,color);
            }
        }

        //parsing the doors
        if(doors.length()>0)
            for (  int i = 0; i < doors.length(); i++) {
                QDomElement el = doors.item(i).toElement();

                //door thickness, default to 15 cm
                double thickness = el.attribute("thickness","15").toDouble()*FAKTOR;
                //door height default to 250 cm
                double height = el.attribute("height","250").toDouble()*FAKTOR;
                //door color default to blue
                double color = el.attribute("color","255").toDouble();

                //get the points defining each wall
                //not that a wall is not necesarily defined by two points, could be more...
                QDomNodeList points = el.elementsByTagName("point");
                //Debug::Messages("found:  " << points.length() <<" for this wall" <<endl;
                for (  int i = 0; i < points.length() - 1; i++) {

                    double x1=points.item(i).toElement().attribute("xPos", "0").toDouble()*FAKTOR;
                    double y1=points.item(i).toElement().attribute("yPos", "0").toDouble()*FAKTOR;
                    double z1=points.item(i).toElement().attribute("zPos", "0").toDouble()*FAKTOR;

                    double x2=points.item(i+1).toElement().attribute("xPos", "0").toDouble()*FAKTOR;
                    double y2=points.item(i+1).toElement().attribute("yPos", "0").toDouble()*FAKTOR;
                    double z2=points.item(i+1).toElement().attribute("zPos", "0").toDouble()*FAKTOR;
                    geometry->addDoor(x1, y1, z1, x2, y2,z2,thickness,height,color);
                }
            }

        // parsing the objets
        for (  int i = 0; i < spheres.length(); i++) {

            double center[3];
            center[0] = spheres.item(i).toElement().attribute("centerX", "0").toDouble()*FAKTOR;
            center[1]= spheres.item(i).toElement().attribute("centerY", "0").toDouble()*FAKTOR;
            center[2]= spheres.item(i).toElement().attribute("centerZ", "0").toDouble()*FAKTOR;
            double color= spheres.item(i).toElement().attribute("color", "0").toDouble()*FAKTOR;
            double radius= spheres.item(i).toElement().attribute("radius", "0").toDouble()*FAKTOR;
            //double width = spheres.item(i).toElement().attribute("width", "0").toDouble();
            //double height= spheres.item(i).toElement().attribute("height", "0").toDouble();

            geometry->addObjectSphere(center,radius,color);
        }
        // cubic shapes
        for (  int i = 0; i < cuboids.length(); i++) {

            double center[3];
            center[0] = cuboids.item(i).toElement().attribute("centerX", "0").toDouble()*FAKTOR;
            center[1]= cuboids.item(i).toElement().attribute("centerY", "0").toDouble()*FAKTOR;
            center[2]= cuboids.item(i).toElement().attribute("centerZ", "0").toDouble()*FAKTOR;
            double color= cuboids.item(i).toElement().attribute("color", "0").toDouble();
            double length= cuboids.item(i).toElement().attribute("length", "0").toDouble()*FAKTOR;
            double width = cuboids.item(i).toElement().attribute("width", "0").toDouble()*FAKTOR;
            double height= cuboids.item(i).toElement().attribute("height", "0").toDouble()*FAKTOR;
            geometry->addObjectBox(center,height,width,length,color);
            //		Debug::Error("cuboids: "<<length<<" || " <<width << " || "<<height<<" || "<<color<<endl;
        }
        // floors
        for (  int i = 0; i < floors.length(); i++) {

            double left =floors.item(i).toElement().attribute("xMin","0").toDouble()*FAKTOR;
            double right =floors.item(i).toElement().attribute("xMax","0").toDouble()*FAKTOR;
            double up =floors.item(i).toElement().attribute("yMax","0").toDouble()*FAKTOR;
            double down =floors.item(i).toElement().attribute("yMin","0").toDouble()*FAKTOR;
            double z =floors.item(i).toElement().attribute("z","0").toDouble()*FAKTOR;
            geometry->addFloor(left,down,right,up,z);
        }
        // cylinders
        for (  int i = 0; i < cylinders.length(); i++) {

            double center[3], rotation[3];
            center[0] = cylinders.item(i).toElement().attribute("centerX", "0").toDouble()*FAKTOR;
            center[1]= cylinders.item(i).toElement().attribute("centerY", "0").toDouble()*FAKTOR;
            center[2]= cylinders.item(i).toElement().attribute("centerZ", "0").toDouble()*FAKTOR;
            double color= cylinders.item(i).toElement().attribute("color", "0").toDouble();
            double radius= cylinders.item(i).toElement().attribute("radius", "0").toDouble()*FAKTOR;
            double height= cylinders.item(i).toElement().attribute("height", "0").toDouble()*FAKTOR;
            rotation[0] = cylinders.item(i).toElement().attribute("angleX", "90").toDouble();
            rotation[1] = cylinders.item(i).toElement().attribute("angleY", "0").toDouble();
            rotation[2] = cylinders.item(i).toElement().attribute("angleZ", "0").toDouble();
            geometry->addObjectCylinder(center,radius,height,rotation,color);
        }

        //Labels
        for (  int i = 0; i < labels.length(); i++) {

            double center[3];
            center[0] = labels.item(i).toElement().attribute("centerX", "0").toDouble()*FAKTOR;
            center[1]= labels.item(i).toElement().attribute("centerY", "0").toDouble()*FAKTOR;
            center[2]= labels.item(i).toElement().attribute("centerZ", "0").toDouble()*FAKTOR;
            double color= labels.item(i).toElement().attribute("color", "0").toDouble();
            string caption= labels.item(i).toElement().attribute("text", "").toStdString();
            geometry->addObjectLabel(center,center,caption,color);
        }
        // you should normally have only one geometry node, but one never knows...
        geoNode = geoNode.nextSiblingElement("geometry");
    }
}

QString SaxParser::extractGeometryFilename(QString &filename)
{
    QString extracted_geo_name="";
    //first try to look at a string <file location="filename.xml"/>
    QFile file(filename);
    QString line;
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            //look for a line with
            line = in.readLine();
            //cout<<"checking: "<<line.toStdString()<<endl;
            if(line.contains("location" ,Qt::CaseInsensitive))
                if(line.contains("<file" ,Qt::CaseInsensitive)) {
                    //try to extract what ever is inside the quotes

                    QString begin="\"";
                    QString end="\"";
                    int startIndex = line.indexOf(begin)+begin.length();
                    if(startIndex <= 0)continue; //false alarm
                    int endIndex = line.indexOf(end,startIndex);
                    if(endIndex <= 0)continue; // false alarm
                    extracted_geo_name= line.mid(startIndex,endIndex - startIndex);
                    return extracted_geo_name;
                    //break;// we are done
                }
            if(line.contains("<geometry" ,Qt::CaseInsensitive))
                if(line.contains("version" ,Qt::CaseInsensitive)) {
                    //real geometry file
                    QFileInfo fileInfoGeometry(filename);
                    extracted_geo_name=fileInfoGeometry.fileName();
                    return extracted_geo_name;
                }
        }
    }

    //maybe this is already the geometry file itself ?
    //do a rapid test
//    FacilityGeometry* geo = new FacilityGeometry();
//    QFileInfo fileInfoGeometry(filename);
//    extracted_geo_name=fileInfoGeometry.fileName();

//    //just check if it starts with geometry
//    //if(parseGeometryJPS(extracted_geo_name,geo)==true)
//    //{
//        return extracted_geo_name;
//    //}
//    delete geo;

    return "";
}


void SaxParser::parseGeometryXMLV04(QString filename, FacilityGeometry *geo)
{
    QDomDocument doc("");

    QFile file(filename);

    int size =file.size()/(1024*1024);

    //avoid dom parsing a very large dataset
    if(size>500) {
        //cout<<"The file is too large: "<<filename.toStdString()<<endl;
        return;
    }

    //cout<<"filename: "<<filename.toStdString()<<endl;

    //TODO: check if you can parse this with the building classes.
    // This should be a fall back option

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

    if(version<0.4) {
        QMessageBox::warning(0, QObject::tr("Parsing Error"),
                             QObject::tr("Only geometry version >= 0.4 supported"));
    }

    //parsing the subrooms
    QDomNodeList xSubRoomsNodeList=doc.elementsByTagName("subroom");
    //parsing the walls
    for (  int i = 0; i < xSubRoomsNodeList.length(); i++) {
        QDomElement xPoly = xSubRoomsNodeList.item(i).firstChildElement("polygon");
        double position[3]= {0,0,0};
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

            for( int i=0; i<xVertices.count()-1; i++) {
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
            for( int i=0; i<xVertices.count()-1; i++) {
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
        double color = xCrossing.attribute("color","120").toDouble();
        QString id= xCrossing.attribute("id","-1");

        double x1=xVertices.item(0).toElement().attribute("px", "0").toDouble()*xToCmfactor;
        double y1=xVertices.item(0).toElement().attribute("py", "0").toDouble()*xToCmfactor;
        double z1=xVertices.item(0).toElement().attribute("pz", "0").toDouble()*xToCmfactor;

        double x2=xVertices.item(1).toElement().attribute("px", "0").toDouble()*xToCmfactor;
        double y2=xVertices.item(1).toElement().attribute("py", "0").toDouble()*xToCmfactor;
        double z2=xVertices.item(1).toElement().attribute("pz", "0").toDouble()*xToCmfactor;
        geo->addNavLine(x1, y1, z1, x2, y2,z2,thickness,height,color);

        double center[3]= {(x1+x2)/2.0, (y1+y2)/2.0, (z2+z1)/2.0};
        geo->addObjectLabel(center,center,id.toStdString(),21);
    }

    QDomNodeList xTransitionsList=doc.elementsByTagName("transition");
    for (int i = 0; i < xTransitionsList.length(); i++) {
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
        double center[3]= {(x1+x2)/2.0, (y1+y2)/2.0, (z2+z1)/2.0};
        geo->addObjectLabel(center,center,id,21);
    }
}

void SaxParser::InitHeader(int major, int minor, int patch)
{
    if ( (minor==6) || (minor==5 && patch==1) ) {
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
    } else {
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
    if(major!=0) {
        cout<<"unsupported header version: "<<major<<"."<<minor<<"."<<patch<<endl;
        cout<<"Please use 0.5 0.5.1 or 0.6 "<<endl;
        exit(0);
    }
}
