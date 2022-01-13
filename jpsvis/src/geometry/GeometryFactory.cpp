#include "GeometryFactory.h"

#include "FacilityGeometry.h"

#include <vtkAssembly.h>
#include <vtkRenderer.h>

GeometryFactory::GeometryFactory() {}

void GeometryFactory::Init(vtkRenderer * renderer)
{
    for(auto && rooms : _geometryFactory) {
        for(auto && subroom : rooms.second) {
            subroom.second->CreateActors();
            renderer->AddActor(subroom.second->getActor2D());
            renderer->AddActor(subroom.second->getActor3D());
        }
    }
}

void GeometryFactory::Set2D(bool status)
{
    for(auto && room : _geometryFactory) {
        for(auto && subroom : room.second) {
            if(_geometryFactory[room.first][subroom.first]->getVisibility())
                subroom.second->set2D(status);
        }
    }
}

void GeometryFactory::Set3D(bool status)
{
    for(auto && room : _geometryFactory) {
        for(auto && subroom : room.second) {
            if(_geometryFactory[room.first][subroom.first]->getVisibility())
                subroom.second->set3D(status);
        }
    }
}

void GeometryFactory::Clear()
{
    _geometryFactory.clear();
    _model.clear();
    _model.setObjectName("");
}

void GeometryFactory::ChangeWallsColor(double * color)
{
    for(auto && room : _geometryFactory) {
        for(auto && subroom : room.second) {
            if(_geometryFactory[room.first][subroom.first]->getVisibility())
                subroom.second->changeWallsColor(color);
        }
    }
}

void GeometryFactory::ChangeExitsColor(double * color)
{
    for(auto && room : _geometryFactory) {
        for(auto && subroom : room.second) {
            if(_geometryFactory[room.first][subroom.first]->getVisibility())
                subroom.second->changeExitsColor(color);
        }
    }
}

void GeometryFactory::ChangeNavLinesColor(double * color)
{
    for(auto && room : _geometryFactory) {
        for(auto && subroom : room.second) {
            if(_geometryFactory[room.first][subroom.first]->getVisibility())
                subroom.second->changeNavLinesColor(color);
        }
    }
}

void GeometryFactory::ChangeFloorColor(double * color)
{
    for(auto && room : _geometryFactory) {
        for(auto && subroom : room.second) {
            if(_geometryFactory[room.first][subroom.first]->getVisibility())
                subroom.second->changeFloorColor(color);
        }
    }
}

void GeometryFactory::ChangeObstaclesColor(double * color)
{
    for(auto && room : _geometryFactory) {
        for(auto && subroom : room.second) {
            if(_geometryFactory[room.first][subroom.first]->getVisibility())
                subroom.second->changeObstaclesColor(color);
        }
    }
}

void GeometryFactory::ShowDoors(bool status)
{
    for(auto && room : _geometryFactory) {
        for(auto && subroom : room.second) {
            if(_geometryFactory[room.first][subroom.first]->getVisibility())
                subroom.second->showDoors(status);
        }
    }
}

void GeometryFactory::ShowStairs(bool status)
{
    for(auto && room : _geometryFactory) {
        for(auto && subroom : room.second) {
            if(_geometryFactory[room.first][subroom.first]->getVisibility())
                subroom.second->showStairs(status);
        }
    }
}

void GeometryFactory::ShowWalls(bool status)
{
    for(auto && room : _geometryFactory) {
        for(auto && subroom : room.second) {
            if(_geometryFactory[room.first][subroom.first]->getVisibility())
                subroom.second->showWalls(status);
        }
    }
}

void GeometryFactory::ShowNavLines(bool status)
{
    for(auto && room : _geometryFactory) {
        for(auto && subroom : room.second) {
            if(_geometryFactory[room.first][subroom.first]->getVisibility())
                subroom.second->showNavLines(status);
        }
    }
}

void GeometryFactory::ShowFloor(bool status)
{
    for(auto && room : _geometryFactory) {
        for(auto && subroom : room.second) {
            if(_geometryFactory[room.first][subroom.first]->getVisibility())
                subroom.second->showFloor(status);
        }
    }
}
void GeometryFactory::ShowObstacles(bool status)
{
    for(auto && room : _geometryFactory) {
        for(auto && subroom : room.second) {
            if(_geometryFactory[room.first][subroom.first]->getVisibility())
                subroom.second->showObstacles(status);
        }
    }
}


void GeometryFactory::ShowGradientField(bool status)
{
    for(auto && room : _geometryFactory) {
        for(auto && subroom : room.second) {
            if(_geometryFactory[room.first][subroom.first]->getVisibility())
                subroom.second->showGradientField(status);
        }
    }
}

void GeometryFactory::ShowGeometryLabels(int status)
{
    for(auto && room : _geometryFactory) {
        for(auto && subroom : room.second) {
            if(_geometryFactory[room.first][subroom.first]->getVisibility())
                subroom.second->showGeometryLabels(status);
        }
    }
}

bool GeometryFactory::RefreshView()
{
    int count = -2;
    if(_model.objectName() != "initialized") {
        _model.setObjectName("initialized");
        _model.setHorizontalHeaderItem(0, new QStandardItem("Entity"));
        //_model.setHorizontalHeaderItem( 1, new QStandardItem( "Description" ) );
        for(auto && room : _geometryFactory) {
            count++;
            // room caption
            // QStandardItem *roomcaption = new QStandardItem( QString("R %0").arg(room.first));
            // roomcaption->setEditable( false );
            //_model.setItem(room.first, 1, roomcaption);
            QString roomCaption;
            if(room.first >= 0)
                roomCaption = QString::fromStdString(
                    (_geometryFactory[room.first].begin())->second->GetRoomCaption());
            else
                roomCaption = "empty";
            QStandardItem * item =
                new QStandardItem(QString("Room: %0 (%1)").arg(room.first).arg(roomCaption));
            //                                                   .arg(QString::fromStdString(
            //   _geometryFactory[room.first][0]->GetRoomCaption()

            // room.second[0].second->GetRoomCaption()
            item->setCheckable(true);
            item->setCheckState(Qt::Checked);

            for(auto && subroom : room.second) {
                QStandardItem * child = new QStandardItem(
                    QString("%2: %0 (%1)")
                        .arg(subroom.first)
                        .arg(QString::fromStdString(subroom.second->GetSubRoomCaption()))
                        .arg(QString::fromStdString(subroom.second->GetDescription())));
                child->setEditable(false);
                child->setCheckable(true);
                child->setCheckState(Qt::Checked);
                item->appendRow(child);
                _model.setItem(count, 0, item);
                QString data = QString("%0:%1").arg(room.first).arg(subroom.first);
                child->setData(data);
                // Subroom caption
                // QStandardItem *childcaption = new QStandardItem( QString("S
                // %0").arg(subroom.first)); childcaption->setEditable( false );
                //_model.setItem(room.first, 1, childcaption);
            }
        }
        return true;
    }
    return false;
}

const std::map<int, std::map<int, std::shared_ptr<FacilityGeometry>>> &
GeometryFactory::GetGeometry() const
{
    return _geometryFactory;
}

void GeometryFactory::AddElement(int room, int subroom, std::shared_ptr<FacilityGeometry> geo)
{
    _geometryFactory[room][subroom] = geo;
}

std::shared_ptr<FacilityGeometry> GeometryFactory::GetElement(int room, int subroom)
{
    if(_geometryFactory.count(room)) {
        if(_geometryFactory[room].count(subroom)) {
            return _geometryFactory[room][subroom];
        }
    }
    return nullptr;
}

void GeometryFactory::UpdateVisibility(int room, int subroom, bool status)
{
    if(_geometryFactory.count(room)) {
        if(_geometryFactory[room].count(subroom)) {
            _geometryFactory[room][subroom]->setVisibility(status);
        }
    }
}

QStandardItemModel & GeometryFactory::GetModel()
{
    return _model;
}
