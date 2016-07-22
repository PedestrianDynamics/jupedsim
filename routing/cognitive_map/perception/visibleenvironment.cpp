#include "visibleenvironment.h"
#include "../../../visiLibity/source_code/visilibity.hpp"
#include "../../../geometry/Building.h"
#include "../../../pedestrian/Pedestrian.h"
#include "../../../geometry/SubRoom.h"

VisibleEnvironment::VisibleEnvironment()
{

}

VisibleEnvironment::VisibleEnvironment(const Building *b, const Pedestrian *ped)
{

//    //Creating complete spatial structure
//    _b=b;

//    std::map<int, std::unique_ptr<SubRoom> > subrooms;
//    std::vector<Wall> walls;
//    const std::map<int, std::unique_ptr<Room> > rooms= _b->GetAllRooms();

//    for (auto it=rooms.begin(); it!=rooms.end(); ++it)
//    {
//        for (auto it2:it->second->GetAllSubRooms())
//        {
//            //subrooms.emplace(it2.second);

//            //for (auto it2=subrooms.begin(); it!=subrooms.end(); ++it)
//            //{
//            for (Wall wall:it2.second->GetAllWalls())
//                walls.push_back(wall);
//            //}
//        }


//    }





//    //Boundary and holes
//    std::vector<VisiLibity::Polygon> polygons;
//    VisiLibity::Polygon boundary;
//    //Boundary of environment
//    boundary.push_back(VisiLibity::Point(-999999,-999999));
//    boundary.push_back(VisiLibity::Point(999999,-999999));
//    boundary.push_back(VisiLibity::Point(999999,999999));
//    boundary.push_back(VisiLibity::Point(-999999,999999));
//    polygons.push_back(boundary);

//    //Holes

//    for (Wall wall:walls)
//    {
//        //0.12 equals the half of a wall's width
//        polygons.push_back(VisiLibity::Polygon(std::vector<VisiLibity::Point>{
//        VisiLibity::Point(wall.GetPoint1()._x-0.12,wall.GetPoint1()._y-0.12),
//        VisiLibity::Point(wall.GetPoint1()._x-0.12,wall.GetPoint1()._y+0.12),
//        VisiLibity::Point(wall.GetPoint2()._x+0.12,wall.GetPoint2()._y+0.12),
//        VisiLibity::Point(wall.GetPoint2()._x+0.12,wall.GetPoint2()._y-0.12)}));
//    }

//    //set up the environment
//    VisiLibity::Environment environment(polygons);
//    //environment.reverse_holes();
//    if (!environment.is_valid())
//    {
//        Log->Write("Error:\tEnvironment for Visibilitypolygon not valid");
//        exit(EXIT_FAILURE);
//    }


}
