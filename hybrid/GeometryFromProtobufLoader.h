//
// Created by Gregor Lämmel on 30/03/16.
//

#ifndef JPSCORE_GEOMETRYFROMPROTOBUFLOADER_H
#define JPSCORE_GEOMETRYFROMPROTOBUFLOADER_H

#include "../geometry/GeometryReader.h"
#include "../general/Configuration.h"

class GeometryFromProtobufLoader : public GeometryReader {
public:
     GeometryFromProtobufLoader(const Configuration* configuration);
     virtual ~GeometryFromProtobufLoader(){};

     virtual void LoadBuilding(Building* building) override;

     virtual bool LoadTrafficInfo(Building* building) override;

private:
     const Configuration* _configuration;

     bool LoadGeometry(Building* building);

     bool LoadRoutingInfo(Building* building);
};

#endif //JPSCORE_GEOMETRYFROMPROTOBUFLOADER_H
