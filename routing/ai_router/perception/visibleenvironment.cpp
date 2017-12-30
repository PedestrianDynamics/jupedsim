#include "visibleenvironment.h"
#include "../../../visiLibity/source_code/visilibity.hpp"
#include "../../../geometry/Building.h"
#include "../../../pedestrian/Pedestrian.h"
#include "../../../geometry/SubRoom.h"

//#include <boost/foreach.hpp>
#define UNUSED(x) [&x]{}()  // c++11 silence warnings

VisibleEnvironment::VisibleEnvironment()
{

}

VisibleEnvironment::VisibleEnvironment(const Building *b, const Pedestrian *ped)
{
    _ped=ped;
    typedef boost::geometry::model::polygon<Point> BoostPolygon;

    std::vector<BoostPolygon> boostHoles;


    //Creating complete spatial structure
    _b=b;
    std::vector<Wall> walls;

    //get all walls
    for (auto it=_b->GetAllRooms().begin(); it!=_b->GetAllRooms().end(); ++it)
    {
        for (auto it2=it->second->GetAllSubRooms().begin(); it2!=it->second->GetAllSubRooms().end(); ++it2)
        {
            for (Wall wall:it2->second->GetAllWalls())
                walls.push_back(wall);
        }

    }


    for (Wall wall:walls)
    {
        Point dirVector=wall.GetPoint2()-wall.GetPoint1();
        dirVector=dirVector/dirVector.Norm();
        Point normalVector = Point(-dirVector._y,dirVector._x);
        //0.12 equals the half of a wall's width
        BoostPolygon boostHole;
        if (wall.GetPoint1()._x <= wall.GetPoint2()._x)
        {
            boost::geometry::append(boostHole,wall.GetPoint1()-normalVector*0.00012);
            boost::geometry::append(boostHole,wall.GetPoint1()+normalVector*0.00012);
            boost::geometry::append(boostHole,wall.GetPoint2()+normalVector*0.00012);
            boost::geometry::append(boostHole,wall.GetPoint2()-normalVector*0.00012);
            boost::geometry::correct(boostHole);
        }
        else
        {
            boost::geometry::append(boostHole,wall.GetPoint2()+normalVector*0.00012);
            boost::geometry::append(boostHole,wall.GetPoint2()-normalVector*0.00012);
            boost::geometry::append(boostHole,wall.GetPoint1()-normalVector*0.00012);
            boost::geometry::append(boostHole,wall.GetPoint1()+normalVector*0.00012);
            boost::geometry::correct(boostHole);
        }
        boostHoles.push_back(boostHole);
    }


//    std::vector<Point> const& points = boostHoles[0].outer();
//    for (std::vector<Point>::size_type i = 0; i < points.size(); ++i)
//    {
//        Log->Write(std::to_string(points[i]._x));
//        Log->Write(std::to_string(points[i]._y));
//    }

//    std::vector<Point> const& points2 = boostHoles[1].outer();
//    for (std::vector<Point>::size_type i = 0; i < points2.size(); ++i)
//    {
//        Log->Write(std::to_string(points2[i]._x));
//        Log->Write(std::to_string(points2[i]._y));
//    }


    std::vector<BoostPolygon> boostHolesCorrected;
    std::list<BoostPolygon> output;
    bool statusIdentical;
    Log->Write("INFO: PERCEPTION: Preparing visible environment ...");
    for (BoostPolygon hole:boostHoles)
    {
        statusIdentical=false;

        for (BoostPolygon correctedhole:boostHolesCorrected)
        {

            if (boost::geometry::equals(hole,correctedhole))
            {
                // if two walls are identical
                statusIdentical=true;
                break;
            }

//            boost::geometry::difference(hole,correctedhole,output);  //what if output.size is greater than one?
//            if (output.empty())
//            {
//                // if walls one lies in wall two
//                statusIdentical=true;
//                break;
//            }
            //hole=output.front();
            //output.clear();
        }
        if (!statusIdentical)
            boostHolesCorrected.push_back(hole);
    }



    //Boundary and holes
    std::vector<VisiLibity::Polygon> polygons;
    VisiLibity::Polygon boundary;
    //Boundary of environment
    boundary.push_back(VisiLibity::Point(b->GetBoundaryVertices()[0]._x-5,b->GetBoundaryVertices()[0]._y-5));
    boundary.push_back(VisiLibity::Point(b->GetBoundaryVertices()[3]._x+5,b->GetBoundaryVertices()[3]._y-5));
    boundary.push_back(VisiLibity::Point(b->GetBoundaryVertices()[2]._x+5,b->GetBoundaryVertices()[2]._y+5));
    boundary.push_back(VisiLibity::Point(b->GetBoundaryVertices()[1]._x-5,b->GetBoundaryVertices()[1]._y+5));
    polygons.push_back(boundary);

    //Holes

    for (BoostPolygon correctedhole:boostHolesCorrected)
    {
        VisiLibity::Polygon VisiPolygon;
        std::vector<Point> const& points = correctedhole.outer();

        Point dirVector2= (points[3]+points[2])/2-(points[1]+points[0])/2;
        dirVector2=dirVector2/dirVector2.Norm();

        for (std::vector<Point>::size_type i = 0; i < points.size()-1; ++i)
        {
            //VisiPolygon.push_back(VisiLibity::Point(points[i]._x,points[i]._y));
            if (i<2)
                VisiPolygon.push_back(VisiLibity::Point(points[i]._x+dirVector2._x*0.0012,points[i]._y+dirVector2._y*0.0012));
            else
                VisiPolygon.push_back(VisiLibity::Point(points[i]._x-dirVector2._x*0.0012,points[i]._y-dirVector2._y*0.0012));
        }
        polygons.push_back(VisiPolygon);
    }



//    for (Wall wall:walls)
//    {
//        //0.12 equals the half of a wall's width
//        polygons.push_back(VisiLibity::Polygon(std::vector<VisiLibity::Point>{
//        VisiLibity::Point(wall.GetPoint1()._x-0.12,wall.GetPoint1()._y-0.12),
//        VisiLibity::Point(wall.GetPoint1()._x-0.12,wall.GetPoint1()._y+0.12),
//        VisiLibity::Point(wall.GetPoint2()._x+0.12,wall.GetPoint2()._y+0.12),
//        VisiLibity::Point(wall.GetPoint2()._x+0.12,wall.GetPoint2()._y-0.12)}));
//    }

    //set up the environment
    VisiLibity::Environment environment(polygons);
    //environment.reverse_holes();
    if (!environment.is_valid())
    {
        Log->Write("ERROR:\tEnvironment for Visibilitypolygon not valid. \n");
        exit(EXIT_FAILURE);
    }

    //VisiLibity::Guards my_guards(std::vector(Point(2095.53,14423.4)));

    VisiLibity::Visibility_Polygon
      my_visibility_polygon(VisiLibity::Point(4.0,3.5), environment,0.01);
    std::cout << "The visibility polygon is \n" << my_visibility_polygon << std::endl;


    //my_visibility_polygon.

    Log->Write("INFO: PERCEPTION: Visible environment prepared.");

}
