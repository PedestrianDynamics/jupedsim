#pragma once

#include "sign.h"
#include "cgalgeometry.h"

#include <boost/geometry/geometry.hpp>

// Define the used kernel and arrangement

typedef boost::geometry::model::linestring<Point> Linestring;
typedef boost::geometry::model::polygon<Point> BoostPolygon;

class VisibleEnvironment
{
public:
    explicit VisibleEnvironment();
    explicit VisibleEnvironment(const Building* b);
    //const VisiLibity::Environment *GetVisEnv() const;
    const std::vector<Linestring>& GetAllWallsOfEnv(ptrFloor floor) const;
    void WriteOutWalls() const;
    void SetSigns(ptrFloor floor, const std::vector<Sign> &signs);
    const CGALGeometry& GetCGALGeometry() const;
    void AddSign(ptrFloor floor, const Sign& sign);
    void AddSign(ptrFloor floor, Sign&& sign);
    const std::vector<Sign> *GetSignsOfFloor(ptrFloor floor) const;


private:
    const Building* _b;
    //VisiLibity::Environment _env;
    // int: room_id: LinestringVector: Walls in room with id=room_id
    std::unordered_map<ptrFloor,std::vector<Linestring>> _allWalls;
    //std::vector<Sign> _signs;
    std::unordered_map<ptrFloor, std::vector<Sign>> _signs;
    CGALGeometry _cgalGeometry;
    std::vector<Line> _additionalLines;

};
