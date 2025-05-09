// Copyright (c) 2005 Rijksuniversiteit Groningen (Netherlands)
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL: https://github.com/CGAL/cgal/blob/v6.0.1/Skin_surface_3/include/CGAL/Marching_tetrahedra_observer_default_3.h $
// $Id: include/CGAL/Marching_tetrahedra_observer_default_3.h 50cfbde3b84 $
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Nico Kruithof <Nico@cs.rug.nl>

#ifndef CGAL_MARCHING_TETRAHEDRA_OBSERVER_DEFAULT_3_H
#define CGAL_MARCHING_TETRAHEDRA_OBSERVER_DEFAULT_3_H

#include <CGAL/license/Skin_surface_3.h>


#include <CGAL/basic.h>

namespace CGAL {

template <class Vertex_iterator, class Cell_iterator, class Polyhedron_3>
class Marching_tetrahedra_observer_default_3
{
public:
  typedef Polyhedron_3                        Polyhedron;

  typedef Cell_iterator                       T_Cell_iterator;
  typedef typename Polyhedron::Vertex_handle  Polyhedron_vertex_handle;
  typedef typename Polyhedron::Facet_handle   Polyhedron_facet_handle;

  Marching_tetrahedra_observer_default_3() { }
  Marching_tetrahedra_observer_default_3(const Marching_tetrahedra_observer_default_3&) { }

  void after_vertex_insertion(T_Cell_iterator, int, int, Polyhedron_vertex_handle) { }
  void after_facet_insertion(T_Cell_iterator, Polyhedron_facet_handle) { }
};

} //namespace CGAL

#endif // CGAL_MARCHING_TETRAHEDRA_OBSERVER_DEFAULT_3_H
