/*
 * VoronoiDiagram.cpp
 *
 *  Created on: 06.02.2013
 *      Author: JUNZHANG
 */

#include "VoronoiDiagram.h"


VoronoiDiagram::VoronoiDiagram() {
	// TODO Auto-generated constructor stub

}

VoronoiDiagram::~VoronoiDiagram() {
	// TODO Auto-generated destructor stub
}

void VoronoiDiagram::getVoronoiPolygons(float *XInFrame, float *YInFrame,int numPedsInFrame, double minX, double minY, double maxX, double maxY) {

    for(int i = 0; i < numPedsInFrame; i++){
    	points.push_back(point_type(XInFrame[i], YInFrame[i]));
    }
	voronoi_diagram<double> vd;
	construct_voronoi(points.begin(), points.end(), &vd);

    printf("Traversing Voronoi graph.\n");
	    printf("Number of visited primary edges using cell iterator: %d\n",
	        iterate_primary_edges(vd, minX, minY, maxX, maxY));
}


point_type VoronoiDiagram::retrieve_point(const cell_type& cell) {
  source_index_type index = cell.source_index();
    return points[index];

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

  std::cout<<origin.x()<<'\t'<< origin.y()<<'\t'<<direction.x()<<'\t'<<direction.y()<<std::endl;

}
// Traversing Voronoi edges using cell iterator.
int VoronoiDiagram::iterate_primary_edges(const voronoi_diagram<double> &vd, double minX, double minY, double maxX, double maxY)
{
	  int result = 0;
	  int Ncell = 0;
	  for (voronoi_diagram<double>::const_cell_iterator it = vd.cells().begin(); it != vd.cells().end(); ++it)
	  {
			  const voronoi_diagram<double>::cell_type& cell = *it;
			  const voronoi_diagram<double>::edge_type* edge = cell.incident_edge();
				// This is convenient way to iterate edges around Voronoi cell.
			  do
			  {
					if (edge->is_primary())
					  ++result;
					if(edge->vertex0())
					{
						std::cout<<'s'<<edge->vertex0()->x()<<'\t'<<edge->vertex0()->y()<<std::endl;
					}
					else
					{
						clip_infinite_edge(*edge);
					}
					if(!edge->vertex1())
					{
						std::cout<<'e';
						clip_infinite_edge(*edge);
					}

					edge = edge->next();

			  } while (edge != cell.incident_edge());
			  Ncell++;
			  std::cout<<"This is the "<<Ncell<<"th Cell"<<std::endl;
	  }
	  return result;
}

void VoronoiDiagram::Inter_Ray_Box(double ray[2][2], double minX, double minY, double maxX, double maxY){
    double x0 = ray[0][0], y0 = ray[0][1];
    double x1 = ray[1][0], y1 = ray[1][1];
    double x,y;

    if((x1 != x0)&&(y1 != y0))
    {
        double k = (y1-y0) / (x1-x0);
        double b = y0 - k * x0;
        double temp[4][2] = {{(minY - b)/k,minY},{maxX,k*maxX+b},{(maxY - b)/k,maxY},{minX,k*minX+b}};
        for(int i=0;i<4;i++)
        {
        	if(((x0-x1)>0 && (x0-temp[i][0])>0) || ((x0-x1)<0 && (x0-temp[i][0])<0) || ((y0-y1)>0 && (y0-temp[i][1])>0) || ((y0-y1)<0 && (y0-temp[i][1])<0))
        	{
        		x = temp[i][0];
        		y = temp[i][1];
        		break;
        	}
        }
    }
    else if (x1==x0)
    {
    	if (y1>y0)
    	{
    		x = x0;
    		y = maxY;
    	}
    	else
    	{
    		x = x0;
    		y = minY;
    	}
    }
    else
    {
    	if (x1>x0)
    	{
    		x = maxX;
    		y = x0;
    	}
    	else
    	{
    		x = minX;
    		y = x0;
    	}
    }

    cout<<"intersection point: ( "<<x<<","<<y<<")\n";
}

