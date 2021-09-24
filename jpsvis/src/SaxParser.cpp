/**
 * @file    SaxParser.cpp
 * @author  Ulrich Kemloh <kemlohulrich@gmail.com>
 * @version 0.1
 * Created on: 6 Sep 2010
 * Copyright (C) <2009-2010>
 *
 * @section LICENSE
 * This file is part of JuPedSim.
 *
 * JuPedSim is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * JuPedSim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with JuPedSim. If not, see <http://www.gnu.org/licenses/>.
 *
 * @section DESCRIPTION
 *
 *
 *
 */
#include "SaxParser.h"

#include "Frame.h"
#include "FrameElement.h"
#include "Log.h"
#include "SyncData.h"
#include "SystemSettings.h"
#include "TrajectoryPoint.h"
#include "geometry/Building.h"
#include "geometry/FacilityGeometry.h"
#include "geometry/GeometryFactory.h"
#include "geometry/JPoint.h"
#include "geometry/SubRoom.h"
#include "geometry/Wall.h"

#include <QMessageBox>
#include <QProgressDialog>
#include <QPushButton>
#include <QString>
#include <cmath>
#include <iostream>
#include <limits>
#include <optional>
#include <vtkActor.h>
#include <vtkAssembly.h>
#include <vtkCellArray.h>
#include <vtkGenericDataObjectReader.h>
#include <vtkImageDataGeometryFilter.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyDataReader.h>
#include <vtkPolygon.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkStripper.h>
#include <vtkStructuredGridReader.h>
#include <vtkStructuredPointsReader.h>
#include <vtkTriangleFilter.h>
#include <vtkVersion.h>


#define VTK_CREATE(type, name) vtkSmartPointer<type> name = vtkSmartPointer<type>::New()


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
SaxParser::SaxParser(GeometryFactory & geoFac, SyncData & dataset, double * fps) :
    _geoFactory(geoFac), _dataset(dataset)
{
    _para             = fps;
    _parsingWalls     = false;
    _parsingCrossings = false;
    _color            = 0.0;
    _dataset.clearFrames();

    _geometry =
        std::shared_ptr<FacilityGeometry>(new FacilityGeometry("No name", "No name", "No name"));
    _geoFactory.AddElement(-1, -1, _geometry);

    // default header
    InitHeader(0, 0, 0);
}

SaxParser::~SaxParser() {}

bool SaxParser::startElement(
    const QString & /* namespaceURI */,
    const QString & /* localName */,
    const QString & qName,
    const QXmlAttributes & at)
{
    if(qName == "header") {
        for(int i = 0; i < at.length(); i++) {
            if(at.localName(i) == "version") {
                QStringList query = at.value(i).split(".");
                int major         = 0;
                int minor         = 0;
                int patch         = 0;
                switch(query.size()) {
                    case 1:
                        major = query.at(0).toInt();
                        break;
                    case 2:
                        major = query.at(0).toInt();
                        minor = query.at(1).toInt();
                        break;
                    case 3:
                        major = query.at(0).toInt();
                        minor = query.at(1).toInt();
                        patch = query.at(2).toInt();
                        break;
                }
                InitHeader(major, minor, patch);
            }
        }
    } else if(qName == "file") {
        for(int i = 0; i < at.length(); i++) {
            if(at.localName(i) == "location") {
                QString fileName = at.value(i);
                if(!fileName.isEmpty()) {
                    if(fileName.endsWith(".xml", Qt::CaseInsensitive)) {
                        // SaxParser::parseGeometryJPS(fileName,geometry);
                    } else if(fileName.endsWith(".trav", Qt::CaseInsensitive)) {
                        SaxParser::parseGeometryTRAV(fileName, _geoFactory);
                    }
                }
            }
        }
    } else if(qName == "source") {
        double xmin, xmax, ymin, ymax;
        double z = 0; // @todo read this some when we go 3D

        string source_id = "";
        for(int i = 0; i < at.length(); i++) {
            if(at.localName(i) == "id") {
                source_id = at.value(i).toStdString();
            } else if(at.localName(i) == "x_min") {
                xmin = at.value(i).toDouble() * FAKTOR;
            } else if(at.localName(i) == "x_max") {
                xmax = at.value(i).toDouble() * FAKTOR;
            } else if(at.localName(i) == "y_min") {
                ymin = at.value(i).toDouble() * FAKTOR;
            } else if(at.localName(i) == "y_max") {
                ymax = at.value(i).toDouble() * FAKTOR;
            }
        }
        _geometry->addRectangle(xmin, ymin, xmax, ymax, 0, 120.0, 150.0, source_id);
        //@todo: here z=0. What about sources in the 2 floor?
    }

    else if(qName == "goal") {
        double xmin, xmax, ymin, ymax;
        double z        = 0; // @todo read this some when we go 3D
        QString caption = "";
        for(int i = 0; i < at.length(); i++) {
            if(at.localName(i) == "caption") {
                caption = at.value(i);
            } else if(at.localName(i) == "x_min") {
                xmin = at.value(i).toDouble() * FAKTOR;
            } else if(at.localName(i) == "x_max") {
                xmax = at.value(i).toDouble() * FAKTOR;
            } else if(at.localName(i) == "y_min") {
                ymin = at.value(i).toDouble() * FAKTOR;
            } else if(at.localName(i) == "y_max") {
                ymax = at.value(i).toDouble() * FAKTOR;
            }
        }
        _geometry->addRectangle(xmin, ymin, xmax, ymax, z, 90.0, 90.0, caption.toStdString());
    } // goal
    else if(qName == "floor") {
        double xMin = 0, xMax = 0, yMin = 0, yMax = 0;

        for(int i = 0; i < at.length(); i++) {
            if(at.localName(i) == "xMin") {
                xMin = at.value(i).toDouble() * FAKTOR;
            } else if(at.localName(i) == "xMax") {
                xMax = at.value(i).toDouble() * FAKTOR;
            } else if(at.localName(i) == "yMin") {
                yMin = at.value(i).toDouble() * FAKTOR;
            } else if(at.localName(i) == "yMax") {
                yMax = at.value(i).toDouble() * FAKTOR;
            }
        }
        _geometry->addFloor(xMin, yMin, xMax, yMax);
    } else if(qName == "cuboid") {
        double length = 0, height = 0, width = 0, color = 0;
        double center[3] = {0, 0, 0};

        for(int i = 0; i < at.length(); i++) {
            if(at.localName(i) == "centerX") {
                center[0] = at.value(i).toDouble() * FAKTOR;
            } else if(at.localName(i) == "centerY") {
                center[1] = at.value(i).toDouble() * FAKTOR;
            } else if(at.localName(i) == "centerZ") {
                center[2] = at.value(i).toDouble() * FAKTOR;
            } else if(at.localName(i) == "length") {
                length = at.value(i).toDouble() * FAKTOR;
            } else if(at.localName(i) == "height") {
                height = at.value(i).toDouble() * FAKTOR;
            } else if(at.localName(i) == "width") {
                width = at.value(i).toDouble() * FAKTOR;
            } else if(at.localName(i) == "color") {
                color = at.value(i).toDouble() * FAKTOR;
            }
        }
        _geometry->addObjectBox(center, height, width, length, color);

    } else if(qName == "sphere") {
        double radius = 0, color = 0;
        double center[3] = {0, 0, 0};

        for(int i = 0; i < at.length(); i++) {
            if(at.localName(i) == "centerX") {
                center[0] = at.value(i).toDouble() * FAKTOR;
            } else if(at.localName(i) == "centerY") {
                center[1] = at.value(i).toDouble() * FAKTOR;
            } else if(at.localName(i) == "centerZ") {
                center[2] = at.value(i).toDouble() * FAKTOR;
            } else if(at.localName(i) == "radius") {
                radius = at.value(i).toDouble() * FAKTOR;
            } else if(at.localName(i) == "color") {
                color = at.value(i).toDouble();
            }
        }
        _geometry->addObjectSphere(center, radius, color);
    } else if(qName == "label") {
        double color     = 0;
        double center[3] = {0, 0, 0};
        QString text;

        for(int i = 0; i < at.length(); i++) {
            if(at.localName(i) == "centerX") {
                center[0] = at.value(i).toDouble() * FAKTOR;
            } else if(at.localName(i) == "centerY") {
                center[1] = at.value(i).toDouble() * FAKTOR;
            } else if(at.localName(i) == "centerZ") {
                center[2] = at.value(i).toDouble() * FAKTOR;
            } else if(at.localName(i) == "text") {
                text = at.value(i);
            } else if(at.localName(i) == "color") {
                color = at.value(i).toDouble();
            }
        }
        _geometry->addObjectLabel(center, center, text.toStdString(), color);
    } else if(qName == "cylinder") {
        double height = 0, radius = 0, color = 0;
        double center[3]   = {0, 0, 0};
        double rotation[3] = {0, 0, 0};

        for(int i = 0; i < at.length(); i++) {
            if(at.localName(i) == "centerX") {
                center[0] = at.value(i).toDouble() * FAKTOR;
            } else if(at.localName(i) == "centerY") {
                center[1] = at.value(i).toDouble() * FAKTOR;
            } else if(at.localName(i) == "centerZ") {
                center[2] = at.value(i).toDouble() * FAKTOR;
            } else if(at.localName(i) == "height") {
                height = at.value(i).toDouble() * FAKTOR;
            } else if(at.localName(i) == "radius") {
                radius = at.value(i).toDouble() * FAKTOR;
            } else if(at.localName(i) == "color") {
                color = at.value(i).toDouble();
            } else if(at.localName(i) == "angleX") {
                rotation[0] = at.value(i).toDouble();
            } else if(at.localName(i) == "angleY") {
                rotation[1] = at.value(i).toDouble();
            } else if(at.localName(i) == "angleZ") {
                rotation[2] = at.value(i).toDouble();
            }
        }
        _geometry->addObjectCylinder(center, radius, height, rotation, color);
    } else if(qName == "agents") {
    } else if(qName == "roomCaption") {
    } else if(qName == "frameRate") {
    } else if(qName == "geometry") {
    } else if(qName == "gradient_field") {
        for(int i = 0; i < at.length(); i++) {
            if(at.localName(i) == "filename") {
                ParseGradientFieldVTK(at.value(i), _geoFactory);
            }
        }

    } else if(qName == "wall") {
        _parsingWalls = true;
        _thickness    = 15;
        _height       = 250;
        _color        = 0;
        _caption      = "";

        for(int i = 0; i < at.length(); i++) {
            if(at.localName(i) == "thickness") {
                _thickness = at.value(i).toDouble() * FAKTOR;
            } else if(at.localName(i) == "height") {
                _height = at.value(i).toDouble() * FAKTOR;
            } else if(at.localName(i) == "color") {
                _color = at.value(i).toDouble();
            } else if(at.localName(i) == "caption") {
                _caption = at.value(i);
            }
        }

    } else if(qName == "door") {
        _parsingWalls = false;
        _thickness    = 15;
        _height       = 250;
        _color        = 255;
        _caption      = "";

        for(int i = 0; i < at.length(); i++) {
            if(at.localName(i) == "thickness") {
                _thickness = at.value(i).toDouble() * FAKTOR;
            } else if(at.localName(i) == "height") {
                _height = at.value(i).toDouble() * FAKTOR;
            } else if(at.localName(i) == "color") {
                _color = at.value(i).toDouble();
            } else if(at.localName(i) == "caption") {
                _caption = at.value(i);
            }
        }
    }
    //     <event time="9" type="door" state="open" id="0" caption="door0" />
    // call void GeometryFactory::ChangeExitsColor(double* color)
    // _geoFactory.ChangeExitsColor(Qt::red);

    // FIXME
    else if(qName == "crossing") {
        _parsingWalls     = false;
        _parsingCrossings = true;
        _thickness        = 15;
        _height           = 250;
        _color            = 255;
        _caption          = "";

        for(int i = 0; i < at.length(); i++) {
            if(at.localName(i) == "thickness") {
                _thickness = at.value(i).toDouble() * FAKTOR;
            } else if(at.localName(i) == "height") {
                _height = at.value(i).toDouble() * FAKTOR;
            } else if(at.localName(i) == "color") {
                _color = at.value(i).toDouble();
            } else if(at.localName(i) == "caption") {
                _caption = at.value(i);
            }
        }
    } else if(qName == "hline") {
        _parsingWalls     = false;
        _parsingCrossings = true;
        _thickness        = 15;
        _height           = 250;
        _color            = 255;
        _caption          = "";
        QString room_id, subroom_id;

        for(int i = 0; i < at.length(); i++) {
            if(at.localName(i) == "thickness") {
                _thickness = at.value(i).toDouble() * FAKTOR;
            } else if(at.localName(i) == "height") {
                _height = at.value(i).toDouble() * FAKTOR;
            } else if(at.localName(i) == "color") {
                _color = at.value(i).toDouble();
            } else if(at.localName(i) == "caption") {
                _caption = at.value(i);
            } else if(at.localName(i) == "room_id") {
                room_id = at.value(i);
            } else if(at.localName(i) == "subroom_id") {
                subroom_id = at.value(i);
            }
        }
        _caption = room_id + ":" + subroom_id + ":" + _caption;

    } else if(qName == "timeFirstFrame") {
        /*unsigned long timeFirstFrame_us=0;
          unsigned long timeFirstFrame_s=0;

          for(int i=0; i<at.length(); i++) {
          if(at.localName(i)=="microsec") {
          timeFirstFrame_us=at.value(i).toULong();
          } else if(at.localName(i)=="sec") {
          timeFirstFrame_s=at.value(i).toULong();
          }
          }
          dataset->setDelayAbsolute(timeFirstFrame_s,timeFirstFrame_us);
        */
    } else if(qName == "point") {
        double xPos = 0;
        double yPos = 0;
        double zPos = 0;

        for(int i = 0; i < at.length(); i++) {
            if(at.localName(i) == "xPos") {
                xPos = at.value(i).toDouble() * FAKTOR;
            } else if(at.localName(i) == "yPos") {
                yPos = at.value(i).toDouble() * FAKTOR;
            } else if(at.localName(i) == "zPos") {
                zPos = at.value(i).toDouble() * FAKTOR;
            }
        }
        double CHT[3] = {_color, _height, _thickness};
        JPoint * pt   = new JPoint(xPos, yPos, zPos);
        pt->setColorHeightThicknes(CHT);
        _currentPointsList.push_back(pt);

    } else if(qName == "frame") {
        for(int i = 0; i < at.length(); i++) {
            if(at.localName(i) == "ID") {
                _currentFrameID = at.value(i).toInt();
            }
        }

    } else if(qName == "agent") {
        int id      = 0;
        double xPos = 0;
        double yPos = 0;
        double zPos = 0;
        // double agent_color =std::numeric_limits<double>::quiet_NaN();
        // double xVel=std::numeric_limits<double>::quiet_NaN();
        // double yVel=std::numeric_limits<double>::quiet_NaN();
        // double zVel=std::numeric_limits<double>::quiet_NaN();
        double dia_a    = std::numeric_limits<double>::quiet_NaN();
        double dia_b    = std::numeric_limits<double>::quiet_NaN();
        double el_angle = std::numeric_limits<double>::quiet_NaN();
        double el_color = std::numeric_limits<double>::quiet_NaN();
        double el_x     = std::numeric_limits<double>::quiet_NaN();
        double el_y     = std::numeric_limits<double>::quiet_NaN();
        double el_z     = std::numeric_limits<double>::quiet_NaN();

        for(int i = 0; i < at.length(); i++) {
            if(at.localName(i) == "ID") {
                id = at.value(i).toInt();
                // TODO: maybe you should change ur format to take the ID 0 as first valid ID.
                if(id == 0) {
                    // slotErrorOutput("Person with ID=0 detected. ID should start with 1 !");
                    return false;
                }
            } else if(at.localName(i) == _jps_xPos) {
                xPos = at.value(i).toDouble() * FAKTOR;
                // xPos=at.value(i).toDouble();
            } else if(at.localName(i) == _jps_yPos) {
                // yPos=at.value(i).toDouble();
                yPos = at.value(i).toDouble() * FAKTOR;
            } else if(at.localName(i) == _jps_zPos) {
                zPos = at.value(i).toDouble() * FAKTOR;
            }

            else if(at.localName(i) == _jps_radiusA) {
                dia_a = at.value(i).toDouble() * FAKTOR;
                // dia_a=at.value(i).toDouble();
            } else if(at.localName(i) == _jps_radiusB) {
                dia_b = at.value(i).toDouble() * FAKTOR;
                // dia_b=at.value(i).toDouble();
            } else if(at.localName(i) == _jps_ellipseOrientation) {
                el_angle = at.value(i).toDouble();
            } else if(at.localName(i) == _jps_ellipseColor) {
                el_color = at.value(i).toDouble();
            } else if(at.localName(i) == "agentColor") {
                // agent_color=at.value(i).toDouble();
            } else if(at.localName(i) == _jps_xVel) {
                // xVel=at.value(i).toDouble()*FAKTOR;
            } else if(at.localName(i) == _jps_yVel) {
                // yVel=at.value(i).toDouble()*FAKTOR;
            } else if(at.localName(i) == _jps_zVel) {
                // zVel=at.value(i).toDouble()*FAKTOR;
            }
        }
        //        xml2txt

        double pos[3]    = {xPos, yPos, zPos};
        double angle[3]  = {0, 0, el_angle};
        double radius[3] = {dia_a, dia_b, 30.0};

        FrameElement * element = new FrameElement(id - 1);
        element->SetPos(pos);
        element->SetOrientation(angle);
        element->SetRadius(radius);
        element->SetColor(el_color);
        _currentFrame.push_back(element);

    } else if(qName == "agentInfo") {
        double height = std::numeric_limits<double>::quiet_NaN();
        double color  = std::numeric_limits<double>::quiet_NaN();
        double id     = std::numeric_limits<double>::quiet_NaN();

        for(int i = 0; i < at.length(); i++) {
            if(at.localName(i) == "ID") {
                id = at.value(i).toDouble();
            }
            if(at.localName(i) == "height") {
                height = at.value(i).toDouble() * FAKTOR;
            }
            if(at.localName(i) == "color") {
                color = at.value(i).toDouble();
            }
        }
        if(std::isnan(id))
            return true;

        if(!std::isnan(height)) {
            _initialPedestriansHeights.append(QString::number(int(id)));
            _initialPedestriansHeights.append(QString::number(height));
        }
        if(!std::isnan(color)) {
            _initialPedestriansColors.append(QString::number(int(id)));
            _initialPedestriansColors.append(QString::number(int(color)));
        }
    }
    return true;
}

bool SaxParser::characters(const QString & str)
{
    _currentText.append(str);
    return true;
}

bool SaxParser::endElement(
    const QString & /* namespaceURI */,
    const QString & /* localName */,
    const QString & qName)
{
    if(qName == "header") {
    } else if(qName == "agents") {
        _dataset.setNumberOfAgents(_currentText.toInt());
    } else if(qName == "frameRate") {
        _para[0] = _currentText.toFloat();
    } else if(qName == "wall") {
        if(_currentPointsList.size() > 1)
            for(unsigned int i = 0; i < _currentPointsList.size() - 1; i++) {
                _geometry->addWall(
                    _currentPointsList[i], _currentPointsList[i + 1], _caption.toStdString());
            }
        clearPoints();
    } else if(qName == "door") {
        for(unsigned int i = 0; i < _currentPointsList.size() - 1; i++) {
            _geometry->addDoor(
                _currentPointsList[i], _currentPointsList[i + 1], _caption.toStdString());
        }
        clearPoints();
    } else if(qName == "crossing") {
        if(_currentPointsList.size() > 1) // hack
            for(unsigned int i = 0; i < _currentPointsList.size() - 1; i++) {
                _geometry->addNavLine(
                    _currentPointsList[i], _currentPointsList[i + 1], _caption.toStdString());
            }
        clearPoints();
    } else if(qName == "hline") {
        if(_currentPointsList.size() > 1) {
            for(unsigned int i = 0; i < _currentPointsList.size() - 1; i++) {
                int room_id     = -1;
                int subroom_id  = -1;
                QStringList lst = _caption.split(":");
                if(lst.length() > 2) {
                    room_id    = lst[0].toInt();
                    subroom_id = lst[1].toInt();
                    _caption   = lst[2];
                }
                auto && geo = _geoFactory.GetElement(room_id, subroom_id);
                if(geo != nullptr) {
                    geo->addNavLine(
                        _currentPointsList[i], _currentPointsList[i + 1], _caption.toStdString());
                } else {
                    _geometry->addNavLine(
                        _currentPointsList[i], _currentPointsList[i + 1], _caption.toStdString());
                }
            }
        }
        clearPoints();
    } else if(qName == "step") { // FIXME
        for(unsigned int i = 0; i < _currentPointsList.size() - 1; i++) {
            _geometry->addDoor(
                _currentPointsList[i], _currentPointsList[i + 1], _caption.toStdString());
        }
        clearPoints();
    } else if(qName == "frame") {
        Frame * frame = new Frame(_currentFrameID);
        while(!_currentFrame.empty()) {
            frame->addElement(_currentFrame.back());
            _currentFrame.pop_back();
        }

        // compute the polydata, might increase the runtime
        frame->ComputePolyData();

        _dataset.addFrame(frame);
        // to be on the safe side
        _currentFrame.clear();

    } else if(qName == "agent") {
    } else if(qName == "geometry") {
    } else if(qName == "point") {
    } else if(qName == "shape") {
        _dataset.setInitialHeights(_initialPedestriansHeights);
        _dataset.setInitialColors(_initialPedestriansColors);
    } else if(qName == "gradient_field") {
    }
    _currentText.clear();
    return true;
}

bool SaxParser::fatalError(const QXmlParseException & exception)
{
    QMessageBox::warning(
        0,
        QObject::tr("SAX Handler"),
        QObject::tr("Parse error at line %1, column "
                    "%2:\n%3.")
            .arg(exception.lineNumber())
            .arg(exception.columnNumber())
            .arg(exception.message()));
    return false;
}

bool SaxParser::attributeDecl(
    const QString & eName,
    const QString & aName,
    const QString & type,
    const QString & valueDefault,
    const QString & value)
{
    QString dummy = eName + aName + type + valueDefault + value;
    return (dummy == dummy);
    // return true;
}

void SaxParser::clearPoints()
{
    while(!_currentPointsList.empty()) {
        delete _currentPointsList.back();
        _currentPointsList.pop_back();
    }
    _currentPointsList.clear();
    return;
}
double SaxParser::GetElevation(QString geometryFile, int roomId, int subroomId)
{
    double C_z;
    QString wd;
    QDir dir(wd);
    QDir fileDir(geometryFile);
    SystemSettings::getWorkingDirectory(wd);
    if(!fileDir.isAbsolute()) {
        QString s    = dir.relativeFilePath(geometryFile);
        geometryFile = wd + QDir::separator() + s;
    }
    QDomDocument doc("");
    QFile file(geometryFile);
    if(!file.open(QIODevice::ReadOnly)) {
        Log::Error(
            "GetElevation: could not open the file: %s\n", geometryFile.toStdString().c_str());
        exit(EXIT_FAILURE);
    }
    QString * errorCode = new QString();
    if(!doc.setContent(&file, errorCode)) {
        file.close();
        Log::Error(">> ErrorCode %d\n", errorCode);
        exit(EXIT_FAILURE);
    }
    TiXmlDocument docGeo(geometryFile.toStdString());
    if(!docGeo.LoadFile()) {
        Log::Info("%s", docGeo.ErrorDesc());
        Log::Error(
            "LoadGeometry: could not parse the geometry file %s\n",
            geometryFile.toStdString().c_str());
        return -1;
    }

    TiXmlElement * xRootNode = docGeo.RootElement();
    if(!xRootNode) {
        Log::Error("Root element does not exist");
        return -1;
    }
    TiXmlNode * xRoomsNode = xRootNode->FirstChild("rooms");
    if(!xRoomsNode) {
        Log::Error("The geometry should have at least one room and one subroom");
        return false;
    }
    for(TiXmlElement * xRoom = xRoomsNode->FirstChildElement("room"); xRoom;
        xRoom                = xRoom->NextSiblingElement("room")) {
        int room_id = xmltoi(xRoom->Attribute("id"), -1);
        if(room_id != roomId)
            continue;

        for(TiXmlElement * xSubRoom = xRoom->FirstChildElement("subroom"); xSubRoom;
            xSubRoom                = xSubRoom->NextSiblingElement("subroom")) {
            int subroom_id = xmltoi(xSubRoom->Attribute("id"), -1);
            if(subroom_id != subroomId)
                continue;
            C_z = xmltof(xSubRoom->Attribute("C_z"), 0.0);
        }
    }
    return C_z;
}
std::tuple<Point, Point> SaxParser::GetTrackStartEnd(QString geometryFile, int trackId)
{
    QString wd;
    QDir dir(wd);
    QDir fileDir(geometryFile);
    SystemSettings::getWorkingDirectory(wd);
    if(!fileDir.isAbsolute()) {
        QString s    = dir.relativeFilePath(geometryFile);
        geometryFile = wd + QDir::separator() + s;
    }

    // QString = QDir::cleanPath(wd + QDir::separator() + fileName);
    Log::Info("filename: <%s)", geometryFile.toStdString().c_str());
    Log::Info("wd: <%s>", wd.toStdString().c_str());


    std::vector<Point> end_points;
    Point start_point(0, 0);
    Point end_point(0, 0);

    QDomDocument doc("");
    QFile file(geometryFile);
    if(!file.open(QIODevice::ReadOnly)) {
        Log::Error(
            "GetTrackStartEnd: could not open the file: %s\n", geometryFile.toStdString().c_str());
        exit(EXIT_FAILURE);
    }
    QString * errorCode = new QString();
    if(!doc.setContent(&file, errorCode)) {
        file.close();
        Log::Error(">> ErrorCode: %d\n", errorCode);
        exit(EXIT_FAILURE);
    }
    QDomElement root = doc.firstChildElement("geometry");
    // only parsing the geometry node
    if(root.tagName() != "geometry") {
        std::tuple<Point, Point> ret = std::make_tuple(Point(0, 0), Point(0, 0));
        Log::Error("root %s is not geometry\n", root.tagName().toStdString().c_str());
        return ret;
    }


    // parsing the subrooms
    QDomNodeList xSubRoomsNodeList = doc.elementsByTagName("subroom");
    // parsing the walls
    for(int i = 0; i < xSubRoomsNodeList.length(); i++) {
        QDomElement xPoly = xSubRoomsNodeList.item(i).firstChildElement("polygon");
        while(!xPoly.isNull()) {
            auto Type          = xPoly.attribute("type", "notype").toStdString();
            auto Caption       = xPoly.attribute("caption", "nocaption").toStdString();
            int parsed_trackId = xPoly.attribute("track_id", "-1").toInt();
            if(Type != "track") {
                xPoly = xPoly.nextSiblingElement("polygon");
                continue;
            }

            if(parsed_trackId != trackId) {
                xPoly = xPoly.nextSiblingElement("polygon");
                continue;
            }
            QDomNodeList xVertices = xPoly.elementsByTagName("vertex");
            for(int i = 0; i < xVertices.count(); i++) {
                auto start = xVertices.item(i).toElement().attribute("start", "false");

                double x1 = xVertices.item(i).toElement().attribute("px", "0").toDouble();
                double y1 = xVertices.item(i).toElement().attribute("py", "0").toDouble();
                if(start == "true") {
                    start_point = Point(x1, y1);
                } else {
                    end_points.push_back(Point(x1, y1)); // collect other points of track
                }
            }
            xPoly = xPoly.nextSiblingElement("polygon");
        } // poly
    }     // sub

    double min_d = -10000;
    // find the track point with the biggest distance to start.
    for(auto p : end_points) {
        double d = (p - start_point).NormSquare();
        if(d > min_d) {
            end_point = p;
            min_d     = d;
        }
    }
    std::tuple<Point, Point> ret = std::make_tuple(start_point, end_point);
    return ret;
}


bool SaxParser::parseGeometryJPS(QString fileName, GeometryFactory & geoFac)
{
    Log::Info(
        "Enter SaxParser::parseGeometryJPS with filename <%s>", fileName.toStdString().c_str());

    double captionsColor = 0; // red
    QDir fileDir(fileName);
    QString wd;
    QDir dir(wd);
    SystemSettings::getWorkingDirectory(wd);

    if(!fileName.endsWith(".xml", Qt::CaseInsensitive))
        return false;
    if(!fileDir.isAbsolute()) {
        QString s = dir.relativeFilePath(fileName);
        fileName  = wd + QDir::separator() + s;
    }

    // QString = QDir::cleanPath(wd + QDir::separator() + fileName);
    Log::Info("filename: <%s)", fileName.toStdString().c_str());
    Log::Info("wd: <%s>", wd.toStdString().c_str());
    Building * building = new Building();
    string geometrypath = fileName.toStdString();
    building->SetProjectRootDir(wd.toStdString());

    // read the geometry
    if(!building->LoadGeometry(geometrypath))
        return false;
    if(!building->InitGeometry())
        return false; // create the polygons

    int room_id    = -1;
    int subroom_id = -1;
    for(auto && itr_room : building->GetAllRooms()) {
        room_id++;
        for(auto && itr_subroom : itr_room.second->GetAllSubRooms()) {
            subroom_id++;
            string room_caption = itr_room.second->GetCaption() + "_RId_" +
                                  QString::number(itr_room.first).toStdString();
            string subroom_caption = itr_subroom.second->GetCaption() + "_RId_" +
                                     QString::number(itr_room.first).toStdString();
            auto geometry = shared_ptr<FacilityGeometry>(
                new FacilityGeometry(itr_subroom.second->GetType(), room_caption, subroom_caption));

            int currentFloorPolyID = 0;
            int currentObstPolyID  = 0;

            // Setup the points
            VTK_CREATE(vtkPoints, floor_points);
            VTK_CREATE(vtkPoints, obstacles_points);
            // Add the polygon to a list of polygons
            VTK_CREATE(vtkCellArray, floor_polygons);
            VTK_CREATE(vtkCellArray, obstacles_polygons);

            // string caption = r->GetCaption();
            SubRoom * sub = itr_subroom.second.get();

            vector<Point> poly = sub->GetPolygon();
            if(sub->IsClockwise() == true) {
                std::reverse(poly.begin(), poly.end());
            }

            // Create the polygon
            VTK_CREATE(vtkPolygon, polygon);
            polygon->GetPointIds()->SetNumberOfIds(poly.size());

            for(unsigned int s = 0; s < poly.size(); s++) {
                floor_points->InsertNextPoint(
                    poly[s]._x * FAKTOR, poly[s]._y * FAKTOR, sub->GetElevation(poly[s]) * FAKTOR);
                polygon->GetPointIds()->SetId(s, currentFloorPolyID++);
            }
            floor_polygons->InsertNextCell(polygon);

            // plot the walls only for not stairs
            const vector<Wall> & walls = sub->GetAllWalls();
            for(unsigned int w = 0; w < walls.size(); w++) {
                Point p1  = walls[w].GetPoint1();
                Point p2  = walls[w].GetPoint2();
                double z1 = sub->GetElevation(p1);
                double z2 = sub->GetElevation(p2);

                if(sub->GetType() == "stair") {
                    geometry->addStair(
                        p1._x * FAKTOR,
                        p1._y * FAKTOR,
                        z1 * FAKTOR,
                        p2._x * FAKTOR,
                        p2._y * FAKTOR,
                        z2 * FAKTOR);
                } else {
                    geometry->addWall(
                        p1._x * FAKTOR,
                        p1._y * FAKTOR,
                        z1 * FAKTOR,
                        p2._x * FAKTOR,
                        p2._y * FAKTOR,
                        z2 * FAKTOR);
                }
            }

            // insert the subroom caption
            string caption = itr_room.second->GetCaption() + " ( " +
                             QString::number(sub->GetSubRoomID()).toStdString() + " ) ";
            const Point & p = sub->GetCentroid();
            double z        = sub->GetElevation(p);
            double pos[3]   = {p._x * FAKTOR, p._y * FAKTOR, z * FAKTOR};
            geometry->addObjectLabel(pos, pos, caption, captionsColor);

            // plot the obstacles
            for(auto obst : sub->GetAllObstacles()) {
                for(auto wall : obst->GetAllWalls()) {
                    Point p1  = wall.GetPoint1();
                    Point p2  = wall.GetPoint2();
                    double z1 = sub->GetElevation(p1);
                    double z2 = sub->GetElevation(p2);
                    geometry->addWall(
                        p1._x * FAKTOR,
                        p1._y * FAKTOR,
                        z1 * FAKTOR,
                        p2._x * FAKTOR,
                        p2._y * FAKTOR,
                        z2 * FAKTOR);
                }
                // add the obstacle caption
                const Point & p = obst->GetCentroid();
                double z        = sub->GetElevation(p);
                double pos[3]   = {p._x * FAKTOR, p._y * FAKTOR, z * FAKTOR};
                geometry->addObjectLabel(pos, pos, obst->GetCaption(), captionsColor);

                // add a special texture to the obstacles
                auto poly = obst->GetPolygon();
                // if(obst->IsClockwise()==true) {
                //  std::reverse(poly.begin(),poly.end());
                //}

                // Create the polygon
                VTK_CREATE(vtkPolygon, polygon);
                polygon->GetPointIds()->SetNumberOfIds(poly.size());

                for(unsigned int s = 0; s < poly.size(); s++) {
                    obstacles_points->InsertNextPoint(
                        poly[s]._x * FAKTOR,
                        poly[s]._y * FAKTOR,
                        sub->GetElevation(poly[s]) * FAKTOR);
                    polygon->GetPointIds()->SetId(s, currentObstPolyID++);
                }
                obstacles_polygons->InsertNextCell(polygon);
            }

            // Create a PolyData to represent the floor
            VTK_CREATE(vtkPolyData, floorPolygonPolyData);
            floorPolygonPolyData->SetPoints(floor_points);
            floorPolygonPolyData->SetPolys(floor_polygons);
            geometry->addFloor(floorPolygonPolyData);

            // Create a PolyData to represen the obstacles
            VTK_CREATE(vtkPolyData, obstPolygonPolyData);
            obstPolygonPolyData->SetPoints(obstacles_points);
            obstPolygonPolyData->SetPolys(obstacles_polygons);
            geometry->addObstacles(obstPolygonPolyData);

            // add the crossings
            for(auto && cr : itr_subroom.second->GetAllCrossings()) {
                Point p1  = cr->GetPoint1();
                Point p2  = cr->GetPoint2();
                double z1 = cr->GetSubRoom1()->GetElevation(p1);
                double z2 = cr->GetSubRoom1()->GetElevation(p2);
                geometry->addNavLine(
                    p1._x * FAKTOR,
                    p1._y * FAKTOR,
                    z1 * FAKTOR,
                    p2._x * FAKTOR,
                    p2._y * FAKTOR,
                    z2 * FAKTOR);

                const Point & p = cr->GetCentre();
                double pos[3]   = {p._x * FAKTOR, p._y * FAKTOR, z1 * FAKTOR};
                geometry->addObjectLabel(
                    pos,
                    pos,
                    "nav_" + QString::number(cr->GetID()).toStdString() + "_" +
                        QString::number(cr->GetUniqueID()).toStdString(),
                    captionsColor);
            }

            // add the exits
            for(auto && tr : itr_subroom.second->GetAllTransitions()) {
                Point p1  = tr->GetPoint1();
                Point p2  = tr->GetPoint2();
                double z1 = 0;
                double z2 = 0;

                if(tr->GetSubRoom1()) // get elevation for both points
                {
                    z2 = tr->GetSubRoom1()->GetElevation(p2);
                    z1 = tr->GetSubRoom1()->GetElevation(p1);
                } else if(!tr->GetSubRoom2()) {
                    z2 = tr->GetSubRoom2()->GetElevation(p2);
                    z1 = tr->GetSubRoom2()->GetElevation(p1);
                } else
                    Log::Error(
                        "Can not calculate elevations for transition %d:%s. Both subrooms are not "
                        "defined\n",
                        tr->GetID(),
                        tr->GetCaption().c_str());

                geometry->addDoor(
                    p1._x * FAKTOR,
                    p1._y * FAKTOR,
                    z1 * FAKTOR,
                    p2._x * FAKTOR,
                    p2._y * FAKTOR,
                    z2 * FAKTOR);

                const Point & p = tr->GetCentre();
                double pos[3]   = {p._x * FAKTOR, p._y * FAKTOR, z1 * FAKTOR};
                geometry->addObjectLabel(
                    pos,
                    pos,
                    "door_" + QString::number(tr->GetID()).toStdString() + +"_" +
                        QString::number(tr->GetUniqueID()).toStdString(),
                    captionsColor);
            }

            geoFac.AddElement(room_id, subroom_id, geometry);
        }
    }
    // free memory
    delete building;
    return true;
}

/// provided for convenience and will be removed in the next version
void SaxParser::parseGeometryTRAV(QString content, GeometryFactory & geoFac, QDomNode geo)
{
    Log::Info("external geometry found");
    // creating am empty document
    // to be filled
    QDomDocument doc("");
    QDomNode geoNode;
    auto geometry =
        shared_ptr<FacilityGeometry>(new FacilityGeometry("no name", "no name", "no name"));

    // first try to open the file
    if(content.endsWith(".trav", Qt::CaseInsensitive)) {
        QFile file(content);
        if(!file.open(QIODevice::ReadOnly)) {
            Log::Error("could not open the File");
            return;
        }
        QString * errorCode = new QString();
        if(!doc.setContent(&file, errorCode)) {
            file.close();
            // slotErrorOutput(*errorCode);
            Log::Error("error code: <%s>", errorCode->toStdString().c_str());
            return;
        }
        file.close();
        geoNode = doc.documentElement().namedItem("geometry");

        if(geoNode.isNull()) {
            Log::Error("No geometry information found. <geometry> <geometry/> tag is missing.");
        }
    } else {
        if(content.isEmpty()) {
            geoNode = geo;
            Log::Info("parsing the old fashion way");
        } else {
            content          = "<travisto>\n" + content + "\n</travisto>\n";
            QString errorMsg = "";
            doc.setContent(content, &errorMsg);

            if(!errorMsg.isEmpty()) {
                Log::Error("%s", (const char *) errorMsg.toStdString().c_str());
                return;
            }
            geoNode = doc.elementsByTagName("geometry").item(0);
        }
    }

    // for the case there is more than just one geometry Node
    while(!geoNode.isNull()) {
        QDomElement e      = geoNode.toElement();
        QDomNodeList walls = e.elementsByTagName("wall");
        QDomNodeList doors = e.elementsByTagName("door");

        // objects which can be positioned everywhere in the facility
        QDomNodeList spheres   = e.elementsByTagName("sphere");
        QDomNodeList cuboids   = e.elementsByTagName("cuboid");
        QDomNodeList floors    = e.elementsByTagName("floor");
        QDomNodeList cylinders = e.elementsByTagName("cylinder");
        QDomNodeList labels    = e.elementsByTagName("label");


        // parsing the walls
        for(int i = 0; i < walls.length(); i++) {
            QDomElement el = walls.item(i).toElement();

            // wall thickness, default to 30 cm
            double thickness = el.attribute("thickness", "15").toDouble() * FAKTOR;
            // wall height default to 250 cm
            double height = el.attribute("height", "250").toDouble() * FAKTOR;
            // wall color default to blue
            double color = el.attribute("color", "0").toDouble();

            // get the points defining each wall
            // not that a wall is not necessarily defined by two points, could be more...
            QDomNodeList points = el.elementsByTagName("point");
            for(int i = 0; i < points.length() - 1; i++) {
                double x1 = points.item(i).toElement().attribute("xPos", "0").toDouble() * FAKTOR;
                double y1 = points.item(i).toElement().attribute("yPos", "0").toDouble() * FAKTOR;
                double z1 = points.item(i).toElement().attribute("zPos", "0").toDouble() * FAKTOR;

                double x2 =
                    points.item(i + 1).toElement().attribute("xPos", "0").toDouble() * FAKTOR;
                double y2 =
                    points.item(i + 1).toElement().attribute("yPos", "0").toDouble() * FAKTOR;
                double z2 =
                    points.item(i + 1).toElement().attribute("zPos", "0").toDouble() * FAKTOR;
                geometry->addWall(x1, y1, z1, x2, y2, z2, thickness, height, color);
            }
        }

        // parsing the doors
        if(doors.length() > 0)
            for(int i = 0; i < doors.length(); i++) {
                QDomElement el = doors.item(i).toElement();

                // door thickness, default to 15 cm
                double thickness = el.attribute("thickness", "15").toDouble() * FAKTOR;
                // door height default to 250 cm
                double height = el.attribute("height", "250").toDouble() * FAKTOR;
                // door color default to blue
                double color = el.attribute("color", "255").toDouble();

                // get the points defining each wall
                // not that a wall is not necesarily defined by two points, could be more...
                QDomNodeList points = el.elementsByTagName("point");
                // Debug::Messages("found:  " << points.length() <<" for this wall" <<endl;
                for(int i = 0; i < points.length() - 1; i++) {
                    double x1 =
                        points.item(i).toElement().attribute("xPos", "0").toDouble() * FAKTOR;
                    double y1 =
                        points.item(i).toElement().attribute("yPos", "0").toDouble() * FAKTOR;
                    double z1 =
                        points.item(i).toElement().attribute("zPos", "0").toDouble() * FAKTOR;

                    double x2 =
                        points.item(i + 1).toElement().attribute("xPos", "0").toDouble() * FAKTOR;
                    double y2 =
                        points.item(i + 1).toElement().attribute("yPos", "0").toDouble() * FAKTOR;
                    double z2 =
                        points.item(i + 1).toElement().attribute("zPos", "0").toDouble() * FAKTOR;
                    geometry->addDoor(x1, y1, z1, x2, y2, z2, thickness, height, color);
                }
            }

        // parsing the objets
        for(int i = 0; i < spheres.length(); i++) {
            double center[3];
            center[0] = spheres.item(i).toElement().attribute("centerX", "0").toDouble() * FAKTOR;
            center[1] = spheres.item(i).toElement().attribute("centerY", "0").toDouble() * FAKTOR;
            center[2] = spheres.item(i).toElement().attribute("centerZ", "0").toDouble() * FAKTOR;
            double color = spheres.item(i).toElement().attribute("color", "0").toDouble() * FAKTOR;
            double radius =
                spheres.item(i).toElement().attribute("radius", "0").toDouble() * FAKTOR;
            // double width = spheres.item(i).toElement().attribute("width", "0").toDouble();
            // double height= spheres.item(i).toElement().attribute("height", "0").toDouble();

            geometry->addObjectSphere(center, radius, color);
        }
        // cubic shapes
        for(int i = 0; i < cuboids.length(); i++) {
            double center[3];
            center[0] = cuboids.item(i).toElement().attribute("centerX", "0").toDouble() * FAKTOR;
            center[1] = cuboids.item(i).toElement().attribute("centerY", "0").toDouble() * FAKTOR;
            center[2] = cuboids.item(i).toElement().attribute("centerZ", "0").toDouble() * FAKTOR;
            double color = cuboids.item(i).toElement().attribute("color", "0").toDouble();
            double length =
                cuboids.item(i).toElement().attribute("length", "0").toDouble() * FAKTOR;
            double width = cuboids.item(i).toElement().attribute("width", "0").toDouble() * FAKTOR;
            double height =
                cuboids.item(i).toElement().attribute("height", "0").toDouble() * FAKTOR;
            geometry->addObjectBox(center, height, width, length, color);
            //		Debug::Error("cuboids: "<<length<<" || " <<width << " || "<<height<<" ||
            //"<<color<<endl;
        }
        // floors
        for(int i = 0; i < floors.length(); i++) {
            double left  = floors.item(i).toElement().attribute("xMin", "0").toDouble() * FAKTOR;
            double right = floors.item(i).toElement().attribute("xMax", "0").toDouble() * FAKTOR;
            double up    = floors.item(i).toElement().attribute("yMax", "0").toDouble() * FAKTOR;
            double down  = floors.item(i).toElement().attribute("yMin", "0").toDouble() * FAKTOR;
            double z     = floors.item(i).toElement().attribute("z", "0").toDouble() * FAKTOR;
            geometry->addFloor(left, down, right, up, z);
        }
        // cylinders
        for(int i = 0; i < cylinders.length(); i++) {
            double center[3], rotation[3];
            center[0] = cylinders.item(i).toElement().attribute("centerX", "0").toDouble() * FAKTOR;
            center[1] = cylinders.item(i).toElement().attribute("centerY", "0").toDouble() * FAKTOR;
            center[2] = cylinders.item(i).toElement().attribute("centerZ", "0").toDouble() * FAKTOR;
            double color = cylinders.item(i).toElement().attribute("color", "0").toDouble();
            double radius =
                cylinders.item(i).toElement().attribute("radius", "0").toDouble() * FAKTOR;
            double height =
                cylinders.item(i).toElement().attribute("height", "0").toDouble() * FAKTOR;
            rotation[0] = cylinders.item(i).toElement().attribute("angleX", "90").toDouble();
            rotation[1] = cylinders.item(i).toElement().attribute("angleY", "0").toDouble();
            rotation[2] = cylinders.item(i).toElement().attribute("angleZ", "0").toDouble();
            geometry->addObjectCylinder(center, radius, height, rotation, color);
        }

        // Labels
        for(int i = 0; i < labels.length(); i++) {
            double center[3];
            center[0]    = labels.item(i).toElement().attribute("centerX", "0").toDouble() * FAKTOR;
            center[1]    = labels.item(i).toElement().attribute("centerY", "0").toDouble() * FAKTOR;
            center[2]    = labels.item(i).toElement().attribute("centerZ", "0").toDouble() * FAKTOR;
            double color = labels.item(i).toElement().attribute("color", "0").toDouble();
            string caption = labels.item(i).toElement().attribute("text", "").toStdString();
            geometry->addObjectLabel(center, center, caption, color);
        }
        // you should normally have only one geometry node, but one never knows...
        geoNode = geoNode.nextSiblingElement("geometry");
    }

    geoFac.AddElement(0, 0, geometry);
}

QString SaxParser::extractGeometryFilename(QString & filename)
{
    QString extracted_geo_name = "";
    // first try to look at a string <file location="filename.xml"/>
    QFile file(filename);
    QString line;
    if(file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while(!in.atEnd()) {
            // look for a line with
            line = in.readLine();
            if(line.contains("location", Qt::CaseInsensitive))
                if(line.contains("<file", Qt::CaseInsensitive)) {
                    // try to extract what ever is inside the quotes

                    QString begin  = "\"";
                    QString end    = "\"";
                    int startIndex = line.indexOf(begin) + begin.length();
                    if(startIndex <= 0)
                        continue; // false alarm
                    int endIndex = line.indexOf(end, startIndex);
                    if(endIndex <= 0)
                        continue; // false alarm
                    extracted_geo_name = line.mid(startIndex, endIndex - startIndex);
                    return extracted_geo_name;
                    // break;// we are done
                }
            if(line.contains("<geometry", Qt::CaseInsensitive))
                if(line.contains("version", Qt::CaseInsensitive)) {
                    // real geometry file
                    QFileInfo fileInfoGeometry(filename);
                    extracted_geo_name = fileInfoGeometry.fileName();
                    return extracted_geo_name;
                }
        }
    }

    // maybe this is already the geometry file itself ?
    // do a rapid test
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
// not used yet!!
bool SaxParser::getSourcesTXT(QString & filename)
{
    Log::Info("Enter getSourcesTXT with %s\n", filename.toStdString().c_str());

    std::string sfilename = filename.toStdString();
    TiXmlDocument docSource(sfilename);
    if(!docSource.LoadFile()) {
        Log::Error("%s", docSource.ErrorDesc());
        Log::Error("could not parse the sources file.");
        return false;
    }

    TiXmlElement * xRootNodeSource = docSource.RootElement();
    if(!xRootNodeSource) {
        Log::Error("Root element does not exist in source file.");
        return false;
    }
    if(xRootNodeSource->ValueStr() != "JPScore") {
        Log::Error("Root element value in source file is not 'JPScore'.");
        return false;
    }
    TiXmlNode * xSourceF = xRootNodeSource->FirstChild("agents_sources");
    if(!xSourceF) {
        Log::Error("No agents_sources tag in file not found.");
        return false;
    }
    Log::Info("Loading sources from file");
    TiXmlNode * xSourceNodeF = xSourceF->FirstChild("source");
    if(xSourceNodeF) {
        for(TiXmlElement * e = xSourceF->FirstChildElement("source"); e;
            e                = e->NextSiblingElement("source")) {
            float xmin    = xmltof(e->Attribute("x_min"), 0);
            float xmax    = xmltof(e->Attribute("x_max"), 0);
            float ymin    = xmltof(e->Attribute("y_min"), 0);
            float ymax    = xmltof(e->Attribute("y_max"), 0);
            bool dont_add = (xmin == 0) && (xmax == 0) && (ymin == 0) && (ymax == 0);
            // if(! dont_add)
            // _geometry->addSource(xmin,ymin,xmax,ymax);
        } // for
    }
    return true;
}


QString SaxParser::extractSourceFileTXT(QString & filename)
{
    QString extracted_source_name = "";
    QFile file(filename);
    QString line;
    if(file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while(!in.atEnd()) {
            // look for a line with
            line = in.readLine();
            if(line.split(":").size() == 2) {
                if(line.split(":")[0].contains("sources", Qt::CaseInsensitive)) {
                    extracted_source_name = line.split(":")[1].simplified().remove(' ');
                    break;
                }
            }
        } // while
    }     // if open
    if(extracted_source_name == "") {
        Log::Warning("Could not extract source file!");
    }

    else
        Log::Info(
            "Extracted source from TXT file <%s>", extracted_source_name.toStdString().c_str());
    return extracted_source_name;
}

QString SaxParser::extractTrainTypeFileTXT(QString & filename)
{
    QString extracted_tt_name = "";
    QFile file(filename);
    QString line;
    if(file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while(!in.atEnd()) {
            // look for a line with
            line = in.readLine();
            if(line.split(":").size() == 2) {
                if(line.split(":")[0].contains("trainType", Qt::CaseInsensitive)) {
                    extracted_tt_name = line.split(":")[1].simplified().remove(' ');
                    break;
                }
            }
        } // while
    }     // if open
    if(extracted_tt_name == "") {
        Log::Warning("Could not extract trainType file!");
    }

    else
        Log::Info(
            "Extracted trainType from TXT file <%s>", extracted_tt_name.toStdString().c_str());
    return extracted_tt_name;
}

QString SaxParser::extractTrainTimeTableFileTXT(QString & filename)
{
    QString extracted_ttt_name = "";
    QFile file(filename);
    QString line;
    if(file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while(!in.atEnd()) {
            // look for a line with
            line = in.readLine();
            if(line.split(":").size() == 2) {
                if(line.split(":")[0].contains("trainTimeTable", Qt::CaseInsensitive)) {
                    extracted_ttt_name = line.split(":")[1].simplified().remove(' ');
                    break;
                }
            }
        } // while
    }     // if open
    if(extracted_ttt_name == "") {
        Log::Warning("Could not extract trainTimeTable file!");
    }

    else
        Log::Info(
            "Extracted trainTimeTable from TXT file <%s>",
            extracted_ttt_name.toStdString().c_str());
    return extracted_ttt_name;
}


QString SaxParser::extractGoalFileTXT(QString & filename)
{
    QString extracted_goal_name = "";
    QFile file(filename);
    QString line;
    if(file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while(!in.atEnd()) {
            // look for a line with
            line = in.readLine();
            if(line.split(":").size() == 2) {
                if(line.split(":")[0].contains("goals", Qt::CaseInsensitive)) {
                    extracted_goal_name = line.split(":")[1].simplified().remove(' ');
                    break;
                }
            }
        } // while
    }     // if open
    if(extracted_goal_name == "") {
        Log::Warning("Could not extract goal file!");
    }

    else
        Log::Info("Extracted goal from TXT file <%s>", extracted_goal_name.toStdString().c_str());
    return extracted_goal_name;
}


QString SaxParser::extractGeometryFilenameTXT(QString & filename)
{
    QString extracted_geo_name = "";
    QFile file(filename);
    QString line;
    if(file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while(!in.atEnd()) {
            // look for a line with
            line = in.readLine();
            if(line.split(":").size() == 2) {
                if(line.split(":")[0].contains("geometry", Qt::CaseInsensitive)) {
                    extracted_geo_name = line.split(":")[1].simplified().remove(' ');
                    break;
                }
            }
        } // while
    }     // if open
    if(extracted_geo_name == "") {
        Log::Warning("Could not extract geometry file!");
        //          extracted_geo_name = "geo.xml";
    }

    else
        Log::Info(
            "Extracted geometry from TXT file <%s>", extracted_geo_name.toStdString().c_str());
    return extracted_geo_name;
}


void SaxParser::parseGeometryXMLV04(QString filename, GeometryFactory & geoFac)
{
    Log::Info("Parsing XML v04");
    QDomDocument doc("");
    QFile file(filename);

    int size = file.size() / (1024 * 1024);

    // avoid dom parsing a very large dataset
    if(size > 500) {
        Log::Error("The file is too large: %s", filename.toStdString().c_str());
        return;
    }

    auto geo = shared_ptr<FacilityGeometry>(new FacilityGeometry("no name", "no name", "no name"));

    // TODO: check if you can parse this with the building classes.
    // This should be a fall back option

    if(!file.open(QIODevice::ReadOnly)) {
        qDebug() << "could not open the file: " << filename << Qt::endl;
        return;
    }
    QString * errorCode = new QString();
    if(!doc.setContent(&file, errorCode)) {
        file.close();
        qDebug() << errorCode << Qt::endl;
        return;
    }
    QDomElement root = doc.documentElement();

    // only parsing the geometry node
    if(root.tagName() != "geometry")
        return;


    double version = root.attribute("version", "-1").toDouble();

    string unit        = root.attribute("unit", "cm").toStdString();
    double xToCmfactor = 100;
    if(unit == "cm")
        xToCmfactor = 1;
    if(unit == "m")
        xToCmfactor = 100;

    if(version < 0.4) {
        QMessageBox::warning(
            0, QObject::tr("Parsing Error"), QObject::tr("Only geometry version >= 0.4 supported"));
    }

    // parsing the subrooms
    QDomNodeList xSubRoomsNodeList = doc.elementsByTagName("subroom");
    // parsing the walls
    for(int i = 0; i < xSubRoomsNodeList.length(); i++) {
        QDomElement xPoly  = xSubRoomsNodeList.item(i).firstChildElement("polygon");
        double position[3] = {0, 0, 0};
        double pos_count   = 1;
        double color       = 0;

        while(!xPoly.isNull()) {
            // wall thickness, default to 30 cm
            double thickness = xPoly.attribute("thickness", "15").toDouble();
            // wall height default to 250 cm
            double height = xPoly.attribute("height", "250").toDouble();
            // wall color default to blue
            color = xPoly.attribute("color", "0").toDouble();

            QDomNodeList xVertices = xPoly.elementsByTagName("vertex");
            pos_count += xVertices.count() - 1;

            for(int i = 0; i < xVertices.count() - 1; i++) {
                // all unit are converted in cm
                double x1 =
                    xVertices.item(i).toElement().attribute("px", "0").toDouble() * xToCmfactor;
                double y1 =
                    xVertices.item(i).toElement().attribute("py", "0").toDouble() * xToCmfactor;
                double z1 =
                    xVertices.item(i).toElement().attribute("pz", "0").toDouble() * xToCmfactor;
                double x2 =
                    xVertices.item(i + 1).toElement().attribute("px", "0").toDouble() * xToCmfactor;
                double y2 =
                    xVertices.item(i + 1).toElement().attribute("py", "0").toDouble() * xToCmfactor;
                double z2 =
                    xVertices.item(i + 1).toElement().attribute("pz", "0").toDouble() * xToCmfactor;

                position[0] += x1;
                position[1] += y1;
                position[2] += z1;

                geo->addWall(x1, y1, z1, x2, y2, z2, thickness, height, color);
            }
            xPoly = xPoly.nextSiblingElement("polygon");
        }

        // add the caption
        string roomCaption =
            xSubRoomsNodeList.item(i).parentNode().toElement().attribute("caption").toStdString();
        string subroomCaption = xSubRoomsNodeList.item(i).toElement().attribute("id").toStdString();
        string caption        = roomCaption + " ( " + subroomCaption + " ) ";
        position[0] /= pos_count;
        position[1] /= pos_count;
        position[2] /= pos_count;
        geo->addObjectLabel(position, position, caption, color);
        geo->SetRoomCaption(roomCaption);
        geo->SetSubRoomCaption(subroomCaption);
    }

    QDomNodeList xObstaclesList = doc.elementsByTagName("obstacle");
    for(int i = 0; i < xObstaclesList.length(); i++) {
        QDomElement xPoly = xObstaclesList.item(i).firstChildElement("polygon");
        while(!xPoly.isNull()) {
            // wall thickness, default to 30 cm
            double thickness = xPoly.attribute("thickness", "15").toDouble();
            // wall height default to 250 cm
            double height = xPoly.attribute("height", "250").toDouble();
            // wall color default to blue
            double color = xPoly.attribute("color", "0").toDouble();

            QDomNodeList xVertices = xPoly.elementsByTagName("vertex");
            for(int i = 0; i < xVertices.count() - 1; i++) {
                double x1 =
                    xVertices.item(i).toElement().attribute("px", "0").toDouble() * xToCmfactor;
                double y1 =
                    xVertices.item(i).toElement().attribute("py", "0").toDouble() * xToCmfactor;
                double z1 =
                    xVertices.item(i).toElement().attribute("pz", "0").toDouble() * xToCmfactor;

                double x2 =
                    xVertices.item(i + 1).toElement().attribute("px", "0").toDouble() * xToCmfactor;
                double y2 =
                    xVertices.item(i + 1).toElement().attribute("py", "0").toDouble() * xToCmfactor;
                double z2 =
                    xVertices.item(i + 1).toElement().attribute("pz", "0").toDouble() * xToCmfactor;
                geo->addWall(x1, y1, z1, x2, y2, z2, thickness, height, color);
            }
            xPoly = xPoly.nextSiblingElement("polygon");
        }
    }

    QDomNodeList xCrossingsList = doc.elementsByTagName("crossing");

    for(int i = 0; i < xCrossingsList.length(); i++) {
        QDomElement xCrossing  = xCrossingsList.item(i).toElement();
        QDomNodeList xVertices = xCrossing.elementsByTagName("vertex");

        /// door thickness, default to 15 cm
        double thickness = xCrossing.attribute("thickness", "15").toDouble();
        // door height default to 250 cm
        double height = xCrossing.attribute("height", "250").toDouble();
        // door color default to blue
        double color = xCrossing.attribute("color", "120").toDouble();
        QString id   = xCrossing.attribute("id", "-1");

        double x1 = xVertices.item(0).toElement().attribute("px", "0").toDouble() * xToCmfactor;
        double y1 = xVertices.item(0).toElement().attribute("py", "0").toDouble() * xToCmfactor;
        double z1 = xVertices.item(0).toElement().attribute("pz", "0").toDouble() * xToCmfactor;

        double x2 = xVertices.item(1).toElement().attribute("px", "0").toDouble() * xToCmfactor;
        double y2 = xVertices.item(1).toElement().attribute("py", "0").toDouble() * xToCmfactor;
        double z2 = xVertices.item(1).toElement().attribute("pz", "0").toDouble() * xToCmfactor;
        geo->addNavLine(x1, y1, z1, x2, y2, z2, thickness, height, color);

        double center[3] = {(x1 + x2) / 2.0, (y1 + y2) / 2.0, (z2 + z1) / 2.0};
        geo->addObjectLabel(center, center, id.toStdString(), 21);
    }

    QDomNodeList xTransitionsList = doc.elementsByTagName("transition");
    for(int i = 0; i < xTransitionsList.length(); i++) {
        QDomElement xTransition = xTransitionsList.item(i).toElement();
        QDomNodeList xVertices  = xTransition.elementsByTagName("vertex");

        /// door thickness, default to 15 cm
        double thickness = xTransition.attribute("thickness", "15").toDouble();
        // door height default to 250 cm
        double height = xTransition.attribute("height", "250").toDouble();
        // door color default to blue
        double color = xTransition.attribute("color", "255").toDouble();

        double x1 = xVertices.item(0).toElement().attribute("px", "0").toDouble() * xToCmfactor;
        double y1 = xVertices.item(0).toElement().attribute("py", "0").toDouble() * xToCmfactor;
        double z1 = xVertices.item(0).toElement().attribute("pz", "0").toDouble() * xToCmfactor;

        double x2 = xVertices.item(1).toElement().attribute("px", "0").toDouble() * xToCmfactor;
        double y2 = xVertices.item(1).toElement().attribute("py", "0").toDouble() * xToCmfactor;
        double z2 = xVertices.item(1).toElement().attribute("pz", "0").toDouble() * xToCmfactor;
        geo->addDoor(x1, y1, z1, x2, y2, z2, thickness, height, color);

        string id        = xTransition.attribute("id", "-1").toStdString();
        double center[3] = {(x1 + x2) / 2.0, (y1 + y2) / 2.0, (z2 + z1) / 2.0};
        geo->addObjectLabel(center, center, id, 21);
    }

    // room 0, subroom 0
    geoFac.AddElement(0, 0, geo);
}

bool SaxParser::ParseTxtFormat(const QString & fileName, SyncData * dataset, double * fps)
{
    Log::Info("parsing txt trajectory <%s> ", fileName.toStdString().c_str());
    *fps = 16; // default value
    QFile inputFile(fileName);
    if(inputFile.open(QIODevice::ReadOnly)) {
        QTextStream in(&inputFile);
        int lastFrameID = -1;
        int maxFrame    = 1000;
        // initialize the process dialog
        QProgressDialog progressDialog("Simulation", "Cancel", 1, maxFrame, NULL);
        progressDialog.setModal(true);
        //_progressDialog->setStyleSheet(stylesheet);
        progressDialog.setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
        //_progressDialog->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint |
        // Qt::WindowTitleHint|Qt::WindowStaysOnTopHint);
        progressDialog.setFixedSize(400, 100);
        progressDialog.setLabelText("<h3>Loading...</h3>");
        QList<QPushButton *> buttons = progressDialog.findChildren<QPushButton *>();
        buttons.at(0)->hide(); // that is the cancel button
        progressDialog.setValue(1);
        progressDialog.show();

        double unitFactor = FAKTOR; // @todo: use correct unit
        int minFrame      = 0;
        while(!in.atEnd()) {
            QString line = in.readLine();
            if(line.startsWith("#")) // looking for framerate
            {
                if(line.split(":").size() == 2) {
                    if(line.split(":")[0].contains("framerate", Qt::CaseInsensitive)) {
                        *fps = line.split(":")[1].toDouble();
                        Log::Info("Frame rate  <%.0f>", *fps);
                    }
                }
                continue;
            }
            line = line.trimmed(); // remove whitespaces from
                                   // beginning and end
            QStringList pieces = line.split(QRegExp("\\s+"));

            double pos[3];
            double angle[3]  = {0, 0, 30};
            double radius[3] = {0.3 * FAKTOR, 0.3 * FAKTOR, 0.3 * FAKTOR};

            int agentID     = -1;
            int frameID     = -1;
            double color    = 155;
            static int once = 1;
            switch(pieces.size()) {
                case 5:
                    agentID = pieces[0].toInt();
                    frameID = pieces[1].toInt();
                    if(once) // first frame we get
                    {
                        minFrame = frameID;
                        once     = 0;
                    }


                    // todo: for some reason when trajectories start
                    // with frames bigger than 0, display is not correct

                    pos[0] = pieces[2].toDouble() * unitFactor;
                    pos[1] = pieces[3].toDouble() * unitFactor;
                    pos[2] = pieces[4].toDouble() * unitFactor;
                    break;

                case 9:
                case 10:
                case 11:
                case 12:
                case 13:
                case 14:
                    agentID   = pieces[0].toInt();
                    frameID   = pieces[1].toInt();
                    color     = pieces[8].toDouble();
                    pos[0]    = pieces[2].toDouble() * unitFactor;
                    pos[1]    = pieces[3].toDouble() * unitFactor;
                    pos[2]    = pieces[4].toDouble() * unitFactor;
                    radius[0] = pieces[5].toDouble() * unitFactor;
                    radius[1] = pieces[6].toDouble() * unitFactor;
                    angle[2]  = pieces[7].toDouble();
                    if(once) // first frame we get
                    {
                        minFrame = frameID;
                        Log::Info("minFrame =  %d\n", minFrame);
                        once = 0;
                    }
                    break;

                default:
                    // try to scan the line for the unit
                    if(line.contains("centimeter", Qt::CaseInsensitive) ||
                       line.contains("centimetre", Qt::CaseInsensitive)) {
                        unitFactor = 0.01;
                        Log::Info("unit centimetre detected");
                    } else if(
                        line.contains("meter", Qt::CaseInsensitive) ||
                        line.contains("metre", Qt::CaseInsensitive)) {
                        unitFactor = 1;
                        Log::Info("unit metre detected");
                    } else {
                        Log::Warning("Ignoring line: <%s>", line.toStdString().c_str());
                    }
                    continue; // next line
                    break;
            }
            frameID -= minFrame;
            FrameElement * element = new FrameElement(agentID - 1);
            element->SetPos(pos);
            element->SetOrientation(angle);
            element->SetRadius(radius);
            element->SetColor(color);
            element->SetMinFrame(minFrame);
            if(dataset->GetFrames().count(frameID) < 1) {
                Frame * frame = new Frame(frameID);
                frame->addElement(element);
                dataset->addFrame(frame);
            } else {
                dataset->GetFrames()[frameID]->addElement(element);
            }

            // a new frame is starting.
            // not longer necessary if you are using maps and frameid
            if(frameID != lastFrameID) {
                progressDialog.setValue(dataset->getSize());
                lastFrameID = frameID;
                QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
            }
        }

        inputFile.close();
        Log::Info("%d frames added!", dataset->GetFrames().size());
        // construct the polydata
        for(const auto & frame : dataset->GetFrames()) {
            frame.second->ComputePolyData();
        }
    } else {
        Log::Error("could not open the file  <%s>", fileName.toStdString().c_str());
        return false;
    }
    return true;
}

bool SaxParser::ParseGradientFieldVTK(QString fileName, GeometryFactory & geoFac)
{
    if(QFileInfo(fileName).isRelative()) {
        QString wd;
        SystemSettings::getWorkingDirectory(wd);
        fileName = wd + "/" + fileName;
    }

    Log::Info("Opening the gradient field: %s", fileName.toStdString().c_str());
    // Read the file
    VTK_CREATE(vtkStructuredPointsReader, reader);
    reader->SetFileName(fileName.toStdString().c_str());
    reader->Update();
    reader->SetLookupTableName("LOOKUP_TABLE default");

    VTK_CREATE(vtkImageDataGeometryFilter, geometryFilter);
    geometryFilter->SetInputConnection(reader->GetOutputPort());
    geometryFilter->Update();

    VTK_CREATE(vtkPolyDataMapper, mapper);
    mapper->SetInputConnection(geometryFilter->GetOutputPort());

    VTK_CREATE(vtkActor, actor);
    actor->SetMapper(mapper);
    // conversion from m to cm
    actor->SetScale(100);

    auto gradient_field =
        shared_ptr<FacilityGeometry>(new FacilityGeometry("Gradient Field", "no name", "no name"));
    gradient_field->addGradientField(actor);

    geoFac.AddElement(-1, -1, gradient_field);
    geoFac.AddElement(-2, -2, gradient_field);
    return true;
}

void SaxParser::InitHeader(int major, int minor, int patch)
{
    if((minor == 6) || (minor == 5 && patch == 1)) {
        _jps_xPos               = QString("x");
        _jps_yPos               = QString("y");
        _jps_zPos               = QString("z");
        _jps_xVel               = QString("xV");
        _jps_yVel               = QString("yV");
        _jps_zVel               = QString("zV");
        _jps_radiusA            = QString("rA");
        _jps_radiusB            = QString("rB");
        _jps_ellipseOrientation = QString("eO");
        _jps_ellipseColor       = QString("eC");
    } else {
        _jps_xPos               = QString("xPos");
        _jps_yPos               = QString("yPos");
        _jps_zPos               = QString("zPos");
        _jps_xVel               = QString("xVel");
        _jps_yVel               = QString("yVel");
        _jps_zVel               = QString("zVel");
        _jps_radiusA            = QString("radiusA");
        _jps_radiusB            = QString("radiusB");
        _jps_ellipseOrientation = QString("ellipseOrientation");
        _jps_ellipseColor       = QString("ellipseColor");
    }
    if(major != 0) {
        Log::Info("unsupported header version: %d.%d.%d\n", major, minor, patch);
        exit(EXIT_FAILURE);
    }
}


bool SaxParser::LoadTrainTimetable(
    std::string Filename,
    std::map<int, std::shared_ptr<TrainTimeTable>> & trainTimeTables)
{
    TiXmlDocument docTTT(Filename);
    if(!docTTT.LoadFile()) {
        Log::Error("%s", docTTT.ErrorDesc());
        Log::Error("could not parse the train timetable file.");
        return false;
    }
    TiXmlElement * xTTT = docTTT.RootElement();
    if(!xTTT) {
        Log::Error("Root element does not exist in TTT file.");
        return false;
    }
    if(xTTT->ValueStr() != "train_time_table") {
        Log::Error("Parsing train timetable file. Root element value is not 'train_time_table'.");
        return false;
    }
    for(TiXmlElement * e = xTTT->FirstChildElement("train"); e;
        e                = e->NextSiblingElement("train")) {
        std::shared_ptr<TrainTimeTable> TTT = parseTrainTimeTableNode(e);
        if(TTT) { // todo: maybe get pointer to train
            if(trainTimeTables.count(TTT->id) != 0) {
                Log::Warning("Duplicate id for train time table found [%d]", TTT->id);
                exit(EXIT_FAILURE);
            }
            // get track start
            trainTimeTables[TTT->id] = TTT;
        } else {
            Log::Error("Something went south!\n");
        }
    }
    return true;
}


bool SaxParser::LoadTrainType(
    std::string Filename,
    std::map<std::string, std::shared_ptr<TrainType>> & trainTypes)
{
    TiXmlDocument docTT(Filename);
    if(!docTT.LoadFile()) {
        Log::Error("%s", docTT.ErrorDesc());
        Log::Error("could not parse the train type file.");
        return false;
    }
    TiXmlElement * xTT = docTT.RootElement();
    if(!xTT) {
        Log::Error("Root element does not exist in TT file.");
        return false;
    }
    if(xTT->ValueStr() != "train_type") {
        Log::Error("Parsing train type file. Root element value is not 'train_type'.");
        return false;
    }
    for(TiXmlElement * e = xTT->FirstChildElement("train"); e; e = e->NextSiblingElement("train")) {
        std::shared_ptr<TrainType> TT = parseTrainTypeNode(e);
        if(TT) {
            if(trainTypes.count(TT->_type.c_str()) != 0) {
                Log::Warning("Duplicate type for train found [%s]", TT->_type.c_str());
            }
            trainTypes[TT->_type] = TT;
        }
    }
    return true;
}


std::shared_ptr<TrainTimeTable> SaxParser::parseTrainTimeTableNode(TiXmlElement * e)
{
    Log::Info("Loading train time table NODE");
    // std::string caption = xmltoa(e->Attribute("caption"), "-1");
    int id              = xmltoi(e->Attribute("id"), -1);
    int track_id        = xmltoi(e->Attribute("track_id"), -1);
    double train_offset = xmltof(e->Attribute("train_offset"), -1);
    bool reversed       = false;
    double elevation    = 5; // dummy default value
    std::string in      = xmltoa(e->Attribute("reversed"), "false");
    std::transform(in.begin(), in.end(), in.begin(), ::tolower);
    if(in == "false") {
        reversed = false;
    } else if(in == "true") {
        reversed = true;
    } else {
        reversed = false;
    }
    std::string type     = xmltoa(e->Attribute("type"), "-1");
    int room_id          = xmltoi(e->Attribute("room_id"), -1);
    int subroom_id       = xmltoi(e->Attribute("subroom_id"), -1);
    float arrival_time   = xmltof(e->Attribute("arrival_time"), -1);
    float departure_time = xmltof(e->Attribute("departure_time"), -1);
    // @todo: check these values for correctness e.g. arrival < departure
    Log::Info("Train time table:");
    Log::Info("   id: %d", id);
    Log::Info("   type: %s", type.c_str());
    Log::Info("   room_id: %d", room_id);
    Log::Info("   subroom_id: %d", subroom_id);
    Log::Info("   track_id: %d", track_id);
    Log::Info("   train_offset: %.2f", train_offset);
    Log::Info("   arrival_time: %.2f", arrival_time);
    Log::Info("departure_time: %.2f", departure_time);
    // Debug::Info("Reversed: {}", reversed);
    std::shared_ptr<TrainTimeTable> trainTimeTab = std::make_shared<TrainTimeTable>(TrainTimeTable{
        id,
        type,
        room_id,
        subroom_id,
        arrival_time,
        departure_time,
        Point(0, 0),
        Point(0, 0),
        Point(0, 0),
        Point(0, 0),
        track_id,
        false,
        false,
        reversed,
        train_offset,
        elevation,
        vtkSmartPointer<vtkPolyDataMapper>::New(),
        vtkSmartPointer<vtkActor>::New(),
        vtkSmartPointer<vtkTextActor3D>::New(),
    });

    return trainTimeTab;
}
std::shared_ptr<TrainType> SaxParser::parseTrainTypeNode(TiXmlElement * node)
{
    Log::Info("Loading train type");

    std::string type = xmltoa(node->Attribute("type"), "NO_TYPE");
    if(type == "NO_TYPE") {
        Log::Warning("No train type name given. Use 'NO_TYPE' instead.");
    }
    Log::Info("type: {}", type.c_str());

    int agents_max = xmltoi(node->Attribute("agents_max"), std::numeric_limits<int>::max());
    if(agents_max == std::numeric_limits<int>::max()) {
        Log::Warning("No agents_max given. Set to default: {}.", agents_max);
    }
    Log::Info("max Agents: {}", agents_max);

    // Read and check if correct value
    double length = -std::numeric_limits<double>::infinity();
    if(const char * attribute = node->Attribute("length"); attribute) {
        if(double value = xmltof(attribute, -std::numeric_limits<double>::infinity());
           value >= 0.) {
            length = value;
        } else {
            Log::Warning(
                "{}: input for length should be non-negative {}. Skip entry.", type.c_str(), value);
            return nullptr;
        }
    } else {
        Log::Warning("{}: input for length not found. Skip entry.", type.c_str());
        return nullptr;
    }
    Log::Info("length: {}", length);


    std::vector<TrainDoor> doors;
    for(TiXmlElement * xDoor = node->FirstChildElement("door"); xDoor != nullptr;
        xDoor                = xDoor->NextSiblingElement("door")) {
        // Read distance and check if correct value
        double distance = -std::numeric_limits<double>::infinity();
        if(const char * attribute = xDoor->Attribute("distance"); attribute) {
            if(double value = xmltof(attribute, -std::numeric_limits<double>::infinity());
               value >= 0.) {
                distance = value;
            } else {
                Log::Warning(
                    "{}: input for distance should be non-negative {}. Skip entry.",
                    type.c_str(),
                    value);
                continue;
            }
        } else {
            Log::Warning("{}: input for distance not found. Skip entry.", type.c_str());
            continue;
        }

        // Read width and check if correct value
        double width = -std::numeric_limits<double>::infinity();
        if(const char * attribute = xDoor->Attribute("width"); attribute) {
            if(double value = xmltof(attribute, -std::numeric_limits<double>::infinity());
               value > 0.) {
                width = value;
            } else {
                Log::Warning(
                    "{}: input for width should be non-negative {}. Skip entry.",
                    type.c_str(),
                    value);
                continue;
            }
        } else {
            Log::Warning("{}: input for width not found. Skip entry.", type.c_str());
            continue;
        }

        // Read flow and check if correct value
        double flow = -std::numeric_limits<double>::infinity();
        if(const char * attribute = xDoor->Attribute("flow"); attribute) {
            if(double value = xmltof(attribute, -std::numeric_limits<double>::infinity());
               value > 0.) {
                flow = value;
            } else {
                Log::Warning(
                    "{}: input for flow should be >0 but is {:5.2}. Skip entry.",
                    type.c_str(),
                    value);
                continue;
            }
        }

        doors.emplace_back(TrainDoor{distance, width, flow});
    }

    if(doors.empty()) {
        Log::Error("Train {}: no doors given. Train will be ignored.", type.c_str());
        return nullptr;
    }

    Log::Info("number of doors: {}", doors.size());
    for(const auto & d : doors) {
        Log::Info(
            "Door:\tdistance: {%5.2f}\twidth: {%5.2f}\toutflow: {%5.2f}",
            d._distance,
            d._width,
            d._flow);
    }

    std::shared_ptr<TrainType> Type = std::make_shared<TrainType>(TrainType{
        type,
        agents_max,
        length,
        doors,
    });
    return Type;
}
