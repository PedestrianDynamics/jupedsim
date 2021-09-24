#ifndef GEOMETRYFACTORY_H
#define GEOMETRYFACTORY_H

#include "FacilityGeometry.h"

#include <QStandardItemModel>
#include <iostream>
#include <map>
#include <memory>

// forward classes
class FacilityGeometry;
class vtkRenderer;

class GeometryFactory
{
public:
    GeometryFactory();

    void Init(vtkRenderer * renderer);
    void ChangeWallsColor(double * color);
    void ChangeExitsColor(double * color);
    void ChangeNavLinesColor(double * color);
    void ChangeFloorColor(double * color);
    void ChangeObstaclesColor(double * color);
    void Set2D(bool status);
    void Set3D(bool status);
    void ShowDoors(bool status);
    void ShowStairs(bool status);
    void ShowWalls(bool status);
    void ShowNavLines(bool status);
    void ShowFloor(bool status);
    void ShowObstacles(bool status);
    void ShowGradientField(bool status);
    void ShowGeometryLabels(int status);
    bool RefreshView();
    void Clear();

    const std::map<int, std::map<int, std::shared_ptr<FacilityGeometry>>> & GetGeometry() const;
    void AddElement(int room, int subroom, std::shared_ptr<FacilityGeometry> geo);
    std::shared_ptr<FacilityGeometry> GetElement(int room, int subroom);
    void UpdateVisibility(int room, int subroom, bool status);
    QStandardItemModel & GetModel();

private:
    // map a room,subroom id to a geometry element
    std::map<int, std::map<int, std::shared_ptr<FacilityGeometry>>> _geometryFactory;
    QStandardItemModel _model;
};

#endif // GEOMETRYFACTORY_H
