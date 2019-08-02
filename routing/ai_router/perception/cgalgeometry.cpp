#include "cgalgeometry.h"

#include "geometry/SubRoom.h"
#include "geometry/Wall.h"

#include <iostream>

CGALGeometry::CGALGeometry()
{

}

CGALGeometry::CGALGeometry(const std::unordered_map<ptrFloor, std::vector<const Line*> > &allWalls)
{

    // for every room (floor) separately:

    for (auto it=allWalls.begin(); it!=allWalls.end(); ++it)
    {
        //std::cout << it->first->GetCaption() << std::endl;
        /// if geometry is not stored in advance (begin)
        ///
        const std::vector<const Line*>& walls=it->second;

        Polygon_with_holes_2 unionR;

        int n = 0;

        std::vector<Polygon_2> separatedPolygons;


        Point dirVector = walls[0]->GetPoint2()- walls[0]->GetPoint1();
        dirVector = dirVector / dirVector.Norm();
        Point normalVector = Point(-dirVector._y, dirVector._x);
        Point P1 = walls[0]->GetPoint1();
        Point P2 = walls[0]->GetPoint2();
        Point Ps1 = P1 - normalVector * 0.12 - dirVector * 0.12;
        Point Ps2 = P2 - normalVector * 0.12 + dirVector * 0.12;
        Point Ps3 = P2 + normalVector * 0.12 + dirVector * 0.12;
        Point Ps4 = P1 + normalVector * 0.12 - dirVector * 0.12;

        unionR.outer_boundary().push_back(Point_2(Ps1._x, Ps1._y));
        unionR.outer_boundary().push_back(Point_2(Ps2._x, Ps2._y));
        unionR.outer_boundary().push_back(Point_2(Ps3._x, Ps3._y));
        unionR.outer_boundary().push_back(Point_2(Ps4._x, Ps4._y));


        Polygon_2 outer;


        for (const Line* wall:walls)
        {
            if (n < 1)
            {
                n++;
                continue;
            }
            dirVector = wall->GetPoint2()-wall->GetPoint1();//wall.at(1) - wall.at(0);
            dirVector = dirVector / dirVector.Norm();
            normalVector = Point(-dirVector._y, dirVector._x);
            //0.12 equals the half of a wall's width
            //BoostPolygon boostHole;
            Polygon_2 cgalPolygon;

            Point PP1 = wall->GetPoint1();
            Point PP2 = wall->GetPoint2();
            Point Pi1 = PP1 - normalVector * 0.12 - dirVector * 0.12;
            Point Pi2 = PP2 - normalVector * 0.12 + dirVector * 0.12;
            Point Pi3 = PP2 + normalVector * 0.12 + dirVector * 0.12;
            Point Pi4 = PP1 + normalVector * 0.12 - dirVector * 0.12;

            cgalPolygon.push_back(Point_2(Pi1._x, Pi1._y));
            cgalPolygon.push_back(Point_2(Pi2._x, Pi2._y));
            cgalPolygon.push_back(Point_2(Pi3._x, Pi3._y));
            cgalPolygon.push_back(Point_2(Pi4._x, Pi4._y));

            if (!CGAL::join(unionR, cgalPolygon, unionR))
            {
                separatedPolygons.push_back(cgalPolygon);
                Log->Write(std::to_string(n)+" of " + std::to_string(walls.size()));
            }

            //outer = unionR.outer_boundary();

            //try to join separated polygons
            std::vector<Polygon_2>::iterator p = separatedPolygons.begin();
            while (p != separatedPolygons.end()) {
                if (CGAL::join(unionR, *p, unionR)) {
                    p = separatedPolygons.erase(p);
                } else
                    ++p;
            }
            ++n;
        }
        std::vector<Polygon_with_holes_2> listPolygons;
        //listPolygons.push_back(unionR);

        size_t num_polygons;
        while (!separatedPolygons.empty()) {
            num_polygons = separatedPolygons.size();
            std::vector<Polygon_2>::iterator p = separatedPolygons.begin();
            while (p != separatedPolygons.end()) {
                if (CGAL::join(unionR, *p, unionR)) {
                    p = separatedPolygons.erase(p);
                } else
                    ++p;
            }
            if (num_polygons == separatedPolygons.size()) {

                listPolygons.push_back(unionR);
                unionR=Polygon_with_holes_2(separatedPolygons.front());
            }
            Log->Write(std::to_string(separatedPolygons.size())+" of " + std::to_string(walls.size()));
        }

        listPolygons.push_back(unionR);

        std::vector<Segment_2> segments;

        Point_2 pb1=Point_2(it->first->GetBoundaryVertices()[0]._x-1,it->first->GetBoundaryVertices()[0]._y-1);
        Point_2 pb2=Point_2(it->first->GetBoundaryVertices()[3]._x+1,it->first->GetBoundaryVertices()[3]._y-1);
        Point_2 pb3=Point_2(it->first->GetBoundaryVertices()[2]._x+1,it->first->GetBoundaryVertices()[2]._y+1);
        Point_2 pb4=Point_2(it->first->GetBoundaryVertices()[1]._x-1,it->first->GetBoundaryVertices()[1]._y+1);
//        std::cout << pb1.x() << "   " << pb1.y() << std::endl;
//        std::cout << pb2.x() << "   " << pb2.y() << std::endl;
//        std::cout << pb3.x() << "   " << pb3.y() << std::endl;
//        std::cout << pb4.x() << "   " << pb4.y() << std::endl;
        segments.push_back(Segment_2(pb1,pb2));
        segments.push_back(Segment_2(pb2,pb3));
        segments.push_back(Segment_2(pb3,pb4));
        segments.push_back(Segment_2(pb4,pb1));

        std::cout << "Room" << it->first->GetID() << std::endl;
        for (Polygon_with_holes_2 poly:listPolygons)
        {
            outer = poly.outer_boundary();


//            Point veryFirstP=Point(roundfloat(CGAL::to_double(outer[0].x()),1),roundfloat(CGAL::to_double(outer[0].y()),1));
//            Point lastPoint = veryFirstP;

//            std::cout << "Polygon" << std::endl;
//            std::cout << lastPoint._x << "   " << lastPoint._y << std::endl;

            size_t i = 1;
            for (; i < outer.size(); ++i)
            {
//                Point newPoint = Point(roundfloat(CGAL::to_double(outer[i].x()),1),roundfloat(CGAL::to_double(outer[i].y()),1));
//                if (newPoint!=lastPoint && newPoint!=veryFirstP)
//                {
//                    segments.push_back(Segment_2(Point_2(lastPoint._x,lastPoint._y), Point_2(newPoint._x,newPoint._y)));//-1].y()),Point_2(outer[i].x(),outer[i].y())));
//                    lastPoint=newPoint;
//                    std::cout << newPoint._x << "   " << newPoint._y << std::endl;/
                segments.push_back(Segment_2(outer[i-1],outer[i]));
//                }
            }
            segments.push_back(Segment_2(outer[i-1],outer[0]));
//            Point_2 CGALlastP = Point_2(lastPoint._x,lastPoint._y);
//            Point_2 CGALFirstP = Point_2(veryFirstP._x,veryFirstP._y);
//            segments.push_back(Segment_2(CGALlastP,CGALFirstP));


        }


        /// if geometry is not stored in advance (end)
        ///
        /// if geometry is stored in file in advance

//        std::vector<Segment_2> segments;


//        Point_2 pb1=Point_2(it->first->GetBoundaryVertices()[0]._x-1,it->first->GetBoundaryVertices()[0]._y-1);
//        Point_2 pb2=Point_2(it->first->GetBoundaryVertices()[3]._x+1,it->first->GetBoundaryVertices()[3]._y-1);
//        Point_2 pb3=Point_2(it->first->GetBoundaryVertices()[2]._x+1,it->first->GetBoundaryVertices()[2]._y+1);
//        Point_2 pb4=Point_2(it->first->GetBoundaryVertices()[1]._x-1,it->first->GetBoundaryVertices()[1]._y+1);
//        segments.push_back(Segment_2(pb1,pb2));
//        segments.push_back(Segment_2(pb2,pb3));
//        segments.push_back(Segment_2(pb3,pb4));
//        segments.push_back(Segment_2(pb4,pb1));


//        std::string line;
//        std::string filename = "D:/Dokumente/jpstests/osloerstr/polygons_id"+std::to_string(it->first->GetID())+".txt";
//        std::cout << filename << std::endl;
//        std::ifstream myfile (filename);
//        std::vector<std::vector<Point_2>> polyVertices;
//        Point oldPoint=Point(FLT_MAX,FLT_MAX);
//        if (myfile.is_open())
//        {
//            while ( std::getline (myfile,line) )
//            {
//                if (line=="Polygon")
//                {
//                  polyVertices.push_back(std::vector<Point_2>{});
//                }
//                else
//                {
//                  std::stringstream linestream(line);
//                  //std::string data;
//                  //std::getline(linestream, data, ';');
//                  double x;
//                  double y;
//                  linestream >> x >> y;
//                  Point point = Point(x,y);
//                  Point vec = oldPoint-point;
//                  if (vec.Norm()>=0.1)
//                  {
//                    polyVertices.back().push_back(Point_2(point._x,point._y));
//                    //Log->Write(std::to_string(it->first->GetID()));
//                //Log->Write(std::to_string(point._x)+"   "+std::to_string(point._y));
//                    oldPoint=point;
//                  }
//                }
//            }
//            myfile.close();
//        }

//        else
//            Log->Write("ERROR \t Unable to open file");


//        for (std::vector<Point_2> vector:polyVertices)
//        {
//            size_t i = 1;
//            for (; i < vector.size(); ++i)
//            {
//                segments.push_back(Segment_2(vector[i - 1], vector[i]));
//            }
//            segments.push_back(Segment_2(vector.back(), vector.front()));
//        }
        /// end if



        // insert geometry into the arrangement

        Arrangement_2 env;
        CGAL::insert_non_intersecting_curves(env, segments.begin(), segments.end());

        //associate room with visible env (walls) of that room
        _CGAL_env.emplace(it->first,env);

        //std::shared_ptr<TEV> tev=std::make_shared<TEV>(env);

        //associate room with TEV
        _tev.emplace(it->first,std::unique_ptr<TEV>(new TEV(env)));


    }

}




const TEV *CGALGeometry::GetTEV(ptrFloor floor) const
{
    return _tev.at(floor).get();
}

const Arrangement_2 &CGALGeometry::GetVisEnvCGAL(ptrFloor floor) const
{
    return _CGAL_env.at(floor);
}


float roundfloat(float num, int precision)
{
    return floorf(num * pow(10.0f,precision) + .5f)/pow(10.0f,precision);
}
