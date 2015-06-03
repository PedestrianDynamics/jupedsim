#include "triangle.h"

#include <tuple>
#include <math.h>

Triangle::Triangle(Point p, const Waypoint &waypoint)
{
    _positionVector=p;
    std::tuple<Point,Point> bpoints = CalcBPoints(p, waypoint.GetPos(), waypoint.GetA(), waypoint.GetB());
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

bool Triangle::Contains(const Point& point) const
{
    double s = (point.GetY()-_positionVector.GetY()-(point.GetX()-_positionVector.GetX())/_vectorA.GetX())
                                                    /(_vectorB.GetY()-_vectorB.GetX()/_vectorA.GetX());
    double r = (point.GetX()-_positionVector.GetX()-s*_vectorB.GetX())/_vectorA.GetX();

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
    double qm=pos.GetX();
    double pm=pos.GetY();


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
            return std::tuple<Point,Point>(p1,p2);
            //print "Beruehrpunkte: (x1,y1) = (",x1,",",y1,"), (x2,y2) = (",x2,",",y2,")"
        }
    }
}

