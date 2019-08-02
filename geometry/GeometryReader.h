//
// Created by Gregor Lämmel on 30/03/16.
//

#pragma once

class Building;

class GeometryReader {
public:
     virtual void LoadBuilding(Building* building) = 0;

     virtual bool LoadTrafficInfo(Building* building) = 0;
     virtual ~GeometryReader(){};
};
