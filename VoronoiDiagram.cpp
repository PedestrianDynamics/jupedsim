/**
 * \file        VoronoiDiagram.cpp
 * \date        Oct 10, 2014
 * \version     v0.6
 * \copyright   <2009-2014> Forschungszentrum Jülich GmbH. All rights reserved.
 *
 * \section License
 * This file is part of JuPedSim.
 *
 * JuPedSim is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * JuPedSim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with JuPedSim. If not, see <http://www.gnu.org/licenses/>.
 *
 * \section Description
 * The VoronoiDiagram class define functions used to calculate Voronoi diagrams
 * from trajectories.
 *
 *
 **/




#include <iostream>
#include "VoronoiDiagram.h"


using namespace std;

VoronoiDiagram::VoronoiDiagram()
{
}

VoronoiDiagram::~VoronoiDiagram()
{
}

// Traversing Voronoi edges using cell iterator.
std::vector<polygon_2d> VoronoiDiagram::getVoronoiPolygons(double *XInFrame, double *YInFrame,
          double *VInFrame, int *IdInFrame, int numPedsInFrame)
{
     int XInFrame_temp[numPedsInFrame];
     int YInFrame_temp[numPedsInFrame];
     double VInFrame_temp[numPedsInFrame];
     int IdInFrame_temp[numPedsInFrame];

     for (int i = 0; i < numPedsInFrame; i++)
     {
          points.push_back(point_type2((int) (XInFrame[i] + 0.5), (int) (YInFrame[i] + 0.5)));
          XInFrame_temp[i] = (int) (XInFrame[i] + 0.5);
          YInFrame_temp[i] = (int) (YInFrame[i] + 0.5);
          VInFrame_temp[i] = VInFrame[i];
          IdInFrame_temp[i] = IdInFrame[i];
     }
     //voronoi_diagram<double> vd;
     VD vd;
     construct_voronoi(points.begin(), points.end(), &vd);

     int result = 0;
     int Ncell = 0;
     std::vector<polygon_2d> polygons;
     double Bound_Max = 10.0E4;
     bool OverBound = false;
     do
     {
          double Bd_Box_minX = -Bound_Max;
          double Bd_Box_minY = -Bound_Max;
          double Bd_Box_maxX = Bound_Max;
          double Bd_Box_maxY = Bound_Max;
          polygon_2d boundingbox;
          boost::geometry::append(boundingbox, boost::geometry::make<point_2d>(-Bound_Max, -Bound_Max));
          boost::geometry::append(boundingbox, boost::geometry::make<point_2d>(-Bound_Max, Bound_Max));
          boost::geometry::append(boundingbox, boost::geometry::make<point_2d>(Bound_Max, Bound_Max));
          boost::geometry::append(boundingbox, boost::geometry::make<point_2d>(Bound_Max, -Bound_Max));
          boost::geometry::correct(boundingbox);
          if(!polygons.empty())
          {
        	  polygons.clear();
        	  OverBound = false;
        	  Ncell=0;
        	  result=0;
          }
          //std::cout<<"Please start to work!!"<<polygons.size()<<std::endl;
          //cout << "Bound_Max:\t" << Bound_Max << endl;
          for (voronoi_diagram<double>::const_cell_iterator it = vd.cells().begin();
                    it != vd.cells().end(); ++it)
          {
               polygon_2d poly;
               vector<point_type2> polypts;
               point_type2 pt_s;
               point_type2 pt_e;
               const voronoi_diagram<double>::cell_type& cell = *it;
               const voronoi_diagram<double>::edge_type* edge = cell.incident_edge();
               point_type2 pt_temp;
               point_type2 thispoint = retrieve_point(*it);
               for (int k = 0; k < numPedsInFrame; k++)
               {
                    if (XInFrame_temp[k] == thispoint.x() && YInFrame_temp[k] == thispoint.y())
                    {
                         VInFrame[Ncell] = VInFrame_temp[k];
                         IdInFrame[Ncell] = IdInFrame_temp[k];
                         break;
                    }
               }
               XInFrame[Ncell] = thispoint.x();
               YInFrame[Ncell] = thispoint.y();
               int NumVertex = 0;
               int index_end = 0;
               bool infinite_s = false;
               bool infinite_e = false;
               // This is convenient way to iterate edges around Voronoi cell.
               //std::cout<<"THIS IS The CELL:\t"<<Ncell<<std::endl;
               do
               {
                    if (edge->is_primary())
                         ++result;
                    if (edge->vertex0())
                    {
                         if(edge->vertex1())
                         {
							 if(fabs(edge->vertex0()->x()) < Bound_Max && (fabs(edge->vertex0()->y()) < Bound_Max)
									 &&fabs(edge->vertex1()->x()) < Bound_Max && (fabs(edge->vertex1()->y()) < Bound_Max))
							 {
								 polypts.push_back(point_type2(edge->vertex0()->x(), edge->vertex0()->y()));
								 pt_temp = point_type2(edge->vertex0()->x(), edge->vertex0()->y());
								 NumVertex++;
							 }
							 else if((fabs(edge->vertex0()->x()) > Bound_Max || (fabs(edge->vertex0()->y()) > Bound_Max))
									 &&fabs(edge->vertex1()->x()) < Bound_Max && (fabs(edge->vertex1()->y()) < Bound_Max))
							 {
								 pt_s = getIntersectionPoint(point_2d(edge->vertex0()->x(), edge->vertex0()->y()), point_2d(edge->vertex1()->x(), edge->vertex1()->y()), boundingbox);
								 polypts.push_back(point_type2(pt_s.x(), pt_s.y()));
								 NumVertex++;
								 infinite_s = true;
							 }
							 else if((fabs(edge->vertex0()->x()) < Bound_Max && (fabs(edge->vertex0()->y()) < Bound_Max))
							 		&&(fabs(edge->vertex1()->x()) > Bound_Max || (fabs(edge->vertex1()->y()) > Bound_Max)))
							 {
								 polypts.push_back(point_type2(edge->vertex0()->x(), edge->vertex0()->y()));
								 pt_e = getIntersectionPoint(point_2d(edge->vertex0()->x(), edge->vertex0()->y()), point_2d(edge->vertex1()->x(), edge->vertex1()->y()), boundingbox);
								 polypts.push_back(point_type2(pt_e.x(), pt_e.y()));
								 NumVertex+=2;
								 index_end = NumVertex;
								 infinite_e = true;
							 }
                         }
                         else
                         {
                        	 if(fabs(edge->vertex0()->x()) < Bound_Max && (fabs(edge->vertex0()->y()) < Bound_Max))
                        	{
                        		 polypts.push_back(point_type2(edge->vertex0()->x(), edge->vertex0()->y()));
                        		 pt_e = clip_infinite_edge(*edge, Bd_Box_minX, Bd_Box_minY, Bd_Box_maxX,
                        		                                    Bd_Box_maxY);
                        		 polypts.push_back(point_type2(pt_e.x(), pt_e.y()));
								 NumVertex+=2;
								 index_end = NumVertex;
								 //std::cout<<"This is a test program 444444!"<<std::endl;
								 infinite_e = true;
                        	}
                         }
                    }
                    else
                    {
                    	 if(edge->vertex1()&&fabs(edge->vertex1()->x()) < Bound_Max && (fabs(edge->vertex1()->y()) < Bound_Max))
                    	 {
							pt_s = clip_infinite_edge(*edge, Bd_Box_minX, Bd_Box_minY, Bd_Box_maxX,
									   Bd_Box_maxY);
							 polypts.push_back(point_type2(pt_s.x(), pt_s.y()));
							 NumVertex++;
							 infinite_s = true;
							 //std::cout<<"This is a test program 555555!"<<std::endl;
                    	 }
                    }
                    edge = edge->next();

               } while (edge != cell.incident_edge());
               Ncell++;

               if (infinite_s && infinite_e)
               {

            	   vector<point_type2> vertexes = add_bounding_points(pt_s, pt_e, pt_temp,
                              Bd_Box_minX, Bd_Box_minY, Bd_Box_maxX, Bd_Box_maxY);
                    //cout<<"size"<<vertexes.size()<<endl;
                    if (!vertexes.empty())
                    {
                         polypts.reserve(polypts.size() + vertexes.size());
                         polypts.insert(polypts.begin() + index_end, vertexes.begin(),
                                   vertexes.end());
                    }
               }
               for (int i = 0; i < (int) polypts.size(); i++)
               {
                    boost::geometry::append(poly, point_2d(polypts[i].x(), polypts[i].y()));
               }

               boost::geometry::correct(poly);
               polygons.push_back(poly);
               //std::cout<<boost::geometry::dsv(poly)<<std::endl;
               //std::cout<<"This cell include the point <"<<thispoint.x()<<", "<<thispoint.y()<< ">"<<std::endl;
          }
     } while (OverBound);
     //std::cout<<"Please come work!!"<<polygons.size()<<std::endl;
     return polygons;
}

point_type2 VoronoiDiagram::retrieve_point(const cell_type& cell)
{
     source_index_type index = cell.source_index();
     return points[index];
}

point_type2 VoronoiDiagram::clip_infinite_edge(const edge_type& edge, double minX, double minY,
          double maxX, double maxY)
{
     const cell_type& cell1 = *edge.cell();
     const cell_type& cell2 = *edge.twin()->cell();
     point_type2 origin, direction, pt;
     // Infinite edges could not be created by two segment sites.

     if (cell1.contains_point() && cell2.contains_point())
     {
          point_type2 p1 = retrieve_point(cell1);
          point_type2 p2 = retrieve_point(cell2);
          origin.x((p1.x() + p2.x()) * 0.5);
          origin.y((p1.y() + p2.y()) * 0.5);
          direction.x(p1.y() - p2.y());
          direction.y(p2.x() - p1.x());
     }
     else
     {
          std::cout << "hahahhahahahahaha\n";
     }

     double side = maxX - minX;
     double koef = side / (std::max)(fabs(direction.x()), fabs(direction.y()));
     if (edge.vertex0() == NULL)
     {
          pt.x(origin.x() - direction.x() * koef);
          pt.y(origin.y() - direction.y() * koef);
     }
     else if (edge.vertex1() == NULL)
     {
          pt.x(origin.x() + direction.x() * koef);
          pt.y(origin.y() + direction.y() * koef);
     }

     polygon_2d boundingbox;
     boost::geometry::append(boundingbox, boost::geometry::make<point_2d>(minX, minY));
     boost::geometry::append(boundingbox, boost::geometry::make<point_2d>(minX, maxY));
     boost::geometry::append(boundingbox, boost::geometry::make<point_2d>(maxX, maxY));
     boost::geometry::append(boundingbox, boost::geometry::make<point_2d>(maxX, minY));
     boost::geometry::correct(boundingbox);

     pt = getIntersectionPoint(point_2d(pt.x(), pt.y()), point_2d(origin.x(), origin.y()), boundingbox);
     /*if(direction.x()==0)
     {

    	 std::cout<<"There is vertical line!"<<std::endl;
     }
     if(direction.y()==0)
          {

         	 std::cout<<"There is horizontal line!"<<std::endl;
          }
     double k = direction.y() / direction.x();
     double b = origin.y() - k * origin.x();

     if ((pt.x() - origin.x()) > 0 && (pt.y() - origin.y()) > 0)
     {
          double y_temp = k * maxX + b;
          double x_temp = (maxY - b) / k;
          if (y_temp < maxY && y_temp > minY)
          {
               pt.x(maxX);
               pt.y(y_temp);
          }
          if (x_temp < maxX && x_temp > minX)
          {
               pt.x(x_temp);
               pt.y(maxY);
          }
     }
     else if ((pt.x() - origin.x()) < 0 && (pt.y() - origin.y()) > 0)
     {
          double y_temp = k * minX + b;
          double x_temp = (maxY - b) / k;
          if (y_temp < maxY && y_temp > minY)
          {
               pt.x(minX);
               pt.y(y_temp);
          }
          if (x_temp < maxX && x_temp > minX)
          {
               pt.x(x_temp);
               pt.y(maxY);
          }
     }
     else if ((pt.x() - origin.x()) < 0 && (pt.y() - origin.y()) < 0)
     {
          double y_temp = k * minX + b;
          double x_temp = (minY - b) / k;
          if (y_temp < maxY && y_temp > minY)
          {
               pt.x(minX);
               pt.y(y_temp);
          }
          if (x_temp < maxX && x_temp > minX)
          {
               pt.x(x_temp);
               pt.y(minY);
          }
     }
     else if ((pt.x() - origin.x()) > 0 && (pt.y() - origin.y()) < 0)
     {
          double y_temp = k * maxX + b;
          double x_temp = (minY - b) / k;
          if (y_temp < maxY && y_temp > minY)
          {
               pt.x(maxX);
               pt.y(y_temp);
          }
          if (x_temp < maxX && x_temp > minX)
          {
               pt.x(x_temp);
               pt.y(minY);
          }
     }*/
     return pt;

}

double VoronoiDiagram::area_triangle(const point_type2& tri_p1, const point_type2& tri_p2, const point_type2& tri_p3)
{
     double x = tri_p2.x() - tri_p1.x();
     double y = tri_p2.y() - tri_p1.y();

     double x2 = tri_p3.x() - tri_p1.x();
     double y2 = tri_p3.y() - tri_p1.y();

     return abs(x * y2 - x2 * y) / 2.;
}

bool VoronoiDiagram::point_inside_triangle(const point_type2& pt, const point_type2& tri_p1, const point_type2& tri_p2, const point_type2& tri_p3)
{
     double s = area_triangle(tri_p1, tri_p2, tri_p3);
     double ss = area_triangle(pt, tri_p1, tri_p2);
     double ss2 = area_triangle(pt, tri_p2, tri_p3);
     double ss3 = area_triangle(pt, tri_p1, tri_p3);

     if (ss + ss2 + ss3 - s > 1e-6)
     {
          return false;
     }
     return true;
}

vector<point_type2> VoronoiDiagram::add_bounding_points(point_type2 pt1, point_type2 pt2,
          point_type2 pt, double minX, double minY, double maxX, double maxY)
{
     double eps = 10E-16;
     vector<point_type2> bounding_vertex;
     if (fabs(pt1.x() - pt2.x()) > eps && fabs(pt1.y() - pt2.y()) > eps)
     {
          if ((fabs(pt1.y() - maxY) < eps && fabs(pt2.x() - maxX) < eps)
                    || (fabs(pt2.y() - maxY) < eps && fabs(pt1.x() - maxX) < eps))
          {
               point_type2 vertex(maxX, maxY);
               if (point_inside_triangle(pt, vertex, pt1, pt2))
               {
                    bounding_vertex.push_back(point_type2(minX, maxY));
                    bounding_vertex.push_back(point_type2(minX, minY));
                    bounding_vertex.push_back(point_type2(maxX, minY));
               }
               else
               {
                    bounding_vertex.push_back(point_type2(maxX, maxY));
               }
          }
          else if ((fabs(pt1.y() - maxY) < eps && fabs(pt2.x() - minX) < eps)
                    || (fabs(pt2.y() - maxY) < eps && fabs(pt1.x() - minX) < eps))
          {
               point_type2 vertex(minX, maxY);
               if (point_inside_triangle(pt, vertex, pt1, pt2))
               {
                    bounding_vertex.push_back(point_type2(minX, minY));
                    bounding_vertex.push_back(point_type2(maxX, minY));
                    bounding_vertex.push_back(point_type2(maxX, maxY));
               }
               else
               {
                    bounding_vertex.push_back(point_type2(minX, maxY));
               }
          }
          else if ((fabs(pt1.y() - minY) < eps && fabs(pt2.x() - minX) < eps)
                    || (fabs(pt2.y() - minY) < eps && fabs(pt1.x() - minX) < eps))
          {
               point_type2 vertex(minX, minY);
               if (point_inside_triangle(pt, vertex, pt1, pt2))
               {
                    bounding_vertex.push_back(point_type2(maxX, minY));
                    bounding_vertex.push_back(point_type2(maxX, maxY));
                    bounding_vertex.push_back(point_type2(minX, maxY));
               }
               else
               {
                    bounding_vertex.push_back(point_type2(minX, minY));
               }
          }
          else if ((fabs(pt1.y() - minY) < eps && fabs(pt2.x() - maxX) < eps)
                    || (fabs(pt2.y() - minY) < eps && fabs(pt1.x() - maxX) < eps))
          {
               point_type2 vertex(minX, minY);
               if (point_inside_triangle(pt, vertex, pt1, pt2))
               {
                    bounding_vertex.push_back(point_type2(maxX, maxY));
                    bounding_vertex.push_back(point_type2(minX, maxY));
                    bounding_vertex.push_back(point_type2(minX, minY));
               }
               else
               {
                    bounding_vertex.push_back(point_type2(maxX, minY));
               }
          }
          else if ((fabs(pt1.y() - minY) < eps && fabs(pt2.y() - maxY) < eps)
                    || (fabs(pt2.y() - minY) < eps && fabs(pt1.y() - maxY) < eps))
          {
               if (fabs(pt1.x() - pt2.x()) < eps)
               {
                    if (pt1.x() < pt.x())
                    {
                         bounding_vertex.push_back(point_type2(minX, maxY));
                         bounding_vertex.push_back(point_type2(minX, minY));
                    }
                    else
                    {
                         bounding_vertex.push_back(point_type2(maxX, minY));
                         bounding_vertex.push_back(point_type2(maxX, maxY));
                    }
               }
               else
               {
                    double tempx = pt1.x()
                              + (pt2.x() - pt1.x()) * (pt.y() - pt1.y()) / (pt2.y() - pt1.y());
                    if (tempx < pt.x())
                    {
                         bounding_vertex.push_back(point_type2(minX, maxY));
                         bounding_vertex.push_back(point_type2(minX, minY));
                    }
                    else
                    {
                         bounding_vertex.push_back(point_type2(maxX, minY));
                         bounding_vertex.push_back(point_type2(maxX, maxY));
                    }
               }
          }
          else if ((fabs(pt1.x() - minX) < eps && fabs(pt2.x() - maxX) < eps)
                    || (fabs(pt2.x() - minX) < eps && fabs(pt1.x() - maxX) < eps))
          {
               if (fabs(pt1.y() - pt2.y()) < eps)
               {
                    if (pt1.y() < pt.y())
                    {
                         bounding_vertex.push_back(point_type2(minX, minY));
                         bounding_vertex.push_back(point_type2(maxX, minY));
                    }
                    else
                    {
                         bounding_vertex.push_back(point_type2(maxX, maxY));
                         bounding_vertex.push_back(point_type2(minX, maxY));
                    }
               }
               else
               {
                    double tempy = pt1.y()
                              + (pt2.y() - pt1.y()) * (pt.x() - pt1.x()) / (pt2.x() - pt1.x());
                    if (tempy < pt.y())
                    {
                         bounding_vertex.push_back(point_type2(minX, minY));
                         bounding_vertex.push_back(point_type2(maxX, minY));
                    }
                    else
                    {
                         bounding_vertex.push_back(point_type2(maxX, maxY));
                         bounding_vertex.push_back(point_type2(minX, maxY));
                    }
               }
          }
     }
     return bounding_vertex;
}


//-----------In getIntersectionPoint() the edges of the square is  vertical or horizontal segment--------------
point_type2 VoronoiDiagram::getIntersectionPoint(point_2d pt0, point_2d pt1, polygon_2d square)
{
	std::vector<point_2d> pt;
	segment edge0(pt0, pt1);
	std::vector<point_2d> const& points = square.outer();
	//std::cout<<"the number of points is:" <<points.size()<<std::endl;
	for (std::vector<point_2d>::size_type i = 1; i < points.size(); ++i)
	{
		segment edge1(points[i], points[i-1]);
		if(boost::geometry::intersects(edge0, edge1))
		{
			boost::geometry::intersection(edge0, edge1, pt);
			break;
		}
	}
	if(pt.empty())
	{
		segment edge1(points[3], points[0]);
		boost::geometry::intersection(edge0, edge1, pt);
	}
	point_type2 interpts(pt[0].x(), pt[0].y());
	return interpts;
}

std::vector<polygon_2d> VoronoiDiagram::cutPolygonsWithGeometry(std::vector<polygon_2d> polygon,
          polygon_2d Geometry, double* xs, double* ys)
{
	 std::vector<polygon_2d> intersetionpolygons;
     std::vector<polygon_2d>::iterator polygon_iterator;
     int temp = 0;
    // std::cout<<"the number of polygons is:"<<polygon.size()<<std::endl;
     for (polygon_iterator = polygon.begin(); polygon_iterator != polygon.end(); polygon_iterator++)
     {
          typedef std::vector<polygon_2d> polygon_list;
          polygon_list v;
          boost::geometry::intersection(Geometry, *polygon_iterator, v);
          if(v.size()==0)
          {
        	  //std::cout<<"the number of cut polygon is:"<<v.size()<<std::endl;
        	  //std::cout<<"The polygon is:"<<boost::geometry::dsv(*polygon_iterator)<<std::endl;
        	  //std::cout<<"The geometry is:"<<boost::geometry::dsv(Geometry)<<std::endl;
          }
          //judge whether the polygon is cut into two separate parts,if so delete the part without including the point
          if (v.size() == 1)
          {
               if (boost::geometry::within(point_2d(xs[temp], ys[temp]), v[0]))
               {
                    intersetionpolygons.push_back(v[0]);
                    if(boost::geometry::area(v[0])>boost::geometry::area(*polygon_iterator)+10)
                    {
                  	  std::cout<<"The error cut polygon is:"<<boost::geometry::dsv(*polygon_iterator)<<std::endl;
                  	  //std::cout<<"The geometry is:"<<boost::geometry::dsv(Geometry)<<std::endl;
                    }
               }
          }
          else
          {
               for (polygon_list::const_iterator it = v.begin(); it != v.end(); ++it)
               {
                    if (boost::geometry::within(point_2d(xs[temp], ys[temp]), *it))
                    {
                         intersetionpolygons.push_back(*it);
                         if(boost::geometry::area(*it)>10000000)
                         {
                       	  std::cout<<"The polygon from IT is:"<<boost::geometry::dsv(*polygon_iterator)<<std::endl;
                       	  std::cout<<"The geometry from IT is:"<<boost::geometry::dsv(Geometry)<<std::endl;
                         }
                    }
               }
          }
          temp++;
     }

     BOOST_FOREACH(polygon_2d &poly, intersetionpolygons)
     {
          boost::geometry::correct(poly);
          //std::cout<<boost::geometry::dsv(poly)<<std::endl;
     }

     return intersetionpolygons;
}

std::vector<polygon_2d> VoronoiDiagram::cutPolygonsWithCircle(std::vector<polygon_2d> polygon,
          double* xs, double* ys, double radius, int edges)
{
     std::vector<polygon_2d> intersetionpolygons;
     std::vector<polygon_2d>::iterator polygon_iterator;
     int temp = 0;
     for (polygon_iterator = polygon.begin(); polygon_iterator != polygon.end(); polygon_iterator++)
     {
          polygon_2d circle;
          {
               for (int angle = 0; angle <=edges; angle++)
               {
                    double ptx= xs[temp] + radius * cos(angle * 2*PI / edges);
                    double pty= ys[temp] + radius * sin(angle * 2*PI / edges);
                    boost::geometry::append(circle, boost::geometry::make<point_2d>(ptx, pty));
                    //cout<<ptx<<"\t"<<pty<<"\t"<<xs[temp]<<"\t"<<ys[temp]<<"\t"<<radius<<endl;
               }
          }
          boost::geometry::correct(circle);
          typedef std::vector<polygon_2d> polygon_list;
          polygon_list v;
          boost::geometry::intersection(circle, *polygon_iterator, v);

          //judge whether the polygon is cut into two separate parts,if so delete the part without including the point
          if (v.size() == 1)
          {
               //correct(v[0]);
               intersetionpolygons.push_back(v[0]);
               //cout<< area(v[0])/area(*polygon_iterator)<<'\t';
          }
          else
          {
               for (polygon_list::const_iterator it = v.begin(); it != v.end(); ++it)
               {
                    if (boost::geometry::within(point_2d(xs[temp], ys[temp]), *it))
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
          boost::geometry::correct(poly);
     }
     return intersetionpolygons;
}

