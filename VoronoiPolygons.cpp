#include "VoronoiDiagramGenerator.h"
#include "VoronoiPolygons.h"
#include <algorithm>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <list>
#include <vector>
#include <math.h>


struct PtsSort
{
   point_2d pt;
   double angle;
};
bool myclockwise(const   PtsSort&   s1,const   PtsSort&   s2)
{

  return   s1.angle   >   s2.angle;

}
double angleSubtendedBy(point_2d point,point_2d about);
bool compareByAngle(point_2d p1, point_2d p2, point_2d about);
model::segment<point_2d> forceSegmentClockwise(model::segment<point_2d> s1, point_2d about);
bool segIntersects(model::segment<point_2d> *s1, model::segment<point_2d> s2);



std::vector<polygon_2d>  getVoronoiPolygons(double* xs, double* ys, int Npoints, double xmin, double xmax, double ymin, double ymax)
{
   VoronoiDiagramGenerator vd;
   double x1,y1,x2,y2;

   std::vector<model::segment<point_2d>* > edges;              // Edges of the Voronoi diagram.
   std::vector<model::segment<point_2d>* > boundaries;         // Boundaries of the diagram.
   std::vector<point_2d> intersections;                 //Intersections of the Voronoi diagram with the bounding box
   //std::vector<model::segment<point_2d>* > intersecting_edges;
   std::vector<model::segment<point_2d > *>::iterator edge_iterator;

   std::vector<polygon_2d> polygons;
   std::vector<model::segment<point_2d> *> thispolygon;

   /*
      Build up the boundary segments, as the Voronoi 
   */

   point_2d bl(xmin,ymin);
   point_2d br(xmax,ymin);
   point_2d tl(xmin,ymax);
   point_2d tr(xmax,ymax);

   model::segment<point_2d> * e1 = new model::segment<point_2d>(bl,br);
   model::segment<point_2d> * e2 = new model::segment<point_2d>(br,tr);
   model::segment<point_2d> * e3 = new model::segment<point_2d>(tr,tl);
   model::segment<point_2d> * e4 = new model::segment<point_2d>(tl,bl);

   boundaries.push_back(e1);
   boundaries.push_back(e2);
   boundaries.push_back(e3);
   boundaries.push_back(e4);

   /*
      Build up the Voronoi diagram segments.
   */
   vd.generateVoronoi(xs,ys,Npoints,xmin,xmax,ymin,ymax);
   vd.resetIterator();

   while(vd.getNext(x1,y1,x2,y2))
   {
	  //printf("GOT Line (%f,%f)->(%f,%f)\n",x1,y1,x2, y2);
	  point_2d *pt1 = new point_2d(x1,y1);
      point_2d *pt2 = new point_2d(x2,y2);

//      segment<point_2d > * ls = new segment<point_2d >( make<point_2d>(20.0, 0.0) ,*pt2);
      model::segment<point_2d> * ls = new model::segment<point_2d >(*pt1,*pt2);
      //There are some strange degenerate points coming out of the Voronoi diagram code
      if (length(*ls) != 0. ) edges.push_back(ls);
   }

   if(Npoints==2)
   {
	   edges.pop_back();
   }

   /*
      These segments don't include a the given boundary, they need to be included.
      1) Locate all the intersections with the boundary and the edges.
      2) Include all the boundary segment points.
      3) Sort them clockwise order
      4) Created another set, rotated by one position.  These are the start and finish points of the new segments.
      5) Create segments and append to edges.
   */

   // 1)
   BOOST_FOREACH(model::segment<point_2d> *edge_segment,edges)
   {
      BOOST_FOREACH(model::segment<point_2d> *boundary_segment,boundaries)
      {
         //intersection_inserter<point_2d>(*boundary_segment, *edge_segment,back_inserter(intersections) );
    	  std::deque<point_2d> temp_intersection;
    	  intersection(*boundary_segment, *edge_segment, temp_intersection );
    	  if(temp_intersection.size()==1)
    	  {
    		  intersections.push_back(temp_intersection[0]);
    	  }
      }
   }

   // 2)
   BOOST_FOREACH(model::segment<point_2d> *boundary_segment,boundaries) {intersections.push_back( (*boundary_segment).first );}
   // 3)
   point_2d midpoint( (xmin+xmax)/2,(ymin+ymax)/2 );

   //******************************************************************************
   PtsSort *intersects;
   intersects = new PtsSort[intersections.size()];
   std::vector<PtsSort>   intersecpt;
   for (int i=0;i<(int)intersections.size();i++)
   {
	   intersects[i].pt= intersections[i];
	   intersects[i].angle=angleSubtendedBy(intersections[i],midpoint);
	   intersecpt.push_back(intersects[i]);
   }
   sort(intersecpt.begin(), intersecpt.end(),myclockwise);
   for (int i=0;i<(int)intersections.size();i++)
   {
	   intersections[i]=intersecpt[i].pt;
   }
   delete intersects;
   //******************************************************************************
//   sort( intersections.begin(), intersections.end(), bind(&compareByAngle,_1,_2,midpoint) );
   // 4)
   std::vector<point_2d> intersections_r(intersections.size() );
   copy(intersections.begin(),intersections.end(),intersections_r.begin() );
   rotate(intersections_r.begin(),intersections_r.begin()+1,intersections_r.end() );

   // 5)
   for(unsigned int i=0;i<intersections.size();i++)
      {
	   	 model::segment<point_2d > *ls = new model::segment<point_2d >(intersections[i],intersections_r[i] );
         edges.push_back(ls);
      }

   /*
      We have all the edges stored in 'edges', including the boundary.
      Now we need to to associate these edges with each point, in order to build the polygons.

      If a line from the midpoint of an edge to the point crosses other lines, then it cannot be part of that cell.

      1) Calculate all the midpoints of the edges.
      2) For each point:
		    Find each edge:
				3) Find a segment from midpoint of edge to point.
			    4) Count number of intersections with other edges.  If it is only 1. i.e. it intersects itself.  Then it is an edge.
      5) Force these edges to point clockwise and take the first one of each segment.  You know have all the points of the polygon
      6) Sort these points clockwise from the middle
      7) Make a polygon from these points.
   */

   //1) 
   std::vector<point_2d> midpoints;
   BOOST_FOREACH (model::segment<point_2d> *edge_segment,edges)
   {
      point_2d midpoint = (*edge_segment).first;
      add_point(midpoint,(*edge_segment).second);
      divide_value(midpoint,2.0);
      midpoints.push_back(midpoint);
   }

   //2) 
   for(int point_index = 0;point_index<Npoints;point_index++)
   {
      point_2d thispoint(xs[point_index],ys[point_index]);
      thispolygon.clear();
      polygon_2d poly;
      std::vector<point_2d> polypoints;
      BOOST_FOREACH (point_2d midpoint,midpoints)
      {

         // 3) 
    	  model::segment<point_2d> midpoint_seg(thispoint,midpoint);
         //intersecting_edges.clear();
    	  std::vector<model::segment<point_2d>* > intersecting_edges;
         // 4)
         //********************************************************************
         BOOST_FOREACH (model::segment<point_2d> *egs, edges)
         {
        	 if((segIntersects(egs ,midpoint_seg)))
        	 {
        		 //back_inserter(intersecting_edges)++= egs;
        		 intersecting_edges.push_back(egs);
        	 }
         }
         //********************************************************************
         if (intersecting_edges.size() == 1)
         {
        	 thispolygon.push_back(intersecting_edges[0]);
         }
      }

      // 5
      for(edge_iterator = thispolygon.begin();edge_iterator != thispolygon.end();edge_iterator++)
      {
    	  //std::cout <<"please pay attention"<< dsv(*edge_iterator) << std::endl;
    	  **edge_iterator = forceSegmentClockwise(**edge_iterator,thispoint);
      }
      BOOST_FOREACH (model::segment<point_2d> *edge_segment,thispolygon) { polypoints.push_back( (*edge_segment).first);}

      // 6)
      //******************************************************************************
      PtsSort *pts;
      pts = new PtsSort[polypoints.size()];
      std::vector<PtsSort>   polypt;
      for (int i=0;i<(int)polypoints.size();i++)
      {
    	  pts[i].pt= polypoints[i];
    	  pts[i].angle=angleSubtendedBy(polypoints[i],thispoint);
    	  polypt.push_back(pts[i]);
      }
      sort(polypt.begin(), polypt.end(),myclockwise);
      for (int i=0;i<(int)polypoints.size();i++)
      {
    	  polypoints[i]=polypt[i].pt;
      }
      delete pts;
      //******************************************************************************

      //sort( polypoints.begin(), polypoints.end(), bind(&compareByAngle,_1,_2,thispoint) );
      // 7)
      BOOST_FOREACH (point_2d point,polypoints){append(poly,point);}
      polygons.push_back(poly);
      //std::cout<<"**********************"<<std::endl;
   }

   delete e1;
   delete e2;
   delete e3;
   delete e4;

/*
   BOOST_FOREACH(segment<point_2d> *edge_segment,edges) {free(edge_segment);}
*/
   BOOST_FOREACH(polygon_2d &poly, polygons) {correct(poly);}

   return polygons;

}

double angleSubtendedBy(point_2d point,point_2d about)
{
      /*
         Calculate the bearing from 'about' to 'point'
         So we can sort things into clockwise order.
      */
      
      subtract_point(point,about);
      double angle=0;

      if (point.x() > 0.  && point.y() > 0.)
         {
             angle = atan(point.y() / point.x() );
         }

      if (point.x() <= 0. && point.y() > 0.)
         {
    	  //angle = M_PI - atan( - point.y() / point.x() );
    	  angle = M_PI - atan(  point.y() / -point.x() );
          if(point.x() == 0.)
          {
        	 angle=M_PI/2;
          }
         }

      if (point.x() <= 0. && point.y() <=0.)
         {
            angle = M_PI + atan(point.y() / point.x() );
            if(point.x() == 0.)
            {
          	 angle=3*M_PI/2;
            }
            if(point.y() == 0.)
            {
          	 angle=M_PI;
            }
            //cout<< angle<<'\t'<< atan(  point.y() / -point.x())<<endl;cout<< angle<<'\t'<< atan(  point.y() / -point.x())<<endl;
         }

      if (point.x() >0. && point.y() <= 0.) 
         {
            angle = 2*M_PI - atan(-point.y() / point.x() );
            if(point.y() == 0.)
            {
          	 angle=2*M_PI;
            }
         }
      return angle;
}

bool compareByAngle(point_2d p1, point_2d p2, point_2d about)
{
   return angleSubtendedBy(p1,about) > angleSubtendedBy(p2,about);
}

model::segment<point_2d> forceSegmentClockwise(model::segment<point_2d> s1, point_2d about)
{
   //double ex = s1.second.x() - s1.first.x();
   //double ey = s1.second.y() - s1.first.y();
	double ex1=s1.first.x()-about.x();
	double ey1=s1.first.y()-about.y();
	double ex2=s1.second.x()-about.x();
	double ey2=s1.second.y()-about.y();
   double lambda;
	lambda=ex1*ey2-ex2*ey1;
   //lambda = s1.first.y()*ex - s1.first.x()*ey + about.x()*ey - about.y()*ex;

   if (lambda < 0.)
      {
         point_2d temp( s1.first );
         s1.first = s1.second;
         s1.second = temp;
      }

   return s1;
}

bool segIntersects(model::segment<point_2d> *s1, model::segment<point_2d> s2)
{
	std::vector<point_2d> intersections;
	double eps = 10.0E-16;
	double midx =(s1->first.x()+s1->second.x())*0.5;
	double midy =(s1->first.y()+s1->second.y())*0.5;
	if( ((fabs(midx-s2.first.x())<eps) && (fabs(midy-s2.first.y())<eps))
			||((fabs(midx-s2.second.x())<eps) && (fabs(midy-s2.second.y())<eps))
	  )
	{
		return true;
	}
	else
	{
		intersection(*s1, s2,intersections );
		return intersections.size() == 1;
	}
}
//----------------------------------------------------------------------------------------------
std::vector<polygon_2d>  cutPolygonsWithGeometry(std::vector<polygon_2d> polygon, polygon_2d Geometry, double* xs, double* ys)
{
	std::vector<polygon_2d> intersetionpolygons;
	std::vector<polygon_2d>::iterator polygon_iterator;
	std::vector<double> tempXs;
	std::vector<double> tempYs;
	int temp=0;
	for(polygon_iterator = polygon.begin(); polygon_iterator!=polygon.end();polygon_iterator++)
	{
		typedef std::vector<polygon_2d > polygon_list;
		polygon_list v;
//		intersection_inserter<polygon_2d>(Geometry, *polygon_iterator, std::back_inserter(v));
		intersection(Geometry, *polygon_iterator, v);
		//std::cout<<"polygon without clip:\t"<<dsv(*polygon_iterator)<<std::endl;
		//judge whether the polygon is cut into two separate parts,if so delete the part without including the point
		if(v.size()==1)
		{
			if(within(make<point_2d>(xs[temp], ys[temp]), v[0]))
			{
				correct(v[0]);
				intersetionpolygons.push_back(v[0]);
				tempXs.push_back(xs[temp]);
				tempYs.push_back(ys[temp]);
				//std::cout<<"clipped polygon:\t"<<dsv(v[0])<<std::endl;
				//cout<< area(v[0])/area(*polygon_iterator)<<'\t';
			}
		}
		else
		{
			for (polygon_list::const_iterator it = v.begin(); it != v.end(); ++it)
			{
				if(within(make<point_2d>(xs[temp], ys[temp]), *it))
				{
					//correct(*it);
					intersetionpolygons.push_back(*it);
					tempXs.push_back(xs[temp]);
					tempYs.push_back(ys[temp]);
					//cout<< area(*it)/area(*polygon_iterator)<<'\t';
				}
			}
		}
		temp++;
	}
	int availblepoint=tempXs.size();
	for(int i=0;i<availblepoint;i++)
	{
		xs[i]=tempXs[i];
		ys[i]=tempYs[i];
	}
	BOOST_FOREACH(polygon_2d &poly, intersetionpolygons)
	{
		correct(poly);

	}
	return intersetionpolygons;
}
std::vector<polygon_2d>  cutPolygonsWithSquare(std::vector<polygon_2d> polygon, double* xs, double* ys, double length)
{
	// length is the side length of the square that used to cut the voronoi cell

	std::vector<polygon_2d> intersetionpolygons;
	std::vector<polygon_2d>::iterator polygon_iterator;
	int temp=0;
	for(polygon_iterator = polygon.begin(); polygon_iterator!=polygon.end();polygon_iterator++)
	{

		std::cout<<dsv(*polygon_iterator )<<std::endl;
//		typedef boost::geometry::point_xy<double> P;
//		boost::geometry::box<P> square(P(xs[temp]-length/2.0,ys[temp]-length/2.0), P(xs[temp]+length/2.0,ys[temp]+length/2.0));
		box_2d square(make<point_2d>(xs[temp]-length/2.0,ys[temp]-length/2.0), make<point_2d>(xs[temp]+length/2.0,ys[temp]+length/2.0));
		//cout<<xs[temp]<<'\t'<<ys[temp]<<endl;
		typedef std::vector<polygon_2d > polygon_list;
		polygon_list v;
		//intersection_inserter<polygon_2d>(square, *polygon_iterator, std::back_inserter(v));
		intersection(square, *polygon_iterator, v);
		//judge whether the polygon is cut into two separate parts,if so delete the part without including the point
		if(v.size()==1)
		{
			intersetionpolygons.push_back(v[0]);
			//cout<< area(v[0])/area(*polygon_iterator)<<'\t';
		}
		else
		{
			for (polygon_list::const_iterator it = v.begin(); it != v.end(); ++it)
			{
				if(within(make<point_2d>(xs[temp], ys[temp]), *it))
				{
					intersetionpolygons.push_back(*it);
					//cout<< area(*it)/area(*polygon_iterator)<<'\t';
				}
			}
		}
		temp++;
	}
	BOOST_FOREACH(polygon_2d &poly, intersetionpolygons)
	{
		correct(poly);
	}
	return intersetionpolygons;
}
std::vector<polygon_2d>  cutPolygonsWithCircle(std::vector<polygon_2d> polygon, double* xs, double* ys, double radius)
{
	std::vector<polygon_2d> intersetionpolygons;
	std::vector<polygon_2d>::iterator polygon_iterator;
	int temp=0;
	for(polygon_iterator = polygon.begin(); polygon_iterator!=polygon.end();polygon_iterator++)
	{
		polygon_2d circle;
		{
			double Cpoint[361][2];
			for(int angle=0;angle<361;angle++)
			{
				Cpoint[angle][0]=xs[temp]+radius*cos(angle*PI/180);
				Cpoint[angle][1]=ys[temp]+radius*sin(angle*PI/180);
			}
			assign_points(circle, Cpoint);
		}
		correct(circle);

		typedef std::vector<polygon_2d > polygon_list;
		polygon_list v;
		//intersection_inserter<polygon_2d>(circle, *polygon_iterator, std::back_inserter(v));
		intersection(circle, *polygon_iterator, v);

		//judge whether the polygon is cut into two separate parts,if so delete the part without including the point
		if(v.size()==1)
		{
			//correct(v[0]);
			intersetionpolygons.push_back(v[0]);
			//cout<< area(v[0])/area(*polygon_iterator)<<'\t';
		}
		else
		{
			for (polygon_list::const_iterator it = v.begin(); it != v.end(); ++it)
			{
				if(within(make<point_2d>(xs[temp], ys[temp]), *it))
				{
					//correct(*it);
					intersetionpolygons.push_back(*it);
					//cout<< area(*it)/area(*polygon_iterator)<<'\t';
				}
			}
		}
		temp++;
	}

	BOOST_FOREACH(polygon_2d &poly, intersetionpolygons)
	{
		correct(poly);
	}
	return intersetionpolygons;


}
