/*
 * VoronoiDiagram.cpp
 *
 *  Created on: 06.02.2013
 *      Author: JUNZHANG
 */

using namespace std;

#include "VoronoiDiagram.h"


VoronoiDiagram::VoronoiDiagram() {
	// TODO Auto-generated constructor stub

}

VoronoiDiagram::~VoronoiDiagram() {
	// TODO Auto-generated destructor stub
}


// Traversing Voronoi edges using cell iterator.
std::vector<polygon_2d> VoronoiDiagram::getVoronoiPolygons(double *XInFrame, double *YInFrame, int numPedsInFrame)
{

	for(int i = 0; i < numPedsInFrame; i++)
	{
	    points.push_back(point_type2(ceil(XInFrame[i]), ceil(YInFrame[i])));
	}
		//voronoi_diagram<double> vd;
	VD vd;
	construct_voronoi(points.begin(), points.end(), &vd);

	int result = 0;
	int Ncell = 0;
	std::vector<polygon_2d> polygons;
	double Bound_Max  = 10.0E10;
	bool OverBound = false;
	   do
	   {
		   double Bd_Box_minX = - Bound_Max;
		   double Bd_Box_minY = - Bound_Max;
		   double Bd_Box_maxX = Bound_Max;
		   double Bd_Box_maxY = Bound_Max;
		  for (voronoi_diagram<double>::const_cell_iterator it = vd.cells().begin(); it != vd.cells().end(); ++it)
		  {
				  polygon_2d poly;
				  vector<point_type2> polypts;
				  point_type2 pt_s;
				  point_type2 pt_e;
				  const voronoi_diagram<double>::cell_type& cell = *it;
				  const voronoi_diagram<double>::edge_type* edge = cell.incident_edge();
				  point_type2 pt_temp;
				  point_type2 thispoint = retrieve_point(*it);
				  XInFrame[Ncell] = thispoint.x();
				  YInFrame[Ncell] = thispoint.y();
				  int NumVertex = 0;
				  int  index_end = 0;
				  bool infinite_s = false;
				  bool infinite_e = false;
					// This is convenient way to iterate edges around Voronoi cell.
				  do
				  {
						if (edge->is_primary())
						  ++result;
						if(edge->vertex0())
						{
							polypts.push_back(point_type2(edge->vertex0()->x(),edge->vertex0()->y()));
							pt_temp = point_type2(edge->vertex0()->x(),edge->vertex0()->y());
							if(fabs(edge->vertex0()->x()) > Bound_Max)
							{
								Bound_Max = 10 * fabs(edge->vertex0()->x());
								OverBound = true;
								cout<<"Bound_Max:\t"<<Bound_Max<<endl;
							}
							if(fabs(edge->vertex0()->y()) > Bound_Max)
							{
								Bound_Max = 10 * fabs(edge->vertex0()->y());
								OverBound = true;
								cout<<"Bound_Max:\t"<<Bound_Max<<endl;
							}
							NumVertex++;
						}
						else
						{
							pt_s = clip_infinite_edge(*edge, Bd_Box_minX, Bd_Box_minY, Bd_Box_maxX, Bd_Box_maxY);
							polypts.push_back(point_type2(pt_s.x(),pt_s.y()));
							NumVertex++;
							infinite_s = true;
						}
						if(!edge->vertex1())
						{
							pt_e = clip_infinite_edge(*edge, Bd_Box_minX, Bd_Box_minY, Bd_Box_maxX, Bd_Box_maxY);
							polypts.push_back(point_type2(pt_e.x(),pt_e.y()));
							NumVertex++;
							index_end = NumVertex;
							infinite_e = true;
						}

						edge = edge->next();

				  } while (edge != cell.incident_edge());
				  Ncell++;
				  if(infinite_s && infinite_e)
				  {
					  vector<point_type2> vertexes = add_bounding_points(pt_s, pt_e, pt_temp, Bd_Box_minX, Bd_Box_minY, Bd_Box_maxX, Bd_Box_maxY);
					  //cout<<"size"<<vertexes.size()<<endl;
					  if(!vertexes.empty())
					  {
						  polypts.reserve(polypts.size()+vertexes.size());
						  //polypts.insert(polypts.end(), vertexes.begin(), vertexes.end() );
						  polypts.insert(polypts.begin()+index_end, vertexes.begin(), vertexes.end() );
						  // 6)
						  //******************************************************************************
						  /*PtsSort *pts;
						  pts = new PtsSort[polypts.size()];
						  std::vector<PtsSort>   polypt;

						  for (int i = 0; i < (int)polypts.size(); i++)
						  {
							  pts[i].pt= polypts[i];
							  pts[i].angle=angleSubtendedBy(polypts[i],thispoint);
							  polypt.push_back(pts[i]);
						  }
						  sort(polypt.begin(), polypt.end(),myclockwise);
						  for (int i=0;i<(int)polypts.size();i++)
						  {
							  polypts[i]=polypt[i].pt;
						  }
						  delete pts;*/
					  }
				  }
				  for (int i=0;i<(int)polypts.size();i++)
				  {
					  boost::geometry::append(poly, point_2d(polypts[i].x(),polypts[i].y()));
				  }

				  boost::geometry::correct(poly);
				  polygons.push_back(poly);
				  //std::cout<<boost::geometry::dsv(poly)<<std::endl;
				  //std::cout<<"This cell include the point <"<<thispoint.x()<<", "<<thispoint.y()<< ">"<<std::endl;
		  }
	   }while(OverBound);

	  return polygons;
}

point_type2 VoronoiDiagram::retrieve_point(const cell_type& cell) {
  source_index_type index = cell.source_index();
    return points[index];

}

point_type2 VoronoiDiagram::clip_infinite_edge( const edge_type& edge, double minX, double minY, double maxX, double maxY)
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
	  std::cout<<"hahahhahahahahaha\n";
  }

  double side =maxX-minX;
  double koef = side /(std::max)(fabs(direction.x()),fabs(direction.y()));
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

  double k =direction.y()/direction.x();
  double b = origin.y() - k*origin.x();

  if((pt.x()-origin.x())>0&&(pt.y()-origin.y())>0)
  {
	  double y_temp =k*maxX+b;
	  double x_temp =(maxY-b)/k;
	  if(y_temp<maxY&&y_temp>minY)
	  {
		  pt.x(maxX);
		  pt.y(y_temp);
	  }
	  if(x_temp<maxX&&x_temp>minX)
	  {
		  pt.x(x_temp);
		  pt.y(maxY);
	  }
  }
  else if((pt.x()-origin.x())<0&&(pt.y()-origin.y())>0)
  {
	  double y_temp =k*minX+b;
	  double x_temp =(maxY-b)/k;
	  if(y_temp<maxY&&y_temp>minY)
	  {
		  pt.x(minX);
		  pt.y(y_temp);
	  }
	  if(x_temp<maxX&&x_temp>minX)
	  {
		  pt.x(x_temp);
		  pt.y(maxY);
	  }
  }
  else if((pt.x()-origin.x())<0&&(pt.y()-origin.y())<0)
  {
	  double y_temp =k*minX+b;
	  double x_temp =(minY-b)/k;
	  if(y_temp<maxY&&y_temp>minY)
	  {
		  pt.x(minX);
		  pt.y(y_temp);
	  }
	  if(x_temp<maxX&&x_temp>minX)
	  {
		  pt.x(x_temp);
		  pt.y(minY);
	  }
  }
  else if((pt.x()-origin.x())>0&&(pt.y()-origin.y())<0)
  {
	  double y_temp =k*maxX+b;
	  double x_temp =(minY-b)/k;
	  if(y_temp<maxY&&y_temp>minY)
	  {
		  pt.x(maxX);
		  pt.y(y_temp);
	  }
	  if(x_temp<maxX&&x_temp>minX)
	  {
		  pt.x(x_temp);
		  pt.y(minY);
	  }
  }
  return pt;

}


double VoronoiDiagram::area_triangle(point_type2 tri_p1, point_type2 tri_p2, point_type2 tri_p3)
{
	double x = tri_p2.x() - tri_p1.x();
	double y = tri_p2.y() - tri_p1.y();

	double x2 = tri_p3.x() - tri_p1.x();
	double y2 = tri_p3.y() - tri_p1.y();

	double s = abs(x*y2 - x2*y)/2.;
	return s;
}

bool VoronoiDiagram::point_inside_triangle(point_type2 pt, point_type2 tri_p1, point_type2 tri_p2, point_type2 tri_p3)
{

	double s = area_triangle(tri_p1, tri_p2, tri_p3);
	double ss = area_triangle(pt, tri_p1, tri_p2);
	double ss2 = area_triangle(pt, tri_p2, tri_p3);
	double ss3 = area_triangle(pt, tri_p1, tri_p3);

	if ( ss + ss2 + ss3 - s > 1e-6 )
	{
		return false;
	}
	return true;
}

vector<point_type2> VoronoiDiagram::add_bounding_points(point_type2 pt1, point_type2 pt2, point_type2 pt, double minX, double minY, double maxX, double maxY)
{
	double eps = 10E-16;
	vector<point_type2> bounding_vertex;
	if(fabs(pt1.x()-pt2.x())>eps && fabs(pt1.y()-pt2.y())>eps)
	{
		if ( (fabs(pt1.y() - maxY) < eps && fabs(pt2.x() - maxX) < eps)  ||
			 (fabs(pt2.y() - maxY) < eps && fabs(pt1.x() - maxX) < eps)
		   )
		{
			point_type2 vertex(maxX, maxY);
			if(point_inside_triangle(pt, vertex, pt1, pt2))
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
		else if ( (fabs(pt1.y() - maxY) < eps && fabs(pt2.x() - minX) < eps)  ||
					(fabs(pt2.y() - maxY) < eps && fabs(pt1.x() - minX) < eps)
		   	   	  )
		{
			point_type2 vertex(minX, maxY);
			if(point_inside_triangle(pt, vertex, pt1, pt2))
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
		else if ( (fabs(pt1.y() - minY) < eps && fabs(pt2.x() - minX) < eps)  ||
					(fabs(pt2.y() - minY) < eps && fabs(pt1.x() - minX) < eps)
		   	      )
		{
			point_type2 vertex(minX, minY);
			if(point_inside_triangle(pt, vertex, pt1, pt2))
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
		else if ( (fabs(pt1.y() - minY) < eps && fabs(pt2.x() - maxX) < eps)  ||
					(fabs(pt2.y() - minY) < eps && fabs(pt1.x() - maxX) < eps)
		   	   	  )
		{
			point_type2 vertex(minX, minY);
			if(point_inside_triangle(pt, vertex, pt1, pt2))
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
		else if ( (fabs(pt1.y() - minY) < eps && fabs(pt2.y() - maxY) < eps)  ||
					(fabs(pt2.y() - minY) < eps && fabs(pt1.y() - maxY) < eps)
	   	   	  	  )
		{
			if(fabs(pt1.x()-pt2.x()) < eps)
			{
				if(pt1.x() < pt.x())
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
				double tempx = pt1.x() + (pt2.x() - pt1.x()) * (pt.y() - pt1.y()) / (pt2.y() - pt1.y());
				if(tempx < pt.x())
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
		else if ( (fabs(pt1.x() - minX) < eps && fabs(pt2.x() - maxX) < eps)  ||
					(fabs(pt2.x() - minX) < eps && fabs(pt1.x() - maxX) < eps)
			   	  )
		{
			if(fabs(pt1.y()-pt2.y()) < eps)
			{
				if(pt1.y() < pt.y())
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
				double tempy = pt1.y() + (pt2.y() - pt1.y()) * (pt.x() - pt1.x()) / (pt2.x() - pt1.x());
				if(tempy < pt.y())
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

std::vector<polygon_2d>  VoronoiDiagram::cutPolygonsWithGeometry(std::vector<polygon_2d> polygon, polygon_2d Geometry, double* xs, double* ys)
{
	std::vector<polygon_2d> intersetionpolygons;
	std::vector<polygon_2d>::iterator polygon_iterator;

	int temp=0;
	for(polygon_iterator = polygon.begin(); polygon_iterator!=polygon.end();polygon_iterator++)
	{
		typedef std::vector<polygon_2d > polygon_list;
		polygon_list v;
		boost::geometry::intersection(Geometry, *polygon_iterator, v);

		//judge whether the polygon is cut into two separate parts,if so delete the part without including the point
		if(v.size()==1)
		{
			if(boost::geometry::within(point_2d(xs[temp], ys[temp]), v[0]))
			{
				intersetionpolygons.push_back(v[0]);
			}
		}
		else
		{
			for (polygon_list::const_iterator it = v.begin(); it != v.end(); ++it)
			{
				if(boost::geometry::within(point_2d(xs[temp], ys[temp]), *it))
				{
					intersetionpolygons.push_back(*it);
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



