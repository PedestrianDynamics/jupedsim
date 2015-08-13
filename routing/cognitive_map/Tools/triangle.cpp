#include "triangle.h"
#include <cmath>

Triangle::Triangle(Point p, const Waypoint &waypoint)
{
    _positionVector=p;
    std::tuple<Point,Point> bpoints = CalcBPoints(p, waypoint.GetPos(), waypoint.GetA(), waypoint.GetB());
//    Log->Write(std::to_string(std::get<0>(bpoints).GetX()));
//    Log->Write(std::to_string(std::get<0>(bpoints).GetY()));
//    Log->Write(std::to_string(std::get<1>(bpoints).GetX()));
//    Log->Write(std::to_string(std::get<1>(bpoints).GetY()));
//    Log->Write(std::to_string(p.GetX()));
//    Log->Write(std::to_string(p.GetY()));
    //Log->Write(std::to_string(std::get<1>(bpoints)));
    _vectorA=std::get<0>(bpoints)-p;
    _vectorB=std::get<1>(bpoints)-p;
}

Triangle::Triangle(Point p1, Point p2, Point p3)
{
    _positionVector=p1;
    _vectorA=p2-p1;
    _vectorB=p3-p1;

}

Triangle::~Triangle()
{

}

bool Triangle::Contains(const Point &point) const
{
//    point=Point(5.0,3.0);
//    _positionVector=Point(1,1);
//    _vectorA=Point(6,6);
//    _vectorB=Point(6,-1);
    //double r = (point.GetY()-_positionVector.GetY()-point.GetX()*_vectorB.GetY()/
      //          _vectorA.GetY()+_positionVector.GetX()*_vectorB.GetY()/_vectorA.GetY())
        //        /(_vectorB.GetX()-_vectorA.GetX()*_vectorB.GetY()/_vectorA.GetY());
    //Log->Write(std::to_string(s));
    double r = 0;
    double s = 0;

    if ((_vectorA.GetY()*_vectorB.GetX()-_vectorA.GetX()*_vectorB.GetY())!=0)
    {
        r = (-_vectorA.GetY()*_positionVector.GetY()+_vectorA.GetY()*point.GetY()+_vectorB.GetY()*_positionVector.GetX()
                    -_vectorB.GetY()*point.GetX())/(_vectorA.GetY()*_vectorB.GetX()-_vectorA.GetX()*_vectorB.GetY());
        s = (point.GetX()-_positionVector.GetX()-r*_vectorB.GetX())/_vectorA.GetX();
    }
    else
    {
        Log->Write("ERROR:\t Denominator is zero (in function triangle::contains)");
        return false;
    }


    if (s>=0 && r>=0)
    {
        return true;
    }
    else
        return false;
}

std::tuple<Point, Point> Triangle::CalcBPoints(const Point &p, const Point &pos, const double &ah, const double &bh)
{
    double px=p.GetX();
    double py=p.GetY();
    double pm=pos.GetX();
    double qm=pos.GetY();

    double alpha = (px-pm)/(ah*ah);
    double beta  = (py-qm)/(bh*bh);

    double a = ah*ah + bh*bh*beta*beta/(alpha*alpha);  // Koeff. f. quadrad. GL. in y
    double b = -2*beta*bh*bh/(alpha*alpha);
    double c = bh*bh/(alpha*alpha)-ah*bh*ah*bh;
    double diskrim = b*b-4*a*c;                        // Diskriminante der Wurzel

    float eps = 1e-10;
    float abd = std::abs(diskrim);



    if (ah>0 and bh>0)
    {
        if ( abd < eps)
            diskrim=0.0;


        if(diskrim>0)
        {
            //print "Es gibt zwei Tangenten"
            double wurz=std::sqrt(diskrim);
            double y1=(-b+wurz)/(2*a);
            double y2=(-b-wurz)/(2*a);
            double x1=(1-beta*y1)/alpha;
            double x2=(1-beta*y2)/alpha;
            y1=y1+qm;
            y2=y2+qm;
            x1=x1+pm;
            x2=x2+pm;
            if (y1 !=0)
            {
                double m1=-bh*bh*x1/(ah*ah*y1);
                double b1=y1-m1*x1;
            }
            if (y2 !=0)
            {
                double m2=-bh*bh*x2/(ah*ah*y2);
                double b2=y2-m2*x2;
            }
            Point p1(x1,y1);
            Point p2(x2,y2);
            //std::cout << "return!" << std::endl;
            return std::tuple<Point,Point>(p1,p2);
            //print "Beruehrpunkte: (x1,y1) = (",x1,",",y1,"), (x2,y2) = (",x2,",",y2,")"
        }

    }
    //std::cout << diskrim << std::endl;
}

