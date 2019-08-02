/**
 * \file        ConvexDecomp.h
 * \date        Jul 4, 2014
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
 *
 *
 **/
#pragma once

#ifdef _CGAL
#include <CGAL/Alpha_shape_2.h>
#include <CGAL/Alpha_shape_euclidean_traits_2.h>
#include <CGAL/Alpha_shape_face_base_2.h>
#include <CGAL/Alpha_shape_vertex_base_2.h>
#include <CGAL/basic.h>
#include <CGAL/Cartesian.h>
#include <CGAL/centroid.h>
#include <CGAL/connect_holes.h>
#include <CGAL/Constrained_Delaunay_triangulation_2.h>
#include <CGAL/Delaunay_triangulation_2.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/IO/Geomview_stream.h>
#include <CGAL/partition_2.h>
#include <CGAL/Partition_is_valid_traits_2.h>
#include <CGAL/Partition_traits_2.h>
#include <CGAL/point_generators_2.h>
#include <CGAL/Polygon_2.h>
#include <CGAL/polygon_function_objects.h>
#include <CGAL/random_polygon_2.h>
#include <CGAL/Triangulation_face_base_2.h>
#include <CGAL/Triangulation_face_base_with_info_2.h>

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <list>

struct FaceInfo2 {
     FaceInfo2() {}
     int nesting_level;

     bool in_domain() {
          return nesting_level%2 == 1;
     }
};

typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef CGAL::Partition_traits_2<K> Traits;
typedef CGAL::Is_convex_2<Traits> Is_convex_2;
typedef Traits::Polygon_2 Polygon_2;
typedef Traits::Point_2 Point_2;
typedef Polygon_2::Vertex_const_iterator Vertex_iterator;
typedef std::list<Polygon_2> Polygon_list;
typedef Polygon_list::const_iterator Polygon_iterator;
typedef CGAL::Partition_is_valid_traits_2<Traits, Is_convex_2> Validity_traits;
typedef Polygon_2::Edge_const_iterator Edge_iterator;
typedef CGAL::Triangulation_vertex_base_2<K> Vb;
typedef CGAL::Triangulation_face_base_with_info_2<FaceInfo2, K> Fbb;
typedef CGAL::Constrained_triangulation_face_base_2<K, Fbb> Fb;
typedef CGAL::Triangulation_data_structure_2<Vb, Fb> TDS;
typedef CGAL::Exact_predicates_tag Itag;
typedef CGAL::Constrained_Delaunay_triangulation_2<K, TDS, Itag> CDT;
typedef CGAL::Alpha_shape_vertex_base_2<K> Av;
typedef CGAL::Triangulation_face_base_2<K> Tf;
typedef CGAL::Alpha_shape_face_base_2<K, Tf> Af;
typedef CGAL::Triangulation_default_data_structure_2<K, Av, Af> Tds;
typedef CGAL::Delaunay_triangulation_2<K, Tds> DT;
typedef CGAL::Polygon_with_holes_2<K> Polygon_with_holes_2;
typedef CGAL::Segment_2<K> Segment_2;
typedef CGAL::Alpha_shape_2<DT> Alpha_shape_2;
typedef Alpha_shape_2::Alpha_iterator Alpha_iterator;
typedef Alpha_shape_2::Vertex_iterator Alpha_vertex_iterator;

inline
void
mark_domains(CDT& ct,
             CDT::Face_handle start,
             int index,
             std::list<CDT::Edge>& border )
{
     if(start->info().nesting_level != -1) {
          return;
     }
     std::list<CDT::Face_handle> queue;
     queue.push_back(start);

     while(! queue.empty()) {
          CDT::Face_handle fh = queue.front();
          queue.pop_front();
          if(fh->info().nesting_level == -1) {
               fh->info().nesting_level = index;
               for(int i = 0; i < 3; i++) {
                    CDT::Edge e(fh,i);
                    CDT::Face_handle n = fh->neighbor(i);
                    if(n->info().nesting_level == -1) {
                         if(ct.is_constrained(e)) border.push_back(e);
                         else queue.push_back(n);
                    }
               }
          }
     }
}

//explore set of facets connected with non constrained edges,
//and attribute to each such set a nesting level.
//We start from facets incident to the infinite vertex, with a nesting
//level of 0. Then we recursively consider the non-explored facets incident
//to constrained edges bounding the former set and increase the nesting level by 1.
//Facets in the domain are those with an odd nesting level.
inline
void
mark_domains(CDT& cdt)
{
     for(CDT::All_faces_iterator it = cdt.all_faces_begin(); it != cdt.all_faces_end(); ++it) {
          it->info().nesting_level = -1;
     }

     int index = 0;
     std::list<CDT::Edge> border;
     mark_domains(cdt, cdt.infinite_face(), index++, border);
     while(! border.empty()) {
          CDT::Edge e = border.front();
          border.pop_front();
          CDT::Face_handle n = e.first->neighbor(e.second);
          if(n->info().nesting_level == -1) {
               mark_domains(cdt, n, e.first->info().nesting_level+1, border);
          }
     }
}

inline
void insert_polygon(CDT& cdt,const Polygon_2& polygon)
{
     if ( polygon.is_empty() ) return;
     CDT::Vertex_handle v_prev=cdt.insert(*CGAL::cpp0x::prev(polygon.vertices_end()));
     for (Polygon_2::Vertex_iterator vit=polygon.vertices_begin();
               vit!=polygon.vertices_end(); ++vit) {
          CDT::Vertex_handle vh=cdt.insert(*vit);
          cdt.insert_constraint(vh,v_prev);
          v_prev=vh;
     }
}

inline
int test_triangulation( )
{
     //construct two non-intersecting nested polygons
     Polygon_2 polygon1;
     polygon1.push_back(Point_2(0,0));
     polygon1.push_back(Point_2(2,0));
     polygon1.push_back(Point_2(2,2));
     polygon1.push_back(Point_2(0,2));
     Polygon_2 polygon2;
     polygon2.push_back(Point_2(0.5,0.5));
     polygon2.push_back(Point_2(1.5,0.5));
     polygon2.push_back(Point_2(1.5,1.5));
     polygon2.push_back(Point_2(0.5,1.5));

     //Insert the polyons into a constrained triangulation
     CDT cdt;
     insert_polygon(cdt,polygon1);
     insert_polygon(cdt,polygon2);

     //Mark facets that are inside the domain bounded by the polygon
     mark_domains(cdt);

     int count=0;
     for (CDT::Finite_faces_iterator fit=cdt.finite_faces_begin();
               fit!=cdt.finite_faces_end(); ++fit) {
          if ( fit->info().in_domain() ) ++count;
     }

     //cdt.draw_triangulation(std::cout);
     std::cout << "There are " << count << " facets in the domain." << std::endl;


     CGAL::Geomview_stream gv(CGAL::Bbox_3(-100, -100, -100, 100, 100, 100));
     gv.set_line_width(4);
     gv.set_trace(true);
     gv.set_bg_color(CGAL::Color(0, 200, 200));
     // gv.clear();

     // use different colors, and put a few sleeps/clear.
     //gv << CGAL::BLUE;
     //gv.set_wired(true);


     CDT::Finite_faces_iterator it;
     for (it = cdt.finite_faces_begin(); it != cdt.finite_faces_end(); it++) {
          std::cout << cdt.triangle(it) << std::endl;
          gv << cdt.triangle(it) ;
     }

     return 0;
}

inline
int test_alpha_shape()
{
     std::list<Point_2> lp;


     lp.push_back(Point_2(0.5,0.5));
     lp.push_back(Point_2(1.5,0.5));
     lp.push_back(Point_2(1.5,1.5));
     lp.push_back(Point_2(0.5,1.5));

     // compute alpha shape
     Alpha_shape_2 as(lp.begin(),lp.end());
     std::cout << "Alpha shape computed in REGULARIZED mode by default"
               << std::endl;


     std::list<Point_2> env=as.Output();
     std::cout <<"size: " <<env.size()<<std::endl;
     getc(stdin);

     for (std::list<Point_2>::const_iterator iterator = env.begin(), end = env.end(); iterator != end; ++iterator) {
          std::cout << *iterator;
     }

     //  for (Alpha_vertex_iterator vit = as.Alpha_shape_vertices_begin();
     //                    vit !=  as.alpha_shape_vertices_end(); ++vit) {
     //
     //    }

     // find optimal alpha value
     Alpha_iterator opt = as.find_optimal_alpha(1);
     std::cout << "Optimal alpha value to get one connected component is "
               <<  *opt    << std::endl;
     as.set_alpha(*opt);
     assert(as.number_of_solid_components() == 1);
     return 0;
}

template<class Kernel, class Container>
void print_polygon (const CGAL::Polygon_2<Kernel, Container>& P)
{
     typename CGAL::Polygon_2<Kernel, Container>::Vertex_const_iterator  vit;

     std::cout << "[ " << P.size() << " vertices:";
     for (vit = P.vertices_begin(); vit != P.vertices_end(); ++vit)
          std::cout << " (" << *vit << ')';
     std::cout << " ]" << std::endl;
}

template<class Kernel, class Container>
void print_polygon_with_holes(const CGAL::Polygon_with_holes_2<Kernel, Container> & pwh)
{
     if (! pwh.is_unbounded()) {
          std::cout << "{ Outer boundary = ";
          print_polygon (pwh.outer_boundary());
     } else
          std::cout << "{ Unbounded polygon." << std::endl;

     typename CGAL::Polygon_with_holes_2<Kernel,Container>::Hole_const_iterator hit;
     unsigned int k = 1;

     std::cout << "  " << pwh.number_of_holes() << " holes:" << std::endl;
     for (hit = pwh.holes_begin(); hit != pwh.holes_end(); ++hit, ++k) {
          std::cout << "    Hole #" << k << " = ";
          print_polygon (*hit);
     }
     std::cout << " }" << std::endl;
}
#endif /* _CGAL */
