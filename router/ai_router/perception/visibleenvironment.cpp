#include "visibleenvironment.h"

#include "geometry/Building.h"
#include "pedestrian/Pedestrian.h"
#include "geometry/SubRoom.h"
#include "geometry/Wall.h"

#include <boost/foreach.hpp>

VisibleEnvironment::VisibleEnvironment():_b(nullptr),_cgalGeometry(CGALGeometry())
{

}

VisibleEnvironment::VisibleEnvironment(const Building *b):_b(b)
{

    Log->Write("INFO: PERCEPTION: Preparing visible environment ...");


    //Creating complete spatial structure

    std::vector<Linestring> lines;
    std::unordered_map<ptrFloor,std::vector<const Line*>> cWalls;
    std::vector<const Line*> wallsOfOneRoom;

    //get walls of rooms and save them separately
    for (auto it = _b->GetAllRooms().begin(); it != _b->GetAllRooms().end(); ++it)
    {
        lines.clear();
        wallsOfOneRoom.clear();
        for (auto it2 = it->second->GetAllSubRooms().begin(); it2 != it->second->GetAllSubRooms().end(); ++it2)
        {
            for (const Wall& wall:it2->second->GetAllWalls())
            {

                Linestring lineS;
                boost::geometry::append(lineS, wall.GetPoint1());
                boost::geometry::append(lineS, wall.GetPoint2());
                if (std::find(lines.begin(), lines.end(), lineS) == lines.end())
                {
                    lines.push_back(lineS);
                    wallsOfOneRoom.push_back(&wall);
                }


            }

            for (const Obstacle *obstacle:it2->second->GetAllObstacles())
            {
                for (const Wall& wall: obstacle->GetAllWalls())
                {
                    Linestring lineS;
                    boost::geometry::append(lineS, wall.GetPoint1());
                    boost::geometry::append(lineS, wall.GetPoint2());
                    if (std::find(lines.begin(), lines.end(), lineS) == lines.end())
                    {
                        lines.push_back(lineS);
                        wallsOfOneRoom.push_back(&wall);

                    }

                }

            }

        }

        if (it->second->GetID()==6 || it->second->GetID()==0 || it->second->GetID()==11)
        {

            for (int transID:it->second->GetAllTransitionsIDs())
            {
                //if ((it->second->GetID()==6 || it->second->GetID()==0))// && _b->GetTransitionByUID(transID)->GetOtherRoom(6)->GetID()==5) || (it->second->GetID()==11 && _b->GetTransitionByUID(transID)->GetOtherRoom(11)!=nullptr
                                                                                                                 //&& (_b->GetTransitionByUID(transID)->GetOtherRoom(11)->GetID()==9 ||
                                                                                                                  //   _b->GetTransitionByUID(transID)->GetOtherRoom(11)->GetID()==10)))
                //{
                    // shift transition a little bit to the outside
                    bool within=false;
                    Point normal = Point(-(_b->GetTransitionByUID(transID)->GetPoint2()-_b->GetTransitionByUID(transID)->GetPoint1())._y,
                                         (_b->GetTransitionByUID(transID)->GetPoint2()-_b->GetTransitionByUID(transID)->GetPoint1())._x);
                    normal=normal/normal.Norm();
                    Point P1 = _b->GetTransitionByUID(transID)->GetCentre()+normal*0.5;


                    for (auto it2 = it->second->GetAllSubRooms().begin(); it2 != it->second->GetAllSubRooms().end(); ++it2)
                    {
                        BoostPolygon boostP;
                        for (Point vertex:it2->second->GetPolygon())
                        {
                            boost::geometry::append(boostP, vertex);
                        }
                        boost::geometry::correct(boostP);

                        if (boost::geometry::within(P1, boostP))
                            within=true;

                    }
                    // MEMORY LEAK!!! FIX ME!!!

                    if (within==true)
                    {
                        Linestring lineS;
                        boost::geometry::append(lineS,_b->GetTransitionByUID(transID)->GetPoint1()-normal*0.5);
                        boost::geometry::append(lineS,_b->GetTransitionByUID(transID)->GetPoint2()-normal*0.5);
                        lines.push_back(lineS);
                        Line* newLine = new Line(_b->GetTransitionByUID(transID)->GetPoint1()-normal*0.5,_b->GetTransitionByUID(transID)->GetPoint2()-normal*0.5);
                        wallsOfOneRoom.push_back(newLine);

                        Linestring lineS1;
                        boost::geometry::append(lineS1,_b->GetTransitionByUID(transID)->GetPoint1()-normal*0.5);
                        boost::geometry::append(lineS1,_b->GetTransitionByUID(transID)->GetPoint1());
                        lines.push_back(lineS1);
                        newLine = new Line(_b->GetTransitionByUID(transID)->GetPoint1()-normal*0.5,_b->GetTransitionByUID(transID)->GetPoint1());
                        wallsOfOneRoom.push_back(newLine);

                        Linestring lineS2;
                        boost::geometry::append(lineS2,_b->GetTransitionByUID(transID)->GetPoint2()-normal*0.5);
                        boost::geometry::append(lineS2,_b->GetTransitionByUID(transID)->GetPoint2());
                        lines.push_back(lineS2);
                        newLine = new Line(_b->GetTransitionByUID(transID)->GetPoint2()-normal*0.5,_b->GetTransitionByUID(transID)->GetPoint2());
                        wallsOfOneRoom.push_back(newLine);
                    }
                    else
                    {
                        Linestring lineS;
                        boost::geometry::append(lineS,_b->GetTransitionByUID(transID)->GetPoint1()+normal*0.5);
                        boost::geometry::append(lineS,_b->GetTransitionByUID(transID)->GetPoint2()+normal*0.5);
                        lines.push_back(lineS);
                        Line* newLine = new Line(_b->GetTransitionByUID(transID)->GetPoint1()+normal*0.5,_b->GetTransitionByUID(transID)->GetPoint2()+normal*0.5);
                        wallsOfOneRoom.push_back(newLine);

                        Linestring lineS1;
                        boost::geometry::append(lineS1,_b->GetTransitionByUID(transID)->GetPoint1()+normal*0.5);
                        boost::geometry::append(lineS1,_b->GetTransitionByUID(transID)->GetPoint1());
                        lines.push_back(lineS1);
                        newLine = new Line(_b->GetTransitionByUID(transID)->GetPoint1()+normal*0.5,_b->GetTransitionByUID(transID)->GetPoint1());
                        wallsOfOneRoom.push_back(newLine);

                        Linestring lineS2;
                        boost::geometry::append(lineS2,_b->GetTransitionByUID(transID)->GetPoint2()+normal*0.5);
                        boost::geometry::append(lineS2,_b->GetTransitionByUID(transID)->GetPoint2());
                        lines.push_back(lineS2);
                        newLine = new Line(_b->GetTransitionByUID(transID)->GetPoint2()+normal*0.5,_b->GetTransitionByUID(transID)->GetPoint2());
                        wallsOfOneRoom.push_back(newLine);
                    }
                }
            //lines.push_back(lineS);

            //}

        }



        _allWalls.emplace(it->second.get(),lines);
        cWalls.emplace(it->second.get(),wallsOfOneRoom);

    }


    _cgalGeometry=CGALGeometry(cWalls);


    WriteOutWalls();

    Log->Write("INFO: PERCEPTION: Visible environment prepared.");

}

//const VisiLibity::Environment* VisibleEnvironment::GetVisEnv() const
//{
//    return &_env;
//}



const std::vector<Linestring> &VisibleEnvironment::GetAllWallsOfEnv(ptrFloor floor) const
{
    return _allWalls.at(floor);
}

void VisibleEnvironment::WriteOutWalls() const
{

    std::ofstream myfile;
    std::string str = "./isovists/walls.txt";
    myfile.open (str);

    for (auto it=_allWalls.begin(); it!=_allWalls.end(); ++it)
    {
        const std::vector<Linestring>& walls=it->second;
        for (Linestring wall:walls)
        {
            for (auto it2=boost::begin(wall);it2!=boost::end(wall);++it2)
                myfile << std::to_string(it2->_x) << " " << std::to_string(it2->_y) << " ";

            myfile << std::endl;

        }

    }

    myfile.close();
}

void VisibleEnvironment::SetSigns(ptrFloor floor, const std::vector<Sign> &signs) {

    _signs.clear();
    _signs.emplace(std::make_pair(floor,signs));
}

const CGALGeometry &VisibleEnvironment::GetCGALGeometry() const
{
    return _cgalGeometry;
}

void VisibleEnvironment::AddSign(ptrFloor floor, const Sign &sign)
{
    _signs[floor].push_back(sign);
}

void VisibleEnvironment::AddSign(ptrFloor floor, Sign &&sign)
{
    _signs[floor].push_back(std::move(sign));
}

const std::vector<Sign> *VisibleEnvironment::GetSignsOfFloor(ptrFloor floor) const
{
    if (_signs.find(floor)==_signs.end())
        return nullptr;
    else
        return &_signs.at(floor);
}
