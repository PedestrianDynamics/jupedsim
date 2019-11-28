/**
 * \file        VoronoiDiagram.cpp
 * \date        Oct 10, 2014
 * \version     v0.7
 * \copyright   <2009-2015> Forschungszentrum Jülich GmbH. All rights reserved.
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


#include "VoronoiDiagram.h"

#include <iostream>


using namespace std;
using namespace boost::geometry;

VoronoiDiagram::VoronoiDiagram() {}

VoronoiDiagram::~VoronoiDiagram() {}
//typedef std::vector<std::pair<polygon_2d>, int> >  poly_id;
// Traversing Voronoi edges using cell iterator.
std::vector<std::pair<polygon_2d, int>> VoronoiDiagram::getVoronoiPolygons(
    vector<double> & XInFrame,
    vector<double> & YInFrame,
    vector<double> & VInFrame,
    vector<int> & IdInFrame,
    const double Bound_Max)
{
    double M2CM        = 10000;
    int numPedsInFrame = IdInFrame.size();
    vector<int> XInFrame_temp;
    vector<int> YInFrame_temp;
    vector<double> VInFrame_temp;
    vector<int> IdInFrame_temp;
    // in case 1 or 2 pedestrians are in the geometry
    // add dummy pedestrians around to enable voronoi calculations
    // @todo: maybe use negative ids for these dummy pedestrians to exclude
    // them from any analysis.
    if(numPedsInFrame == 1 || numPedsInFrame == 2) {
        numPedsInFrame += 4;
        // up right
        XInFrame.push_back(XInFrame[0] + 10 * M2CM);
        YInFrame.push_back(YInFrame[0] + 10 * M2CM);
        VInFrame.push_back(VInFrame[0]);
        //IdInFrame.push_back(IdInFrame[0]+1);
        IdInFrame.push_back(-1);
        // up left
        XInFrame.push_back(XInFrame[0] - 10 * M2CM);
        YInFrame.push_back(YInFrame[0] + 10 * M2CM);
        VInFrame.push_back(VInFrame[0]);
        //IdInFrame.push_back(IdInFrame[0]+2);
        IdInFrame.push_back(-2);
        // down right
        XInFrame.push_back(XInFrame[0] + 10 * M2CM);
        YInFrame.push_back(YInFrame[0] - 10 * M2CM);
        VInFrame.push_back(VInFrame[0]);
        //IdInFrame.push_back(IdInFrame[0]+3);
        IdInFrame.push_back(-3);
        // down left
        XInFrame.push_back(XInFrame[0] - 10 * M2CM);
        YInFrame.push_back(YInFrame[0] - 10 * M2CM);
        VInFrame.push_back(VInFrame[0]);
        // IdInFrame.push_back(IdInFrame[0]+4);
        IdInFrame.push_back(-4);
    }

    for(int i = 0; i < numPedsInFrame; i++) {
        points.push_back(point_type2(round(XInFrame[i]), round(YInFrame[i])));
        XInFrame_temp.push_back(round(XInFrame[i]));
        YInFrame_temp.push_back(round(YInFrame[i]));
        VInFrame_temp.push_back(VInFrame[i]);
        IdInFrame_temp.push_back(IdInFrame[i]);
        // std::cout << "i: " << i << " Id " << IdInFrame[i] << "  pos = " << XInFrame[i] << ", " << YInFrame[i] << "\n";
    }


    VD voronoidiagram;
    construct_voronoi(points.begin(), points.end(), &voronoidiagram);
    int Ncell = 0;
    //std::vector<polygon_2d> polygons;
    std::vector<std::pair<polygon_2d, int>> polygons_id;
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

    for(voronoi_diagram<double>::const_cell_iterator it = voronoidiagram.cells().begin();
        it != voronoidiagram.cells().end();
        ++it) {
        polygon_2d poly;
        vector<point_type2> polypts;
        point_type2 pt_s;
        point_type2 pt_e;
        const voronoi_diagram<double>::cell_type & cell = *it;
        const voronoi_diagram<double>::edge_type * edge = cell.incident_edge();
        point_type2 pt_temp;
        point_type2 thispoint = retrieve_point(*it);
        for(int k = 0; k < numPedsInFrame; k++) {
            if(XInFrame_temp[k] == thispoint.x() && YInFrame_temp[k] == thispoint.y()) {
                VInFrame[Ncell]  = VInFrame_temp[k];
                IdInFrame[Ncell] = IdInFrame_temp[k];
                break;
            }
        }

        XInFrame[Ncell] = thispoint.x();
        YInFrame[Ncell] = thispoint.y();
        int NumVertex   = 0;
        int index_end   = 0;
        bool infinite_s = false;
        bool infinite_e = false;
        // This is convenient way to iterate edges around Voronoi cell.

        do {
            if(edge->vertex0()) {
                if(edge->vertex1()) {
                    if(fabs(edge->vertex0()->x()) < Bound_Max &&
                       (fabs(edge->vertex0()->y()) < Bound_Max) &&
                       fabs(edge->vertex1()->x()) < Bound_Max &&
                       (fabs(edge->vertex1()->y()) < Bound_Max)) {
                        polypts.push_back(point_type2(edge->vertex0()->x(), edge->vertex0()->y()));
                        pt_temp = point_type2(edge->vertex0()->x(), edge->vertex0()->y());
                        NumVertex++;
                    } else if(
                        (fabs(edge->vertex0()->x()) > Bound_Max ||
                         (fabs(edge->vertex0()->y()) > Bound_Max)) &&
                        fabs(edge->vertex1()->x()) < Bound_Max &&
                        (fabs(edge->vertex1()->y()) < Bound_Max)) {
                        pt_s = getIntersectionPoint(
                            point_2d(edge->vertex0()->x(), edge->vertex0()->y()),
                            point_2d(edge->vertex1()->x(), edge->vertex1()->y()),
                            boundingbox);
                        pt_temp = point_type2(edge->vertex1()->x(), edge->vertex1()->y());
                        polypts.push_back(point_type2(pt_s.x(), pt_s.y()));
                        NumVertex++;
                        infinite_s = true;
                    } else if(
                        (fabs(edge->vertex0()->x()) < Bound_Max &&
                         (fabs(edge->vertex0()->y()) < Bound_Max)) &&
                        (fabs(edge->vertex1()->x()) > Bound_Max ||
                         (fabs(edge->vertex1()->y()) > Bound_Max))) {
                        polypts.push_back(point_type2(edge->vertex0()->x(), edge->vertex0()->y()));
                        pt_e = getIntersectionPoint(
                            point_2d(edge->vertex0()->x(), edge->vertex0()->y()),
                            point_2d(edge->vertex1()->x(), edge->vertex1()->y()),
                            boundingbox);
                        pt_temp = point_type2(edge->vertex0()->x(), edge->vertex0()->y());
                        polypts.push_back(point_type2(pt_e.x(), pt_e.y()));
                        NumVertex += 2;
                        index_end  = NumVertex;
                        infinite_e = true;
                    }
                } else {
                    if(fabs(edge->vertex0()->x()) < Bound_Max &&
                       (fabs(edge->vertex0()->y()) < Bound_Max)) {
                        polypts.push_back(point_type2(edge->vertex0()->x(), edge->vertex0()->y()));
                        pt_temp = point_type2(edge->vertex0()->x(), edge->vertex0()->y());
                        pt_e    = clip_infinite_edge(
                            *edge, Bd_Box_minX, Bd_Box_minY, Bd_Box_maxX, Bd_Box_maxY);
                        polypts.push_back(point_type2(pt_e.x(), pt_e.y()));
                        NumVertex += 2;
                        index_end  = NumVertex;
                        infinite_e = true;
                    }
                }
            } else {
                if(edge->vertex1() && fabs(edge->vertex1()->x()) < Bound_Max &&
                   (fabs(edge->vertex1()->y()) < Bound_Max)) {
                    pt_s = clip_infinite_edge(
                        *edge, Bd_Box_minX, Bd_Box_minY, Bd_Box_maxX, Bd_Box_maxY);
                    polypts.push_back(point_type2(pt_s.x(), pt_s.y()));
                    NumVertex++;
                    infinite_s = true;
                }
            }
            edge = edge->next();
        } while(edge != cell.incident_edge());

        if(infinite_s && infinite_e) {
            vector<point_type2> vertexes = add_bounding_points(
                pt_s, pt_e, pt_temp, Bd_Box_minX, Bd_Box_minY, Bd_Box_maxX, Bd_Box_maxY);

            if(!vertexes.empty()) {
                polypts.reserve(polypts.size() + vertexes.size());
                polypts.insert(polypts.begin() + index_end, vertexes.begin(), vertexes.end());
            }
        }
        for(int i = 0; i < (int) polypts.size(); i++) {
            boost::geometry::append(poly, point_2d(polypts[i].x(), polypts[i].y()));
        }

        boost::geometry::correct(poly);
        //polygons.push_back(poly);
        //cout << "poly is: " << typeid(poly).name() << '\n'
        int id_ped                         = IdInFrame[Ncell];
        std::pair<polygon_2d, int> poly_id = std::make_pair(poly, id_ped);
        if(id_ped < 0) {
            continue;
        }
        polygons_id.push_back(poly_id);
        Ncell++;
    } // for voronoi cells
    return polygons_id;
}

point_type2 VoronoiDiagram::retrieve_point(const cell_type & cell)
{
    source_index_type index = cell.source_index();
    return points[index];
}

point_type2 VoronoiDiagram::clip_infinite_edge(
    const edge_type & edge,
    double minX,
    double minY,
    double maxX,
    double maxY)
{
    const cell_type & cell1 = *edge.cell();
    const cell_type & cell2 = *edge.twin()->cell();
    point_type2 origin, direction, pt;
    // Infinite edges could not be created by two segment sites.

    if(cell1.contains_point() && cell2.contains_point()) {
        point_type2 p1 = retrieve_point(cell1);
        point_type2 p2 = retrieve_point(cell2);
        origin.x((p1.x() + p2.x()) * 0.5);
        origin.y((p1.y() + p2.y()) * 0.5);
        direction.x(p1.y() - p2.y());
        direction.y(p2.x() - p1.x());
    } else {
        cout << "A cell cannot find its center point!" << endl;
        exit(0);
    }

    double side = maxX - minX;
    double koef = side / (std::max)(fabs(direction.x()), fabs(direction.y()));
    if(edge.vertex0() == NULL) {
        pt.x(origin.x() - direction.x() * koef);
        pt.y(origin.y() - direction.y() * koef);
    } else if(edge.vertex1() == NULL) {
        pt.x(origin.x() + direction.x() * koef);
        pt.y(origin.y() + direction.y() * koef);
    }

    polygon_2d boundingbox;
    boost::geometry::append(boundingbox, boost::geometry::make<point_2d>(minX, minY));
    boost::geometry::append(boundingbox, boost::geometry::make<point_2d>(minX, maxY));
    boost::geometry::append(boundingbox, boost::geometry::make<point_2d>(maxX, maxY));
    boost::geometry::append(boundingbox, boost::geometry::make<point_2d>(maxX, minY));
    boost::geometry::correct(boundingbox);

    pt = getIntersectionPoint(
        point_2d(pt.x(), pt.y()), point_2d(origin.x(), origin.y()), boundingbox);
    return pt;
}

double VoronoiDiagram::area_triangle(
    const point_type2 & tri_p1,
    const point_type2 & tri_p2,
    const point_type2 & tri_p3)
{
    double x = tri_p2.x() - tri_p1.x();
    double y = tri_p2.y() - tri_p1.y();

    double x2 = tri_p3.x() - tri_p1.x();
    double y2 = tri_p3.y() - tri_p1.y();

    return abs(x * y2 - x2 * y) / 2.;
}

bool VoronoiDiagram::point_inside_triangle(
    const point_type2 & pt,
    const point_type2 & tri_p1,
    const point_type2 & tri_p2,
    const point_type2 & tri_p3)
{
    double s   = area_triangle(tri_p1, tri_p2, tri_p3);
    double ss  = area_triangle(pt, tri_p1, tri_p2);
    double ss2 = area_triangle(pt, tri_p2, tri_p3);
    double ss3 = area_triangle(pt, tri_p1, tri_p3);

    if(ss + ss2 + ss3 - s > 1e-6) {
        return false;
    }
    return true;
}

vector<point_type2> VoronoiDiagram::add_bounding_points(
    const point_type2 & pt1,
    const point_type2 & pt2,
    const point_type2 & pt,
    double minX,
    double minY,
    double maxX,
    double maxY)
{
    vector<point_type2> bounding_vertex;
    if(fabs(pt1.x() - pt2.x()) > J_EPS && fabs(pt1.y() - pt2.y()) > J_EPS) {
        if((fabs(pt1.y() - maxY) < J_EPS && fabs(pt2.x() - maxX) < J_EPS) ||
           (fabs(pt2.y() - maxY) < J_EPS && fabs(pt1.x() - maxX) < J_EPS)) {
            point_type2 vertex(maxX, maxY);
            if(point_inside_triangle(pt, vertex, pt1, pt2)) {
                bounding_vertex.push_back(point_type2(minX, maxY));
                bounding_vertex.push_back(point_type2(minX, minY));
                bounding_vertex.push_back(point_type2(maxX, minY));
            } else {
                bounding_vertex.push_back(point_type2(maxX, maxY));
            }
        } else if(
            (fabs(pt1.y() - maxY) < J_EPS && fabs(pt2.x() - minX) < J_EPS) ||
            (fabs(pt2.y() - maxY) < J_EPS && fabs(pt1.x() - minX) < J_EPS)) {
            point_type2 vertex(minX, maxY);
            if(point_inside_triangle(pt, vertex, pt1, pt2)) {
                bounding_vertex.push_back(point_type2(minX, minY));
                bounding_vertex.push_back(point_type2(maxX, minY));
                bounding_vertex.push_back(point_type2(maxX, maxY));
            } else {
                bounding_vertex.push_back(point_type2(minX, maxY));
            }
        } else if(
            (fabs(pt1.y() - minY) < J_EPS && fabs(pt2.x() - minX) < J_EPS) ||
            (fabs(pt2.y() - minY) < J_EPS && fabs(pt1.x() - minX) < J_EPS)) {
            point_type2 vertex(minX, minY);
            if(point_inside_triangle(pt, vertex, pt1, pt2)) {
                bounding_vertex.push_back(point_type2(maxX, minY));
                bounding_vertex.push_back(point_type2(maxX, maxY));
                bounding_vertex.push_back(point_type2(minX, maxY));
            } else {
                bounding_vertex.push_back(point_type2(minX, minY));
            }
        } else if(
            (fabs(pt1.y() - minY) < J_EPS && fabs(pt2.x() - maxX) < J_EPS) ||
            (fabs(pt2.y() - minY) < J_EPS && fabs(pt1.x() - maxX) < J_EPS)) {
            point_type2 vertex(maxX, minY);
            if(point_inside_triangle(pt, vertex, pt1, pt2)) {
                bounding_vertex.push_back(point_type2(maxX, maxY));
                bounding_vertex.push_back(point_type2(minX, maxY));
                bounding_vertex.push_back(point_type2(minX, minY));
            } else {
                bounding_vertex.push_back(point_type2(maxX, minY));
            }
        } else if(
            (fabs(pt1.y() - minY) < J_EPS && fabs(pt2.y() - maxY) < J_EPS) ||
            (fabs(pt2.y() - minY) < J_EPS && fabs(pt1.y() - maxY) < J_EPS)) {
            if(fabs(pt1.x() - pt2.x()) < J_EPS) {
                if(pt1.x() < pt.x()) {
                    bounding_vertex.push_back(point_type2(minX, maxY));
                    bounding_vertex.push_back(point_type2(minX, minY));
                } else {
                    bounding_vertex.push_back(point_type2(maxX, minY));
                    bounding_vertex.push_back(point_type2(maxX, maxY));
                }
            } else {
                double tempx =
                    pt1.x() + (pt2.x() - pt1.x()) * (pt.y() - pt1.y()) / (pt2.y() - pt1.y());
                if(tempx < pt.x()) {
                    bounding_vertex.push_back(point_type2(minX, maxY));
                    bounding_vertex.push_back(point_type2(minX, minY));
                } else {
                    bounding_vertex.push_back(point_type2(maxX, minY));
                    bounding_vertex.push_back(point_type2(maxX, maxY));
                }
            }
        } else if(
            (fabs(pt1.x() - minX) < J_EPS && fabs(pt2.x() - maxX) < J_EPS) ||
            (fabs(pt2.x() - minX) < J_EPS && fabs(pt1.x() - maxX) < J_EPS)) {
            if(fabs(pt1.y() - pt2.y()) < J_EPS) {
                if(pt1.y() < pt.y()) {
                    bounding_vertex.push_back(point_type2(minX, minY));
                    bounding_vertex.push_back(point_type2(maxX, minY));
                } else {
                    bounding_vertex.push_back(point_type2(maxX, maxY));
                    bounding_vertex.push_back(point_type2(minX, maxY));
                }
            } else {
                double tempy =
                    pt1.y() + (pt2.y() - pt1.y()) * (pt.x() - pt1.x()) / (pt2.x() - pt1.x());
                if(tempy < pt.y()) {
                    bounding_vertex.push_back(point_type2(minX, minY));
                    bounding_vertex.push_back(point_type2(maxX, minY));
                } else {
                    bounding_vertex.push_back(point_type2(maxX, maxY));
                    bounding_vertex.push_back(point_type2(minX, maxY));
                }
            }
        }
    }
    return bounding_vertex;
}


//-----------In getIntersectionPoint() the edges of the square is  vertical or horizontal segment--------------
point_type2 VoronoiDiagram::getIntersectionPoint(
    const point_2d & pt0,
    const point_2d & pt1,
    const polygon_2d & square)
{
    vector<point_2d> pt;
    segment edge0(pt0, pt1);
    vector<point_2d> const & opoints = square.outer();
    for(vector<point_2d>::size_type i = 1; i < opoints.size(); ++i) {
        segment edge1(opoints[i], opoints[i - 1]);
        if(intersects(edge0, edge1)) {
            intersection(edge0, edge1, pt);
            break;
        }
    }
    if(pt.empty()) {
        segment edge1(opoints[3], opoints[0]);
        intersection(edge0, edge1, pt);
    }
    point_type2 interpts(pt[0].x(), pt[0].y());
    return interpts;
}

std::vector<std::pair<polygon_2d, int>> VoronoiDiagram::cutPolygonsWithGeometry(
    const std::vector<std::pair<polygon_2d, int>> & polygon,
    const polygon_2d & Geometry,
    const vector<double> & xs,
    const vector<double> & ys)
{
    // vector<polygon_2d> intersetionpolygons;
    std::vector<std::pair<polygon_2d, int>> intersetionpolygons;
    int temp = 0;
    // std::cout << "geometry = " << dsv(Geometry) << "\n";
    for(const auto & polygon_iterator : polygon) {
        polygon_list v;
        polygon_2d p = polygon_iterator.first;

        intersection(Geometry, p, v);
        // std::cout << "p" << polygon_iterator.second << " = " << dsv(p) << "\n";
        BOOST_FOREACH(auto & it, v) {
            // std::cout << "v" << temp << " = " << dsv(it) << "\n";
            if(within(point_2d(xs[temp], ys[temp]), it)) {
                // std::cout << "within " << temp << ": " << xs[temp] << ", " << ys[temp] << "\n";
                //check and remove duplicates
                //dispatch::unique (it);
                polygon_2d simplified;
                simplify(it, simplified, J_EPS);

                correct(simplified);
                intersetionpolygons.push_back(std::make_pair(simplified, polygon_iterator.second));
            }
        }
        temp++;
    }
    return intersetionpolygons;
}


std::vector<std::pair<polygon_2d, int>> VoronoiDiagram::cutPolygonsWithCircle(
    const std::vector<std::pair<polygon_2d, int>> & polygon,
    const vector<double> & xs,
    const vector<double> & ys,
    double radius,
    int edges)
{
    // std::cout << "ENTER CIRCLE\n";
    std::vector<std::pair<polygon_2d, int>> intersetionpolygons;
    int temp = 0;
    for(const auto & polygon_iterator : polygon) {
        polygon_2d circle;
        polygon_2d p = polygon_iterator.first;
        {
            for(int angle = 0; angle <= edges; angle++) {
                double ptx = xs[temp] + radius * cos(angle * 2 * PI / edges);
                double pty = ys[temp] + radius * sin(angle * 2 * PI / edges);
                append(circle, make<point_2d>(ptx, pty));
            }
        }
        correct(circle);
        polygon_list v;
        intersection(circle, p, v);
        // std::cout << "p" << polygon_iterator.second << " = " << dsv(p) << "\n";
        // std::cout << "circle" << polygon_iterator.second << " = " << dsv(circle) << "\n";

        BOOST_FOREACH(auto & it, v) {
            // std::cout << "v" << temp << " = " << dsv(it) << "\n";
            // std::cout << "check " << temp << ": " << xs[temp] << ", " << ys[temp] << "\n";
            if(within(point_2d(xs[temp], ys[temp]), it)) {
                // std::cout << "within " << temp << ": " << xs[temp] << ", " << ys[temp] << "\n";
                //check and remove duplicates
                //dispatch::unique (it);
                polygon_2d simplified;
                simplify(it, simplified, J_EPS);
                correct(simplified);
                intersetionpolygons.push_back(std::make_pair(simplified, polygon_iterator.second));
            }
        }
        temp++;
    }
    // std::cout << "LEAVE CIRCLE\n";
    return intersetionpolygons;
}
