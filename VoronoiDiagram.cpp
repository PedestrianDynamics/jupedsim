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

/*void VoronoiDiagram::getVoronoiPolygons(float *XInFrame, float *YInFrame,int numPedsInFrame, double minX, double minY, double maxX, double maxY) {

    for(int i = 0; i < numPedsInFrame; i++){
    	points.push_back(point_type(XInFrame[i], YInFrame[i]));
    }
	voronoi_diagram<double> vd;
	construct_voronoi(points.begin(), points.end(), &vd);

    printf("Traversing Voronoi graph.\n");
	    printf("Number of visited primary edges using cell iterator: %d\n",
	        iterate_primary_edges(vd, minX, minY, maxX, maxY));
}*/


// Traversing Voronoi edges using cell iterator.
std::vector<polygon_2d> VoronoiDiagram::getVoronoiPolygons(double *XInFrame, double *YInFrame, int numPedsInFrame, double minX, double minY, double maxX, double maxY)
{

	for(int i = 0; i < numPedsInFrame; i++){
	    	points.push_back(point_type(XInFrame[i], YInFrame[i]));
	    }
		voronoi_diagram<double> vd;
		construct_voronoi(points.begin(), points.end(), &vd);

	int result = 0;
	  int Ncell = 0;
	  std::vector<polygon_2d> polygons;

	  for (voronoi_diagram<double>::const_cell_iterator it = vd.cells().begin(); it != vd.cells().end(); ++it)
	  {
		  	  polygon_2d poly;
		  	  vector<point_type> polypts;
		  	  point_type pt_s;
		  	  point_type pt_e;
		  	  const voronoi_diagram<double>::cell_type& cell = *it;
			  const voronoi_diagram<double>::edge_type* edge = cell.incident_edge();
			  point_type pt_temp;
			  point_type thispoint = retrieve_point(*it);
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
						//std::cout<<"s:"<<edge->vertex0()->x()<<'\t'<<edge->vertex0()->y()<<std::endl;
						//boost::geometry::append(poly, point_2d(edge->vertex0()->x(),edge->vertex0()->y()));
						polypts.push_back(point_type(edge->vertex0()->x(),edge->vertex0()->y()));
						pt_temp = point_type(edge->vertex0()->x(),edge->vertex0()->y());
						NumVertex++;
					}
					else
					{
						//std::cout<<"s_cutted:";
						pt_s = clip_infinite_edge1(*edge, minX, minY, maxX, maxY);
						//boost::geometry::append(poly, point_2d(pt_s.x(),pt_s.y()));
						polypts.push_back(point_type(pt_s.x(),pt_s.y()));
						NumVertex++;
						infinite_s = true;
					}
					if(!edge->vertex1())
					{
						//std::cout<<"e:";
						pt_e = clip_infinite_edge1(*edge, minX, minY, maxX, maxY);
						//boost::geometry::append(poly, point_2d(pt_e.x(),pt_e.y()));
						polypts.push_back(point_type(pt_e.x(),pt_e.y()));
						NumVertex++;
						index_end = NumVertex;
						infinite_e = true;
					}

					edge = edge->next();

			  } while (edge != cell.incident_edge());
			  Ncell++;
			  if(infinite_s && infinite_e)
			  {
				  vector<point_type> vertexes = add_bounding_points(pt_s, pt_e, pt_temp, minX, minY, maxX, maxY);
				  //cout<<"size"<<vertexes.size()<<endl;
				  if(!vertexes.empty())
				  {
					  polypts.reserve(polypts.size()+vertexes.size());
					  //polypts.insert(polypts.end(), vertexes.begin(), vertexes.end() );
					  polypts.insert(polypts.begin()+index_end, vertexes.begin(), vertexes.end() );
				      // 6)
				      //******************************************************************************
/*				      PtsSort *pts;
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
			  std::cout<<boost::geometry::dsv(poly)<<std::endl;
			  //std::cout<<"This cell include the point <"<<thispoint.x()<<", "<<thispoint.y()<< ">"<<std::endl;
	  }
	  return polygons;
}

point_type VoronoiDiagram::retrieve_point(const cell_type& cell) {
  source_index_type index = cell.source_index();
    return points[index];

}

point_type VoronoiDiagram::clip_infinite_edge1( const edge_type& edge, double minX, double minY, double maxX, double maxY)
{
  const cell_type& cell1 = *edge.cell();
  const cell_type& cell2 = *edge.twin()->cell();
  point_type origin, direction, pt;
  // Infinite edges could not be created by two segment sites.

  if (cell1.contains_point() && cell2.contains_point())
  {
    point_type p1 = retrieve_point(cell1);
    point_type p2 = retrieve_point(cell2);
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


void VoronoiDiagram::clip_infinite_edge( const edge_type& edge)
{
  const cell_type& cell1 = *edge.cell();
  const cell_type& cell2 = *edge.twin()->cell();
  point_type origin, direction;
  // Infinite edges could not be created by two segment sites.
  if (cell1.contains_point() && cell2.contains_point())
  {
    point_type p1 = retrieve_point(cell1);
    point_type p2 = retrieve_point(cell2);
    origin.x((p1.x() + p2.x()) * 0.5);
    origin.y((p1.y() + p2.y()) * 0.5);
    direction.x(p1.y() - p2.y());
    direction.y(p2.x() - p1.x());
  }

  //std::cout<<origin.x()<<'\t'<< origin.y()<<'\t'<<direction.x()<<'\t'<<direction.y()<<std::endl;

}

double VoronoiDiagram::area_triangle(point_type tri_p1, point_type tri_p2, point_type tri_p3)
{
	double x = tri_p2.x() - tri_p1.x();
	double y = tri_p2.y() - tri_p1.y();

	double x2 = tri_p3.x() - tri_p1.x();
	double y2 = tri_p3.y() - tri_p1.y();

	double s = abs(x*y2 - x2*y)/2.;
	return s;
}

bool VoronoiDiagram::point_inside_triangle(point_type pt, point_type tri_p1, point_type tri_p2, point_type tri_p3)
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

vector<point_type> VoronoiDiagram::add_bounding_points(point_type pt1, point_type pt2, point_type pt, double minX, double minY, double maxX, double maxY)
{
	double eps = 10E-16;
	vector<point_type> bounding_vertex;
	if(fabs(pt1.x()-pt2.x())>eps && fabs(pt1.y()-pt2.y())>eps)
	{
		if ( (fabs(pt1.y() - maxY) < eps && fabs(pt2.x() - maxX) < eps)  ||
			 (fabs(pt2.y() - maxY) < eps && fabs(pt1.x() - maxX) < eps)
		   )
		{
			point_type vertex(maxX, maxY);
			if(point_inside_triangle(pt, vertex, pt1, pt2))
			{
				bounding_vertex.push_back(point_type(minX, maxY));
				bounding_vertex.push_back(point_type(minX, minY));
				bounding_vertex.push_back(point_type(maxX, minY));
			}
			else
			{
				bounding_vertex.push_back(point_type(maxX, maxY));
			}
		}
		else if ( (fabs(pt1.y() - maxY) < eps && fabs(pt2.x() - minX) < eps)  ||
					(fabs(pt2.y() - maxY) < eps && fabs(pt1.x() - minX) < eps)
		   	   	  )
		{
			point_type vertex(minX, maxY);
			if(point_inside_triangle(pt, vertex, pt1, pt2))
			{
				bounding_vertex.push_back(point_type(minX, minY));
				bounding_vertex.push_back(point_type(maxX, minY));
				bounding_vertex.push_back(point_type(maxX, maxY));
			}
			else
			{
				bounding_vertex.push_back(point_type(minX, maxY));
			}
		}
		else if ( (fabs(pt1.y() - minY) < eps && fabs(pt2.x() - minX) < eps)  ||
					(fabs(pt2.y() - minY) < eps && fabs(pt1.x() - minX) < eps)
		   	      )
		{
			point_type vertex(minX, minY);
			if(point_inside_triangle(pt, vertex, pt1, pt2))
			{
				bounding_vertex.push_back(point_type(maxX, minY));
				bounding_vertex.push_back(point_type(maxX, maxY));
				bounding_vertex.push_back(point_type(minX, maxY));
			}
			else
			{
				bounding_vertex.push_back(point_type(minX, minY));
			}
		}
		else if ( (fabs(pt1.y() - minY) < eps && fabs(pt2.x() - maxX) < eps)  ||
					(fabs(pt2.y() - minY) < eps && fabs(pt1.x() - maxX) < eps)
		   	   	  )
		{
			point_type vertex(minX, minY);
			if(point_inside_triangle(pt, vertex, pt1, pt2))
			{
				bounding_vertex.push_back(point_type(maxX, maxY));
				bounding_vertex.push_back(point_type(minX, maxY));
				bounding_vertex.push_back(point_type(minX, minY));
			}
			else
			{
				bounding_vertex.push_back(point_type(maxX, minY));
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
					bounding_vertex.push_back(point_type(minX, maxY));
					bounding_vertex.push_back(point_type(minX, minY));
				}
				else
				{
					bounding_vertex.push_back(point_type(maxX, minY));
					bounding_vertex.push_back(point_type(maxX, maxY));
				}
			}
			else
			{
				double tempx = pt1.x() + (pt2.x() - pt1.x()) * (pt.y() - pt1.y()) / (pt2.y() - pt1.y());
				if(tempx < pt.x())
				{
					bounding_vertex.push_back(point_type(minX, maxY));
					bounding_vertex.push_back(point_type(minX, minY));
				}
				else
				{
					bounding_vertex.push_back(point_type(maxX, minY));
					bounding_vertex.push_back(point_type(maxX, maxY));
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
					bounding_vertex.push_back(point_type(minX, minY));
					bounding_vertex.push_back(point_type(maxX, minY));
				}
				else
				{
					bounding_vertex.push_back(point_type(maxX, maxY));
					bounding_vertex.push_back(point_type(minX, maxY));
				}
			}
			else
			{
				double tempy = pt1.y() + (pt2.y() - pt1.y()) * (pt.x() - pt1.x()) / (pt2.x() - pt1.x());
				if(tempy < pt.y())
				{
					bounding_vertex.push_back(point_type(minX, minY));
					bounding_vertex.push_back(point_type(maxX, minY));
				}
				else
				{
					bounding_vertex.push_back(point_type(maxX, maxY));
					bounding_vertex.push_back(point_type(minX, maxY));
				}
			}
		}
	}
	return bounding_vertex;
}





